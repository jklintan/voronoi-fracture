#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>

#include "voronoi-fracture.h"

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "Josefine Klintberg and Linus Mossberg", "0.0", "Any");
    MStatus status = plugin.registerCommand("voronoiFracture", VoronoiFracture::creator);

    if (!status)
    {
        status.perror("registerCommand");
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    MStatus status = plugin.deregisterCommand("voronoiFracture");

    if (!status)
    {
        status.perror("registerCommand");
    }

    return status;
}