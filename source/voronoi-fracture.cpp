#include "voronoi-fracture.h"

#include <chrono>

#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MObject.h>
#include <maya/MFnDagNode.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>
#include <maya/MBoundingBox.h>
#include <maya/MPlane.h>
#include <maya/MFnTransform.h>
#include <maya/MMatrix.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MObjectArray.h>

#include "util.h"

VoronoiFracture::VoronoiFracture() {};

VoronoiFracture::~VoronoiFracture() {};

MStatus VoronoiFracture::doIt(const MArgList& args)
{
    MArgDatabase arg_data(syntax(), args);

    num_fragments.setValue(arg_data);
    delete_object.setValue(arg_data);

    MSelectionList list;
    MGlobal::getActiveSelectionList(list);

    MItSelectionList it(list, MFn::kMesh);
    if (it.isDone())
    {
        displayError("Polygon mesh must be selected before fracturing.");
        return MS::kFailure;
    }

    // Use the first kMesh object in the selection
    MDagPath node;
    it.getDagPath(node);

    MFnDagNode node_fn(node);
    displayInfo(MString("Fracturing ") + node_fn.fullPathName());

    MFnDagNode fragment_group;
    MStatus status = generateFragmentMeshes(node_fn.fullPathName().asChar(), num_fragments, fragment_group);
    if (!status)
    {
        displayError("Unable to duplicate selected mesh.");
        return status;
    }

    auto begin = std::chrono::high_resolution_clock::now();

    // Transformation matrix
    MMatrix M = node.inclusiveMatrix();

    // Generate uniform points in bounding box of mesh
    MBoundingBox BB = node_fn.boundingBox();

    MVector extent = BB.max() - BB.min();
    double clip_triangle_half_extent = extent.length() * 10.0;

    std::vector<MPoint> points;
    if constexpr (CLIP_TYPE == ClipType::BOOLEAN)
        points = generateUniformPoints(BB.min(), BB.max(), num_fragments);
    else
        points = generateUniformPoints(BB.min() * M, BB.max() * M, num_fragments);

    std::vector<MObject> clipped;

    for (unsigned int i = 0; i < points.size(); i++)
    {
        const MPoint& p0 = points[i];

        displayInfo(("Processing fragment " + std::to_string(i) + " for point: " +
            std::to_string(p0.x) + ", " +
            std::to_string(p0.y) + ", " +
            std::to_string(p0.z)).c_str()
        );

        MFnMesh fragment(MFnDagNode(fragment_group.child(i)).child(0), &status);
        if (!status)
        {
            displayError("Could not retrieve fragment mesh.");
            return status;
        }

        for (size_t j = 0; j < points.size(); j++)
        {
            if (j == i) continue;

            Plane clip_plane = getBisectorPlane(p0, points[j]);

            if (!clip_plane.isClipped(fragment)) continue;

            if constexpr (CLIP_TYPE == ClipType::BOOLEAN)
                status = booleanClipAndCap(fragment, clip_plane, clip_triangle_half_extent);
            else
                status = internalClipAndCap(fragment.fullPathName().asChar(), clip_plane);

            if (!status)
            {
                displayError("Could not execute clip and cap.");
                return status;
            }

            // Object was completely clipped
            if (fragment.numPolygons() < 4)
            {
                clipped.push_back(fragment_group.child(i));
                break;
            }
        }
    }

    if (clipping_mesh)
    {
        dag_modifier.deleteNode(clipping_mesh->object());
        clipping_mesh.reset();
    }

    // Delete original objects
    if (delete_object) dag_modifier.deleteNode(node_fn.parent(0));

    // Delete clipped fragments
    for (auto& o : clipped) dag_modifier.deleteNode(o);

    dag_modifier.doIt();

    // Rename fragments
    for (unsigned int i = 0; i < fragment_group.childCount(); i++)
    {
        MFnDagNode transform_node_fn(fragment_group.child(i));
        MString new_name = transform_node_fn.setName(("fragment_" + std::to_string(i)).c_str());
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin);
    displayInfo(("Fracture time: " + std::to_string(duration.count() * 1e-6) + " seconds.").c_str());

    return MS::kSuccess;
}

void* VoronoiFracture::creator()
{
    return new VoronoiFracture();
}

MSyntax VoronoiFracture::syntaxCreator()
{
    MSyntax syntax;
    num_fragments.addToSyntax(syntax);
    delete_object.addToSyntax(syntax);
    return syntax;
}

// Mesh clip and cap using internal commands polyCut and polyCloseBorder
MStatus VoronoiFracture::internalClipAndCap(const char* object, const Plane& clip_plane)
{
    const MVector& n = clip_plane.normal;
    const MVector& p = clip_plane.point;

    return MGlobal::executePythonCommand(formatString(
        "Util.clipAndCap('%s', [%f,%f,%f], [%f,%f,%f])", 
        object, -n.x, -n.y, -n.z, p.x, p.y, p.z
    ));
}

// Mesh clip and cap using boolean intersection from C++ api
MStatus VoronoiFracture::booleanClipAndCap(MFnMesh& object, const Plane& clip_plane, double half_extent)
{
    // Legacy mode using triangle plane is way faster (19 sec for 1000 fragments), but 
    // this produces artefacts in edge cases. Non-legacy is more robust but requires 
    // an intersection volume instead, for which a tetrahedra is generated.
    constexpr bool LEGACY = false;

    if (!clipping_mesh)
    {
        clipping_mesh = std::make_unique<MFnMesh>();

        if constexpr (LEGACY)
        {
            constexpr int indices[] = { 0, 1, 2 };
            (void)clipping_mesh->create(3, 1, MPointArray(3), MIntArray(1, 3), MIntArray(indices, 3));
        }
        else
        {
            constexpr int indices[] = { 0, 1, 2, /**/ 0, 3, 1, /**/ 0, 2, 3, /**/ 1, 3, 2 };
            (void)clipping_mesh->create(4, 4, MPointArray(4), MIntArray(4, 3), MIntArray(indices, 12));
        }
    }

    // Generate arbitrary orthonormal basis for plane, (X, Y, clip_plane.normal).
    MVector X = orthogonalUnitVector(clip_plane.normal);
    MVector Y = clip_plane.normal ^ X;

    const auto& p = clip_plane.point;

    for (int i = 0; i < 3; i++)
    {
        double a = i * 2.0 * M_PI / 3.0;
        clipping_mesh->setPoint(i, p + X * half_extent * std::cos(a) + Y * half_extent * std::sin(a));
    }

    if constexpr (!LEGACY) clipping_mesh->setPoint(3, p - clip_plane.normal * half_extent);

    MObjectArray objects;
    objects.append(object.object());
    objects.append(clipping_mesh->object());

    return object.booleanOps(MFnMesh::kIntersection, objects, LEGACY);
}

MStatus VoronoiFracture::generateFragmentMeshes(const char* object, size_t num, MFnDagNode& parent)
{
    MString group_name;
    MStatus status = MGlobal::executePythonCommand(formatString(
        "Util.multiDuplicate('%s', %d, 'fragments')", object, num), group_name
    );

    if (!status) return status;

    MSelectionList group_list;
    status = group_list.add(group_name);

    MItSelectionList group_it(group_list, MFn::kTransform);

    if (group_it.isDone()) return MS::kFailure;

    MObject group;
    group_it.getDependNode(group);

    parent.setObject(group);

    if (parent.childCount() != num) return MS::kFailure;

    return MS::kSuccess;
}