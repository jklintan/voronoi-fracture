#include "util.h"

#include <random>
#include <maya/MPoint.h>

double Plane::signedDistance(const MVector& x)
{
    return normal * (x - point);
}

std::vector<MPoint> generateUniformPoints(const MPoint& min, const MPoint& max, size_t num)
{
    std::mt19937_64 engine(std::random_device{}());
    std::uniform_real_distribution<double> x_dist(min.x, max.x), y_dist(min.y, max.y), z_dist(min.z, max.z);

    std::vector<MPoint> points(num);

    for (auto& p : points)
    {
        p = MPoint(x_dist(engine), y_dist(engine), z_dist(engine));
    }

    return points;
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