#pragma once

inline constexpr char uninitialize_UI[] = R"py(

import maya.cmds as mc

def _uninitializeMenu(menuName):
    if mc.menu(menuName, ex=1):
        mc.deleteUI(menuName)

def _removeUIWindow(windowName):
    if mc.window(windowName, ex=1):
        mc.deleteUI(windowName)

def uninitializeUI(menuName, windowName):
    _removeUIWindow(windowName)
    _uninitializeMenu(menuName)

uninitializeUI("VFMenu", "UI")

)py";
