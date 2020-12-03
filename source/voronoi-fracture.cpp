#include "voronoi-fracture.h"

#include <chrono>
#include <array>

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
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MDagPathArray.h>
#include <maya/MPlug.h>
#include <maya/MFnParticleSystem.h>

#include "point-distribution.h"
#include "util.h"

VoronoiFracture::VoronoiFracture() {};

VoronoiFracture::~VoronoiFracture() {};

MStatus VoronoiFracture::doIt(const MArgList& args)
{
    MArgDatabase arg_data(syntax(), args);

    num_fragments.setValue(arg_data);
    delete_object.setValue(arg_data);
    curve_radius.setValue(arg_data);
    disk_axis.setValue(arg_data);
    steps.setValue(arg_data);
    step_noise.setValue(arg_data);
    min_distance.setValue(arg_data);

    // Must be > 0
    if (step_noise < 1e-6) step_noise = 1e-6;

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

    // Transformation matrix
    MMatrix M = node.inclusiveMatrix();
    MMatrix M_inv = M.inverse();

    MBoundingBox BB = node_fn.boundingBox();
    BB.transformUsing(M);

    MVector extent = BB.max() - BB.min();
    double clip_triangle_half_extent = extent.length() * 10.0;

    const std::vector<MPoint> points = generateSeedPoints(BB, list);

    if (points.empty())
    {
        displayError("Generated point distribution is empty.");
        return MS::kFailure;
    }

    // Vector of pointers to seed points used for separate sorting
    std::vector<std::vector<MPoint>::const_iterator> point_ptrs(points.size());
    std::generate(point_ptrs.begin(), point_ptrs.end(), [b = points.begin()]() mutable { return b++; });

    MDagPathArray fragment_paths;
    MStatus status = generateFragmentMeshes(node_fn.fullPathName().asChar(), points.size(), fragment_paths);
    if (!status)
    {
        displayError("Unable to duplicate selected mesh.");
        return status;
    }

    auto begin = std::chrono::high_resolution_clock::now();

    std::vector<MObject> clipped;

    for (unsigned int i = 0; i < points.size(); i++)
    {
        const MPoint& p0 = points[i];

        displayInfo(("Processing fragment " + std::to_string(i) + " for point: " +
            std::to_string(p0.x) + ", " +
            std::to_string(p0.y) + ", " +
            std::to_string(p0.z)).c_str()
        );

        auto fragment_path = fragment_paths[i];
        fragment_path.extendToShape();
        MFnMesh fragment(fragment_path, &status);
        if (!status)
        {
            displayError("Could not retrieve fragment mesh. " + status.errorString());
            return status;
        }

        // Sort point pointers based on distance to p0
        std::sort(point_ptrs.begin(), point_ptrs.end(),
            [&p0](const auto& a, const auto& b)
            {
                return p0.distanceTo(*a) < p0.distanceTo(*b);
            }
        );

        const auto& p0_ptr = points.begin() + i;

        for (const auto &p1_ptr : point_ptrs)
        {
            if (p0_ptr == p1_ptr) continue;

            Plane clip_plane = getBisectorPlane(p0, *p1_ptr);

            // This in combination with sorting improves performance a lot
            bool is_clipped;
            if (!clip_plane.intersects(fragment, is_clipped))
            {
                if (is_clipped)
                {
                    clipped.push_back(fragment_paths[i].transform());
                    break;
                }
                continue;
            }

            if constexpr (CLIP_TYPE == ClipType::BOOLEAN)
            {
                // MFnMesh::booleanOps operates in object space
                clip_plane = getBisectorPlane(p0 * M_inv, *p1_ptr * M_inv);
                status = booleanClipAndCap(fragment, clip_plane, clip_triangle_half_extent);
            }
            else
            {
                status = internalClipAndCap(fragment, clip_plane);
            }

            if (!status)
            {
                displayError("Could not execute clip and cap.");
                return status;
            }
        }
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin);
    displayInfo(("Fracture time: " + std::to_string(duration.count() * 1e-6) + " seconds.").c_str());

    if (clipping_mesh)
    {
        dag_modifier.deleteNode(clipping_mesh->object());
        clipping_mesh.reset();
    }

    // Delete original objects
    if (delete_object) dag_modifier.deleteNode(node.transform());

    // Delete clipped fragments
    for (auto& o : clipped) dag_modifier.deleteNode(o);

    dag_modifier.doIt();

    // Rename fragments
    int idx = 0;
    for (unsigned int i = 0; i < fragment_paths.length(); i++)
    {
        MObject transform = fragment_paths[i].transform();
        if (!transform.isNull())
        {
            MString new_name = MFnDagNode(transform).setName(("fragment_" + std::to_string(idx++)).c_str());
        }
    }

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
    curve_radius.addToSyntax(syntax);
    disk_axis.addToSyntax(syntax);
    steps.addToSyntax(syntax);
    step_noise.addToSyntax(syntax);
    min_distance.addToSyntax(syntax);
    return syntax;
}

// Mesh clip and cap using internal commands polyCut and polyCloseBorder
MStatus VoronoiFracture::internalClipAndCap(MFnMesh& object, const Plane& clip_plane)
{
    const MVector& n = clip_plane.normal;
    const MVector& p = clip_plane.point;

    return MGlobal::executePythonCommand(formatString(
        "Util.clipAndCap('%s', [%f,%f,%f], [%f,%f,%f])", 
        object.fullPathName().asChar(), -n.x, -n.y, -n.z, p.x, p.y, p.z
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

MStatus VoronoiFracture::generateFragmentMeshes(const char* object, size_t num, MDagPathArray& fragment_paths)
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
    MDagPath node;
    group_it.getDagPath(node);
    node.getAllPathsBelow(fragment_paths);

    if (fragment_paths.length() != num) return MS::kFailure;

    return MS::kSuccess;
}

std::vector<MPoint> VoronoiFracture::generateSeedPoints(const MBoundingBox& BB, const MSelectionList& list)
{
    std::vector<MPoint> points;

    MItSelectionList sphere_it(list, MFn::kImplicitSphere);
    MItSelectionList curve_it(list, MFn::kNurbsCurve);
    MItSelectionList particle_it(list, MFn::kNParticle);
    
    if (!sphere_it.isDone())
    {
        // createNode implicitSphere
        MDagPath node;
        sphere_it.getDagPath(node);
        MFnDagNode node_fn(node);
        MPlug radius_plug = node_fn.findPlug("radius", true);

        displayInfo(MString("Using ") + node_fn.fullPathName());

        double radius = radius_plug.asDouble();
        MMatrix M = node.inclusiveMatrix();
        MPoint position = MTransformationMatrix(M).getTranslation(MSpace::kWorld);

        unsigned disk_axis_i = 0;
        if ((MString)disk_axis == "x") disk_axis_i = 1;
        else if ((MString)disk_axis == "y") disk_axis_i = 2;
        else if ((MString)disk_axis == "z") disk_axis_i = 3;

        MTransformationMatrix axes_transform(M);
        axes_transform.setTranslation(MVector(0, 0, 0), MSpace::kWorld);
        MMatrix M_axes = axes_transform.asMatrix();

        std::array<MVector, 3> axes = {
            MVector(radius, 0, 0) * M_axes,
            MVector(0, radius, 0) * M_axes,
            MVector(0, 0, radius) * M_axes
        };

        if (disk_axis_i == 0)
        {
            if ((unsigned)steps == 0)
                points = PointDistribution::sphereQuadratic(position, axes, num_fragments);
            else
                points = PointDistribution::sphereSteps(position, axes, steps, step_noise, num_fragments);
        }
        else
        {
            if ((unsigned)steps == 0)
                points = PointDistribution::diskQuadratic(position, axes, disk_axis_i, num_fragments);
            else
                points = PointDistribution::diskSteps(position, axes, steps, disk_axis_i, step_noise, num_fragments);
        }
    }
    else if (!curve_it.isDone())
    {
        MDagPath node;
        curve_it.getDagPath(node);
        MFnDagNode node_fn(node);

        displayInfo(MString("Using ") + node_fn.fullPathName());

        MFnNurbsCurve curve(node);

        points = PointDistribution::curve(curve, curve_radius, num_fragments);
    }
    else if (!particle_it.isDone())
    {
        MDagPath node;
        particle_it.getDagPath(node);
        MFnDagNode node_fn(node);

        displayInfo(MString("Using ") + node_fn.fullPathName());

        MFnParticleSystem particles(node);

        points = PointDistribution::particles(particles);
    }
    else
    {
        points = PointDistribution::uniformBoundingBox(BB.min(), BB.max(), num_fragments);
    }

    points = PointDistribution::removeDuplicates(points, min_distance);

    return points;
}