#pragma once

inline constexpr char py_utilities[] = R"py(

import maya.cmds as mc
import pymel.core as pm

class Util:
    def __init__(self):
        pass

    # Stupid exponential duplicate that is faster than duplicating one by one
    @staticmethod
    def multiDuplicate(obj, num, group_name):
        if num < 1:
            return ""

        objects = mc.duplicate(obj)

        while True:
            remaining = num - len(objects)

            if remaining * 2 > num:
                objects.extend(mc.duplicate(objects))
            else:
                objects.extend(mc.duplicate(objects[0:remaining]))
                break

        return mc.group(objects, name = group_name)

    @staticmethod
    def clipAndCap(obj, n, p):
        angle = mc.angleBetween(euler = True, v1 = [0,0,1], v2 = n)
        mc.polyCut(obj, deleteFaces = True, cutPlaneCenter = p, cutPlaneRotate = angle)
        mc.polyCloseBorder(obj)

    # https://forums.autodesk.com/t5/maya-programming/anti-aliasing-in-viewport-2-0/td-p/7182604
    @staticmethod
    def enableAA():
        hwr = pm.PyNode("hardwareRenderingGlobals")
        try:
            hwr.multiSampleEnable.set(1)
        except:
            print("Couldn't set Anti-aliasing")

    @staticmethod
    def clipAndCapMat(obj, n, p, mat):
        angle = mc.angleBetween(euler = True, v1 = [0,0,1], v2 = n)
        mc.polyCut(obj, deleteFaces = True, cutPlaneCenter = p, cutPlaneRotate = angle)
        
        faces_before = mc.polyEvaluate(obj, face=True)
        mc.polyCloseBorder(obj)
        faces_after = mc.polyEvaluate(obj, face=True)
        new_faces = faces_after - faces_before

        hole_faces = ('%s.f[ %d ]' % (obj, (faces_after + new_faces - 1)))
        mc.sets(hole_faces, forceElement = (mat + 'SG'), e=True)
)py";