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

MVector orthogonalUnitVector(const MVector& v);

// From https://stackoverflow.com/a/26221725
template<typename ... Args>
MString formatString(const std::string& format, Args ... args)
{
    int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return buf.get();
}