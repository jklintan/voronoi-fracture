#pragma once

inline constexpr char createFractureUI[] = R"py(

import maya.cmds as mc
from functools import partial

NUM_FRAG_DEFAULT = 5
CURVE_RADIUS_DEFAULT = 0.1
DISK_AXIS_DEFAULT = ""
STEPS_DEFAULT = 0
STEP_NOISE_DEFAULT = 0.05
MIN_DISTANCE_DEFAULT = 0.1

def Diff(li1, li2):
    li_dif = [i for i in li1 + li2 if i not in li1 or i not in li2]
    return li_dif

def Delete(*args):
    # Select all non-locked transforms
    l = mc.ls(transforms = True, readOnly = False) 

    # Get all cameras 
    cameras = cmds.ls(type=('camera'), l=True)
    # Filter all startup cameras that should not be deleted
    startup_cameras = [camera for camera in cameras if cmds.camera(cmds.listRelatives(camera, parent=True)[0], startupCamera=True, q=True)]
    startup_cameras_transforms = map(lambda x: cmds.listRelatives(x, parent=True)[0], startup_cameras)
    
    # Select all transforms that are deletable
    selection = Diff(l, startup_cameras_transforms)
    mc.select(selection)
    if(len(selection) > 0):
        mc.delete()

def AddMesh(*args):
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

def AddImplicit(*args):
    existing = mc.ls('implicitSphere')
    if(len(existing) > 0):
        return 0
    else:
        mc.createNode('implicitSphere', n='implicitSphere')

def OpenImportMenu(*args):
    mc.Import()

def UseCurve(*args):
    # Select all non-locked transforms
    selection = mc.ls(readOnly = False, type="mesh") 
   
    print(selection)
    mc.select(selection)
    if(len(selection) > 0):
        mc.select(selection[0])
        mc.EPCurveTool()

def UseParticleSystem(*args):
    mc.NCreateEmitter()

class CreateFractureUI:
    def __init__(self, title, x, y):
        self.SIZE_X = x
        self.SIZE_Y = y
        self.WINDOW_TITLE = title
        self.NUM_FRAGMENTS = NUM_FRAG_DEFAULT
        self.CURVE_RADIUS = CURVE_RADIUS_DEFAULT
        self.DISK_AXIS = DISK_AXIS_DEFAULT
        self.STEPS = STEPS_DEFAULT
        self.STEP_NOISE = STEP_NOISE_DEFAULT
        self.MIN_DISTANCE = MIN_DISTANCE_DEFAULT
        self._removeOld()
        self._build()

    def _applySlider(self, prop, val, *args):
        setattr(self, prop, val)

    def _fracture(self,*args):
        selection = []
        mc.select(selection)
        existingMesh = mc.ls(type="mesh", readOnly = False)

        if(len(existingMesh) > 0):
            selection.append(existingMesh[0])
            existingImplicit = mc.ls('implicitSphere')
            if(len(existingImplicit) > 0):
                if(self.DISK_AXIS != ""):
                    selection.append(existingImplicit[0])

        mc.select(selection) 
        if(len(selection) > 0):
            mc.voronoiFracture(nf = self.NUM_FRAGMENTS, s = self.STEPS, sn = self.STEP_NOISE, da = self.DISK_AXIS, cr=self.CURVE_RADIUS, md=self.MIN_DISTANCE)

    def _radioButtonUpdate(self, prop, button, val, *args):
        activeButton = 1
        if(val == True):
            activeButton = button;
        if(activeButton == 1):
            setattr(self, prop, "")
        elif(activeButton == 2):
            setattr(self, prop, "x")
        elif(activeButton == 3):
            setattr(self, prop, "y")
        elif(activeButton == 4):
            setattr(self, prop, "z")
        
    def _removeOld(self):
        if mc.window("UI", exists=True):
            mc.deleteUI("UI", window=True)
        
    def _build(self):
        UI = mc.window("UI", title = self.WINDOW_TITLE, iconName='test', sizeable=0, width=self.SIZE_X, height=1, minimizeButton=False, maximizeButton=False)
        
        ####### START UI ########
        mc.rowColumnLayout(numberOfRows=2)
        mc.text("VORONOI FRACTURING", align='center', h=self.SIZE_Y*0.2, font="boldLabelFont", backgroundColor=[0.2, 0.2, 0.2])
      
        self.TABS = mc.tabLayout()
        
        ##### Fracture tab
        firstTab = mc.columnLayout()
        mc.tabLayout(self.TABS, edit=True, tabLabel=[firstTab, 'Fracturing'])

        # Info text
        mc.separator(height=20)
        mc.text(label=' Mark a mesh in the scene view or add a mesh in the menu below \n use the fracture button to break the object according to your properties', align='left')
        
        # A slider designed to alter number of fragments     
        mc.separator(height=20)
        mc.separator( style='in', width=self.SIZE_X, height=20)

        # Multiple options menu
        mc.text(label=" Add objects to the scene", font='boldLabelFont')
        mc.separator(height=5)
        meshOptionMenu = mc.optionMenu(w=self.SIZE_X, changeCommand=AddMesh)
        myMesh = mc.menuItem(label="***")
        myMesh = mc.menuItem(label="Sphere")
        myMesh = mc.menuItem(label="Cube")
        myMesh = mc.menuItem(label="Cylinder")
        myMesh = mc.menuItem(label="Torus")
        myMesh = mc.menuItem(label="Plane")
        mc.separator(height=5)

        # Add implicit and clear scene buttons
        tmpRowWidth = [self.SIZE_X*0.2, self.SIZE_X*0.2, self.SIZE_X*0.2, self.SIZE_X*0.2]
        mc.rowLayout(numberOfColumns=4, columnWidth4=tmpRowWidth)
        mc.button('Load object', width=tmpRowWidth[0], command = OpenImportMenu)
        mc.button('Add implicit sphere', width=tmpRowWidth[1], command = AddImplicit)
        mc.button('Use curves', width=tmpRowWidth[2], command = UseCurve)
        mc.button('Use particle system', width=tmpRowWidth[3], command = UseParticleSystem)
        mc.setParent('..')

        # Property sliders
        mc.separator(height=10)
        mc.separator( style='in', width=self.SIZE_X, height=20)
        mc.text(' Basic properties', font='boldLabelFont')
        mc.separator(height=10)
        fragProp = mc.intSliderGrp(label=" Number of Fragments", value = self.NUM_FRAGMENTS, min=2, max=10000, field=True, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.3), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.45)])
        mc.intSliderGrp(fragProp, e=True, changeCommand = partial(self._applySlider, 'NUM_FRAGMENTS'))
 
        mc.separator(height=10)
        mc.separator( style='in', width=self.SIZE_X, height=20)
        mc.text(' Advanced properties, require implicit node added to scene', font='boldLabelFont')
        mc.separator(height=20)
        
        # A slider designed to alter curve radius
        sliderProp1 = mc.floatSliderGrp(label=" Curve radius", min=0.01, max=1, value = self.CURVE_RADIUS, field=True, step=0.01, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.3), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.45)])
        mc.floatSliderGrp(sliderProp1, e=True, changeCommand = partial(self._applySlider, 'CURVE_RADIUS'))

        # A slider designed to alter disk steps
        sliderProp3 = mc.intSliderGrp(label=" Steps", value = self.STEPS, min=0, max=100, field=True, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.3), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.45)])
        mc.intSliderGrp(sliderProp3, e=True, changeCommand = partial(self._applySlider, 'STEPS'))

        # A slider designed to alter step noise 
        sliderProp4 = mc.floatSliderGrp(label=" Step noise", min=0, max=1, value = self.STEP_NOISE, step=0.01, field=True, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.3), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.45)])
        mc.floatSliderGrp(sliderProp4, e=True, changeCommand = partial(self._applySlider, 'STEP_NOISE'))

        # A slider designed to alter step noise 
        sliderProp2 = mc.floatSliderGrp(label=" Min Distance", min=0.01, max=10, value = self.MIN_DISTANCE, step=0.01, field=True, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.3), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.45)])
        mc.floatSliderGrp(sliderProp2, e=True, changeCommand = partial(self._applySlider, 'MIN_DISTANCE'))

        # Radio buttons for disk axis
        mc.separator(height=5)
        btns = mc.radioButtonGrp(label=' Disk axis', select = 0, changeCommand1 = partial(self._radioButtonUpdate, 'DISK_AXIS', 1), changeCommand2= partial(self._radioButtonUpdate, 'DISK_AXIS', 2), changeCommand3 = partial(self._radioButtonUpdate, 'DISK_AXIS', 3), changeCommand4 = partial(self._radioButtonUpdate, 'DISK_AXIS', 4), labelArray4=['None', 'x', 'y', 'z'], width = self.SIZE_X, numberOfRadioButtons=4, columnAlign=(1,'left'), cw=[(1, self.SIZE_X*0.3), (2, self.SIZE_X*0.2), (3, self.SIZE_X*0.15), (4, self.SIZE_X*0.15)])

        # Apply button
        mc.separator( style='in', width=self.SIZE_X, height=20)
        tmpRowWidth = [self.SIZE_X*0.5, self.SIZE_X*0.5]
        mc.rowLayout(numberOfColumns=2, columnWidth2=tmpRowWidth)
        mc.button('Fracture Mesh', l ='Fracture Mesh', width=tmpRowWidth[0], command = self._fracture, backgroundColor=[0.1, 0.3, 0.1], height=40)
        mc.button('Clear Scene', width=tmpRowWidth[1], command = Delete, backgroundColor=[0.3, 0.1, 0.1], height=40)
        mc.setParent("..")
        mc.setParent("..")
        
        #### Add about us tab
        tmpRowWidth = [self.SIZE_X*0.5, self.SIZE_X*0.5]
        aboutTab = mc.columnLayout()
        mc.tabLayout(self.TABS, edit=True, tabLabel=[aboutTab, 'About us'])
       
        mc.separator(height=20)
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
        mc.separator(height=10)
        mc.separator( style='in', width=self.SIZE_X, height=20)
        
        ## DISPLAY WINDOW
        mc.showWindow(UI)
        
)py";