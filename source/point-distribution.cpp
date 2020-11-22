#include "point-distribution.h"

std::vector<MPoint> PointDistribution::uniformBoundingBox(const MPoint& min, const MPoint& max, size_t num)
{
    std::uniform_real_distribution<double> x_dist(min.x, max.x), y_dist(min.y, max.y), z_dist(min.z, max.z);

    std::vector<MPoint> points(num);

    for (auto& p : points)
    {
        p = MPoint(x_dist(engine), y_dist(engine), z_dist(engine));
    }

    return points;
}

std::vector<MPoint> PointDistribution::radialQuadratic(const MPoint& position, double radius, size_t num)
{
    std::uniform_real_distribution<double> theta_dist(0.0, M_PI);
    std::uniform_real_distribution<double> phi_dist(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<double> r_dist(0.0, radius);

    std::vector<MPoint> points(num, position);

    for (auto& p : points)
    {
        double theta = theta_dist(engine);
        double phi = phi_dist(engine);
        double r = r_dist(engine);

        p += MPoint(
            r * std::sin(theta) * std::cos(phi),
            r * std::sin(theta) * std::sin(phi),
            r * std::cos(theta)
        );
    }

    return points;
}