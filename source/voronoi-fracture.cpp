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
#include <maya/MDagModifier.h>
#include <maya/MFnTransform.h>
#include <maya/MMatrix.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>

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

    MObject mesh = node_fn.object();

    auto begin = std::chrono::high_resolution_clock::now();

    MMatrix transformation_matrix = node.inclusiveMatrix();

    // Generate uniform points in bounding box of mesh
    MBoundingBox BB = node_fn.boundingBox();
    std::vector<MPoint> points = generateUniformPoints(BB.min() * transformation_matrix, BB.max() * transformation_matrix, num_fragments);

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

            status = internalClipAndCap(fragment.fullPathName().asChar(), clip_plane);

            if (!status)
            {
                displayError("Could not execute clip and cap command.");
                return status;
            }

            // Object was completely clipped
            if (fragment.numPolygons() < 3)
            {
                clipped.push_back(fragment_group.child(i));
                break;
            }
        }
    }

    MDagModifier dag_modifier;

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