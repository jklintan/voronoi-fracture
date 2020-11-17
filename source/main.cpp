#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

#include "voronoi-fracture.h"
#include "menuUI.h"

#include <iostream>
#include <fstream>

bool is_file_exist(const char* fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}


MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "Josefine Klintberg and Linus Mossberg", "0.0", "Any");
    MStatus status = plugin.registerCommand("voronoiFracture", VoronoiFracture::creator);

    if (!status)
    {
        status.perror("registerCommand");
    }

    status = plugin.registerCommand("menuUI", MenuUI::creator);

    if (!status)
    {
        status.perror("registerCommand");
    }

    // Create UI menu
    MString menuCmd = "maya.cmds.menuUI()";
    MGlobal::executePythonCommand(menuCmd);

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);

    // Remove UI Window
    MenuUI::remove();

    MStatus status = plugin.deregisterCommand("voronoiFracture");

    if (!status)
    {
        status.perror("registerCommand");
    }

    status = plugin.deregisterCommand("menuUI");

    if (!status)
    {
        status.perror("registerCommand");
    }

    return status;
}