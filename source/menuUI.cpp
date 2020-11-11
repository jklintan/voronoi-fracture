#include "menuUI.h"
#include "menu-utility.h"

#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MObject.h>
#include <maya/MFnDagNode.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>

MenuUI::MenuUI() {}
MenuUI::~MenuUI() {}

MStatus MenuUI::doIt(const MArgList& args) {
  displayInfo("Initializing window");
  return MS::kSuccess;
}

void* MenuUI::creator()
{
  return new MenuUI();
}