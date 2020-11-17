#include "menuUI.h"
#include "menu-utility.h"
#include "scripts/initializeUI.py"
#include "scripts/uninitializeUI.py"

#include <maya/MGlobal.h>

MenuUI::MenuUI() {}
MenuUI::~MenuUI() {}

MStatus MenuUI::doIt(const MArgList& args) 
{
    MGlobal::executePythonCommand(initialize_UI);
    return MS::kSuccess;
}

MStatus MenuUI::remove() 
{
    MGlobal::executePythonCommand(uninitialize_UI);
    return MS::kSuccess;
}

void* MenuUI::creator()
{
    return new MenuUI();
}