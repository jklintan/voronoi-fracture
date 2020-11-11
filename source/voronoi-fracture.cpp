#include "voronoi-fracture.h"

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

#include "util.h"

VoronoiFracture::VoronoiFracture() {};

VoronoiFracture::~VoronoiFracture() {};

MStatus VoronoiFracture::doIt(const MArgList& args)
{
    int num_fragments = 5;
    for (unsigned int i = 0; i < args.length(); i++)
    {
        if (args.asString(i) == MString("-num_fragments"))
        {
            args.get(i + 1, num_fragments);
        }
    }

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
    displayInfo(MString("Fracturing ") + node_fn.name());

    MObject mesh = node_fn.object();

    // Generate uniform points in bounding box of mesh (in local object space)
    MBoundingBox BB = node_fn.boundingBox();
    std::vector<MPoint> points = generateUniformPoints(BB.min(), BB.max(), num_fragments);

    MDagModifier dag_modifier;

    // Create group/transform to store fragments under
    MObject fragment_group = dag_modifier.createNode("transform");
    dag_modifier.renameNode(fragment_group, "fragment_group");
    dag_modifier.doIt();

    // Copy transform from original mesh
    MFnTransform(fragment_group).set(node.inclusiveMatrix());

    MStatus status;
    for (size_t i = 0; i < points.size(); i++)
    {
        const MPoint &p0 = points[i];

        displayInfo(("Processing fragment " + std::to_string(i) + " for point: " + 
            std::to_string(p0.x) + ", " +
            std::to_string(p0.y) + ", " +
            std::to_string(p0.z)).c_str()
        );


        MObject fragment_transform = dag_modifier.createNode("transform", fragment_group, &status);
        dag_modifier.renameNode(fragment_transform, ("fragment_" + std::to_string(i)).c_str());
        dag_modifier.doIt();

        // Duplicate mesh
        MFnMesh fragment;
        fragment.copy(mesh, fragment_transform, &status);
        if (!status)
        {
            displayError("Could not duplicate selected mesh.");
            return status;
        }

        for (size_t j = 0; j < points.size(); j++)
        {
            if (j == i) continue;

            MPlane cut_plane = getBisectorPlane(p0, points[j]);

            // TODO: 
            // - Cut fragment with cut_plane
            // - Remove faces on the positive side (with respect to normal) of the cut_plane
            // - Close resulting hole
        }
    }
    
    return MS::kSuccess;
}

void* VoronoiFracture::creator()
{
    return new VoronoiFracture();
}