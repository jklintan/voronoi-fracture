#pragma once

inline constexpr char initialize_UI[] = R"py(

import maya.cmds as mc

# Create menu
m = Menu("VFMenu", "Fracturing", 1, ['Voronoi Fracturing'], ['CreateFractureUI("Fracturing Menu", 400, 500)']) 
mc.refresh()

)py";
