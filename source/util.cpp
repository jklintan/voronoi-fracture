#include "util.h"

#include <random>
#include <maya/MPoint.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshVertex.h>
#include <maya/MDagPath.h>

double Plane::signedDistance(const MVector& x) const
{
    return normal * (x - point);
}

bool Plane::intersects(const MFnMesh& mesh, bool& strictly_greater) const
{
    bool greater = false, less = false;
    strictly_greater = false;
    for (MItMeshVertex it(mesh.dagPath()); !it.isDone(); it.next())
    {
        MStatus status;
        if (signedDistance(it.position(MSpace::kWorld)) > 0.0)
            greater = true;
        else
            less = true;

        if (greater && less) return true;
    }
    strictly_greater = greater && !less;
    return false;
}

Plane getBisectorPlane(const MPoint& p0, const MPoint& p1)
{
    return Plane(p1 - p0, (p0 + p1) * 0.5);
}

MVector orthogonalUnitVector(const MVector& v)
{
    if (std::abs(v.x) > std::abs(v.y))
        return MVector(-v.z, 0, v.x) / std::sqrt(v.x * v.x + v.z * v.z);
    else
        return MVector(0, v.z, -v.y) / std::sqrt(v.y * v.y + v.z * v.z);
}