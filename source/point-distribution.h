#pragma once

#include <vector>
#include <random>

#include <maya/MPoint.h>

namespace PointDistribution
{
    std::vector<MPoint> uniformBoundingBox(const MPoint& min, const MPoint& max, size_t num);

    std::vector<MPoint> radialQuadratic(const MPoint& position, const std::array<MVector, 3>& axes, size_t num);

    std::vector<MPoint> diskQuadratic(const MPoint& position, const std::array<MVector, 3>& axes, size_t axis, size_t num);

    std::vector<MPoint> diskSteps(const MPoint& position, const std::array<MVector, 3>& axes, size_t steps, size_t axis, double noise_sigma, size_t num);

    std::vector<MPoint> curve(const MFnNurbsCurve &curve, double radius, size_t num);

    // Random engine
    inline std::mt19937_64 engine = std::mt19937_64(std::random_device{}());

    // Common distributions
    inline const std::uniform_real_distribution<double> theta_dist(0.0, M_PI);
    inline const std::uniform_real_distribution<double> phi_dist(0.0, 2.0 * M_PI);
    inline const std::uniform_real_distribution<double> unit_dist(0.0, 1.0);
}
