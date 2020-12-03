#pragma once

#include <vector>
#include <maya/MVector.h>
#include <maya/MGlobal.h>

struct Plane
{
    Plane(const MVector& n, const MVector& p) : normal(n.normal()), point(p) { }

    double signedDistance(const MVector& x) const;
    bool intersects(const MFnMesh& mesh, bool& strictly_greater) const;

    MVector normal, point;
};

Plane getBisectorPlane(const MPoint& p0, const MPoint& p1);

MVector orthogonalUnitVector(const MVector& v);

template<class T>
void displayNumber(const T& number)
{
    MGlobal::displayInfo(std::to_string(number).c_str());
}

// From https://stackoverflow.com/a/26221725
template<typename ... Args>
MString formatString(const std::string& format, Args ... args)
{
    int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return buf.get();
}