#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

#include "voronoi-fracture.h"
#include "menu-utility.h"
#include "menuUI.h"

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "Josefine Klintberg and Linus Mossberg", "0.0", "Any");
    MStatus status = plugin.registerCommand("voronoiFracture", VoronoiFracture::creator);
    MStatus statusUI = plugin.registerCommand("menuUI", MenuUI::creator);

    createMenu();

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
    MStatus statusUI = plugin.deregisterCommand("menuUI");

    removeMenu();

    if (!status)
    {
        status.perror("registerCommand");
    }

    return status;
}