#pragma once

#include <vector>
#include <maya/MVector.h>

struct Plane
{
    Plane(const MVector& n, const MVector& p) : normal(n.normal()), point(p) { }

    double signedDistance(const MVector& x);

    const MVector normal, point;
};

std::vector<MPoint> generateUniformPoints(const MPoint& min, const MPoint& max, size_t num);

Plane getBisectorPlane(const MPoint& p0, const MPoint& p1);