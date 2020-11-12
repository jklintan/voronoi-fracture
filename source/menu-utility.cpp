#include <maya/MStatus.h>
#include <maya/MGlobal.h>

void createMenu() {
    MString menuCmd = MString("menu = maya.cmds.menu('fractureMenu', l = 'Fracturing', to=1, p='MayaWindow')\nmenuItem1 = maya.cmds.menuItem(l = 'Voronoi Fracturing', c = 'maya.cmds.menuUI()')");
    MGlobal::executePythonCommand(menuCmd);
}

void removeMenu() {
    MString cleanupCmd = MString("if maya.cmds.menu('fractureMenu', ex=1):\n   maya.cmds.deleteUI('fractureMenu')");
    MGlobal::executePythonCommand(cleanupCmd);
}

void createWindow() {
    MString windowCmd = MString("maya.cmds.showWindow(maya.cmds.window(title = 'Fracturing Menu', widthHeight = (500, 400)))");
    MGlobal::executePythonCommand(windowCmd);
}
