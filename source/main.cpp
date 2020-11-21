#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

#include "scripts/initializeUI.py"
#include "scripts/uninitializeUI.py"
#include "scripts/UI/menu.py"
#include "scripts/UI/createFractureUI.py"
#include "scripts/utilities.py"

#include "voronoi-fracture.h"

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "Josefine Klintberg and Linus Mossberg", "0.0", "Any");
    MStatus status = plugin.registerCommand("voronoiFracture", VoronoiFracture::creator, VoronoiFracture::syntaxCreator);

    if (!status)
    {
        status.perror("registerCommand");
        return status;
    }

    // Create UI menu
    status = MGlobal::executePythonCommand(
        (std::string(menu) + createFractureUI + initialize_UI).c_str()
    );

    if (!status) return status;

    status = MGlobal::executePythonCommand(py_utilities);

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);

    MStatus status = plugin.deregisterCommand("voronoiFracture");
    if (!status)
    {
        status.perror("registerCommand");
        return status;
    }

    // Remove UI Window
    status = MGlobal::executePythonCommand(uninitialize_UI);

    return status;
}