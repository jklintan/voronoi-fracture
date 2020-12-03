#include "point-distribution.h"

#include <array>

#include <maya/MFnNurbsCurve.h>
#include <maya/MVector.h>
#include <maya/MQuaternion.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MVectorArray.h>
#include "util.h"

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

std::vector<MPoint> PointDistribution::sphereQuadratic(const MPoint& position, const std::array<MVector, 3> &axes, size_t num)
{
    std::vector<MPoint> points(num, position);

    for (auto& p : points)
    {
        double theta = theta_dist(engine);
        double phi = phi_dist(engine);
        double r = unit_dist(engine);

        p += std::sin(theta) * std::cos(phi) * axes[0] * r;
        p += std::sin(theta) * std::sin(phi) * axes[1] * r;
        p += std::cos(theta) * axes[2] * r;
    }

    return points;
}

std::vector<MPoint> PointDistribution::diskQuadratic(const MPoint& position, const std::array<MVector, 3>& axes, size_t axis, size_t num)
{
    unsigned idx = axis - 1;
    unsigned idx0 = std::min(idx - 1u, 2u); // relies on unsigned wrap-around
    unsigned idx1 = idx + 1 > 2 ? 0 : idx + 1;

    std::vector<MPoint> points(num, position);

    for (auto& p : points)
    {
        double phi = phi_dist(engine);
        double r = unit_dist(engine);

        p += std::cos(phi) * r * axes[idx0];
        p += std::sin(phi) * r * axes[idx1];
    }

    return points;
}

std::vector<MPoint> PointDistribution::diskSteps(const MPoint& position, const std::array<MVector, 3>& axes, size_t steps, size_t axis, double noise_sigma, size_t num)
{
    size_t step_points = num / steps;
    unsigned idx = axis - 1;
    unsigned idx0 = std::min(idx - 1u, 2u);
    unsigned idx1 = idx + 1 > 2 ? 0 : idx + 1;

    std::normal_distribution<double> noise(1, noise_sigma);

    std::vector<MPoint> points(step_points * steps, position);

    size_t p_idx = 0;
    for (size_t s = 1; s <= steps; s++)
    {
        double r = s / (double)steps;
        for (size_t sp = 0; sp < step_points; sp++)
        {
            double phi = (sp / (double)step_points) * 2.0 * M_PI;

            auto& p = points[p_idx++];
            p += std::cos(phi) * r * noise(engine) * axes[idx0];
            p += std::sin(phi) * r * noise(engine) * axes[idx1];
        }
    }

    return points;
}

std::vector<MPoint> PointDistribution::sphereSteps(const MPoint& position, const std::array<MVector, 3>& axes, size_t steps, double noise_sigma, size_t num)
{
    // num / steps = phi_steps * theta_steps = phi_steps * phi_steps * 0.5 <=>
    double d_phi_steps = std::sqrt(2.0 * num / steps);
    size_t phi_steps = (size_t)d_phi_steps;
    size_t theta_steps = (size_t)(d_phi_steps * 0.5);

    std::normal_distribution<double> noise(1, noise_sigma);

    std::vector<MPoint> points(steps * phi_steps * theta_steps, position);

    phi_steps++;
    theta_steps++;

    size_t p_idx = 0;
    for (size_t step_idx = 1; step_idx <= steps; step_idx++)
    {
        double r = step_idx / (double)steps;
        for (size_t phi_idx = 1; phi_idx < phi_steps; phi_idx++)
        {
            double phi = 2.0 * M_PI * (phi_idx / (double)phi_steps);
            for (size_t theta_idx = 1; theta_idx < theta_steps; theta_idx++)
            {
                double theta = M_PI * (theta_idx / (double)theta_steps);
                auto& p = points.at(p_idx++);
                p += std::sin(theta) * std::cos(phi) * axes[0] * r * noise(engine);
                p += std::sin(theta) * std::sin(phi) * axes[1] * r * noise(engine);
                p += std::cos(theta) * axes[2] * r * noise(engine);
            }
        }
    }

    return points;
}

std::vector<MPoint> PointDistribution::curve(const MFnNurbsCurve& curve, double radius, size_t num)
{
    std::uniform_real_distribution<double> length_dist(0.0, curve.length(1e-6));

    std::vector<MPoint> points(num);

    for (auto& p : points)
    {
        double t = curve.findParamFromLength(length_dist(engine));
        MPoint point;
        curve.getPointAtParam(t, point, MSpace::kWorld);

        MVector tangent = curve.tangent(t);
        MVector normal = orthogonalUnitVector(tangent);

        MQuaternion quat(phi_dist(engine), tangent);
        MVector direction = normal.rotateBy(quat);

        double r = unit_dist(engine) * radius;

        p = point + direction * r;
    }

    return points;
}

std::vector<MPoint> PointDistribution::particles(const MFnParticleSystem& particles)
{
    MVectorArray positions;
    particles.position(positions);

    std::vector<MPoint> points(positions.length());

    for (unsigned int i = 0; i < positions.length(); i++)
    {
        points[i] = positions[i];
    }

    return points;
}

std::vector<MPoint> PointDistribution::removeDuplicates(const std::vector<MPoint>& points, double tolerance)
{
    std::vector<MPoint> new_points;

    for (const auto& p0 : points)
    {
        bool add = true;
        for (const auto& p1 : new_points)
        {
            if (p0.distanceTo(p1) < tolerance)
            {
                add = false;
                break;
            }
        }
        if (add) new_points.push_back(p0);
    }

    return new_points;
}