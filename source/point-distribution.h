#pragma once

#include <vector>
#include <random>

#include <maya/MPoint.h>

namespace PointDistribution
{
    // Random engine
    static std::mt19937_64 engine = std::mt19937_64(std::random_device{}());

    std::vector<MPoint> uniformBoundingBox(const MPoint& min, const MPoint& max, size_t num);

    std::vector<MPoint> radialQuadratic(const MPoint& position, double radius, size_t num);
}
