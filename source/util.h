#pragma once

#include <vector>
#include <maya/MPlane.h>

std::vector<MPoint> generateUniformPoints(const MPoint& min, const MPoint& max, size_t num);

MPlane getBisectorPlane(const MPoint& p0, const MPoint& p1);