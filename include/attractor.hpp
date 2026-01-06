#ifndef ATTRACTOR_HPP
#define ATTRACTOR_HPP

#include "math.hpp"
#include "geometry.hpp"

#include <functional>

struct AttractorPt
{
    Vertex pt;
    std::vector<Vertex> particle_trail;
};

inline Vector3 Lorenz(const Vector3& v)
{
    float sigma = 10.0f;
    float rho = 28.0f;
    float beta = 8.0 / 3;

    float dx = sigma * (v.y() - v.x());
    float dy = v.x() * (rho - v.z()) - v.y();
    float dz = v.x() * v.y() - beta * v.z();

    return Vector3(dx, dy, dz);
}

class Attractor
{
    public:
    Attractor(std::vector<AttractorPt> pts)
    : points(pts), f(Lorenz) {}

    inline void ProcessPhysics(float delta_time);
    std::vector<AttractorPt> points;
    inline void setFunc(std::function<Vector3(const Vector3&)> new_f) {f = new_f;}

    private:

    std::vector<std::function<Vector3(const Vector3&)>> attractor_functions;
    std::function<Vector3(const Vector3&)> f;
};


inline void Attractor::ProcessPhysics(float delta_time)
{
    auto RK4 = [&](Vertex& pt)
    {
        float STEP_SIZE = std::min(delta_time, .01f);

        Vector3 k1 = f(pt.pos);
        Vector3 k2 = f(pt.pos + k1 * .5 * STEP_SIZE);
        Vector3 k3 = f(pt.pos + k2 * .5 * STEP_SIZE);
        Vector3 k4 = f(pt.pos + k3 * STEP_SIZE);

        pt.pos = pt.pos + (k1 + k2 * 2 + k3 * 2 + k4) * (STEP_SIZE / 6);
    };

    for (auto& pt : points)
    {
        pt.particle_trail.push_back(pt.pt);
        RK4(pt.pt);
        if (pt.particle_trail.size() > 100)
        {
            pt.particle_trail.erase(pt.particle_trail.begin());
        }
    }
}

#endif

