#pragma once

inline constexpr char py_utilities[] = R"py(

import maya.cmds as mc

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

        faces = mc.polyEvaluate(obj, face = True)

        if faces > 2:
            mc.polyCloseBorder(obj)
)py";