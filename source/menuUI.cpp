#include "menuUI.h"
#include "menu-utility.h"

#include <maya/MGlobal.h>

MenuUI::MenuUI() {}
MenuUI::~MenuUI() {}

MStatus MenuUI::doIt(const MArgList& args) {

    displayInfo("Initializing window");
    createWindow();

    return MS::kSuccess;
}

void* MenuUI::creator()
{
    return new MenuUI();
}