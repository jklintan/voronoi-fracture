#pragma once

#include <maya/MGlobal.h>

// Add main menu class
#include "UI/menu.py"
bool loadMenu = MGlobal::executePythonCommand(menu);

// Add window UI class
#include "UI/createFractureUI.py"
bool loadWindow = MGlobal::executePythonCommand(createFractureUI);

inline constexpr char initialize_UI[] = R"py(

import maya.cmds as mc

# Create menu
m = Menu("VFMenu", "Fracturing", 1, ['Voronoi Fracturing'], ['CreateFractureUI("Fracturing Menu", 400, 500)']) 
mc.refresh()

)py";
