#pragma once

inline constexpr char createFractureUI[] = R"py(

import maya.cmds as mc

def _applySlider(*args):
    print("Applying Slider Value " + str(args[0]))

def _delete(*args):
    l = mc.ls(type='geometryShape')
    mc.select(l)
    if(len(l) > 0):
        mc.delete()

def _fracture(*args):
    mc.voronoiFracture()

def _addMesh(*args):
    if(str(args[0]) == '***'):
        return       
    elif(str(args[0]) == 'Sphere'):
        mc.polySphere(sx=1, sy=1, r=5)
    elif (str(args[0]) == 'Cube'):
        mc.polyCube(sx=1, sy=1, sz=1, h=5, w=5, d=5)
    elif (str(args[0]) == 'Cylinder'):
        mc.polyCylinder(sx=1, sy=1, sz=1, h=5)
    elif (str(args[0]) == 'Torus'):
        mc.polyTorus(sx=1, sy=1, r=5, sr=2)
    elif (str(args[0]) == 'Plane'):
        mc.polyPlane(sx=1, sy=1, h=5, w=5)

class CreateFractureUI:
    def __init__(self, title, x, y):
        self.SIZE_X = x
        self.SIZE_Y = y
        self.WINDOW_TITLE = title
        self._removeOld()
        self._build()
        
    def _removeOld(self):
        if mc.window("UI", exists=True):
            mc.deleteUI("UI", window=True)
        
    def _build(self):
        UI = mc.window("UI", title = self.WINDOW_TITLE, iconName='test', sizeable=0, width=self.SIZE_X, height=1, minimizeButton=False, maximizeButton=False)
        
        ####### START UI ########
        mc.rowColumnLayout(numberOfRows=2)
        mc.text("VORONOI FRACTURING", align='center', h=self.SIZE_Y*0.2, font="boldLabelFont", backgroundColor=[0.6, 0.8, 0.7])
      
        self.TABS = mc.tabLayout()
        
        ##### Fracture tab
        firstTab = mc.columnLayout()
        mc.tabLayout(self.TABS, edit=True, tabLabel=[firstTab, 'Fracturing'])
        mc.separator(height=20)

        # Info text
        mc.text(label=' Mark a mesh in the scene view or add a mesh in the menu below \n use the fracture button to break the object according to your properties', align='left')
        
        # Multiple options menu
        mc.separator(height=30)
        mc.text(label=" Add a mesh to the scene")
        mc.separator(height=5)
        meshOptionMenu = mc.optionMenu(w=self.SIZE_X*0.8, changeCommand=_addMesh)
        myMesh = mc.menuItem(label="***")
        myMesh = mc.menuItem(label="Sphere")
        myMesh = mc.menuItem(label="Cube")
        myMesh = mc.menuItem(label="Cylinder")
        myMesh = mc.menuItem(label="Torus")
        myMesh = mc.menuItem(label="Plane")
        mc.separator(height=10)
        mc.button('Clear Scene', width=self.SIZE_X*0.2, command = _delete)
        mc.separator(height=30)
        
        # A slider designed to alter PROPERTY 0        
        sliderProp = mc.floatSliderGrp(label=" Property 0", min=0, max=10, field=True, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.2), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.55)])
        mc.floatSliderGrp(sliderProp, e=True, changeCommand = _applySlider)
        
        # A slider designed to alter PROPERTY 1        
        sliderProp1 = mc.floatSliderGrp(label=" Property 1", min=0, max=10, field=True, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.2), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.55)])
        mc.floatSliderGrp(sliderProp1, e=True, changeCommand = _applySlider)
        
        # A slider designed to alter PROPERTY 2      
        sliderProp2 = mc.floatSliderGrp(label=" Property 2", min=0, max=10, field=True, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.2), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.55)])
        mc.floatSliderGrp(sliderProp2, e=True, changeCommand = _applySlider)

        # Apply button
        mc.separator(height=30)
        mc.button('Fracture Mesh', width=self.SIZE_X, command = _fracture)
        mc.setParent("..")
        
        #### Add about us tab
        tmpRowWidth = [self.SIZE_X*0.5, self.SIZE_X*0.5]
        aboutTab = mc.columnLayout()
        mc.tabLayout(self.TABS, edit=True, tabLabel=[aboutTab, 'About us'])
        
        mc.text("About", align='left', font="boldLabelFont")
        mc.separator(height=20)
        
        # Plugin name
        tmpRowWidth = [self.SIZE_X*0.2, self.SIZE_X*0.5]
        mc.rowLayout(numberOfColumns=2, columnWidth2=tmpRowWidth)
        mc.text(label='Plugin Name:', align='left', width=tmpRowWidth[0])
        mc.text(label='Voronoi Fracturing', align='left', width=tmpRowWidth[1])
        mc.setParent('..')
        
        # Plugin authors
        tmpRowWidth = [self.SIZE_X*0.2, self.SIZE_X*0.5]
        mc.rowLayout(numberOfColumns=2, columnWidth2=tmpRowWidth)
        mc.text(label='Authors:', align='left', width=tmpRowWidth[0])
        mc.text(label='Josefine Klintberg & Linus Mossberg', align='left', width=tmpRowWidth[1])
        mc.setParent('..')

        # Info text
        mc.columnLayout()
        mc.text(label='\n')
        mc.text(label='This plugin was created as part of the course SFX - tricks of the trade \nat Linköping University during 2020', align='left', font='smallPlainLabelFont')
        mc.setParent('..')
        
        ## DISPLAY WINDOW
        mc.showWindow(UI)
        
)py";