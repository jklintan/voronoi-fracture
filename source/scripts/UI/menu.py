#pragma once

inline constexpr char menu[] = R"py(

import maya.cmds as mc

class Menu:
    def __init__(self, name, label, numbSubMenus = 0, namesSubMenus = [], funcSubMenus = []):
        self.MENU_NAME = name
        self.MENU_LABEL = label
        self.NUMB_SUB_MENUS = numbSubMenus
        self.NAMES_SUB_MENUS = []
        self.FUNC_SUB_MENUS = []
        
        # Assertions for user input
        if numbSubMenus > 0:
            assert(len(namesSubMenus) >= numbSubMenus), "Not enough names for stated amount of sub menus"
            assert(len(funcSubMenus) >= numbSubMenus), "Not enough function calls for stated amount of sub menus"
        
        assert(len(namesSubMenus) == len(funcSubMenus)), "The names of sub menus are not equal to the amount of function calls for sub menus"
        
        if numbSubMenus > 0:
            for i in range(numbSubMenus):
                self.NAMES_SUB_MENUS.append(namesSubMenus[i])
                self.FUNC_SUB_MENUS.append(funcSubMenus[i])
                
        self._removeOld()
        self._build()
    
    def _removeOld(self):
        if mc.menu(self.MENU_NAME, ex=1):
            mc.deleteUI(self.MENU_NAME)

    def _globalRemove(name):
        if mc.menu(name, ex=1):
            mc.deleteUI(name)

    def _build(self):
        menu = mc.menu(self.MENU_NAME, l = self.MENU_LABEL, to=1, p='MayaWindow')
        if self.NUMB_SUB_MENUS > 0:
            for i in range(self.NUMB_SUB_MENUS):
                mc.menuItem(l = self.NAMES_SUB_MENUS[i], c = self.FUNC_SUB_MENUS[i])

)py";
