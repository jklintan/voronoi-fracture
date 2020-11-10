#include "voronoi-fracture.h"

#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MObject.h>
#include <maya/MFnDagNode.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>

VoronoiFracture::VoronoiFracture() {};

VoronoiFracture::~VoronoiFracture() {};

MStatus VoronoiFracture::doIt(const MArgList& args)
{
    MSelectionList list;
    MGlobal::getActiveSelectionList(list);

    MItSelectionList it(list, MFn::kMesh);
    if (it.isDone())
    {
        displayError("Polygonal mesh must be selected before fracturing.");
        return MS::kFailure;
    }

    // Use the first kMesh object in the selection
    MDagPath node;
    it.getDagPath(node);

    MFnDagNode node_fn(node);
    displayInfo(MString("Fracturing ") + node_fn.name());

    MObject mesh = node_fn.object();

    MStatus status;
    MItMeshVertex vertex_it(mesh, &status);
    if (!status)
    {
        displayError("Invalid mesh selected");
        return status;
    }

    // Just print the vertices of the selected object for now
    for (; !vertex_it.isDone(); vertex_it.next())
    {
        MPoint p = vertex_it.position(MSpace::kWorld);

        displayInfo((
            std::to_string(p.x) + ", " + 
            std::to_string(p.y) + ", " + 
            std::to_string(p.z)).c_str()
        );
    }
    
    return MS::kSuccess;
}

void* VoronoiFracture::creator()
{
    return new VoronoiFracture();
}