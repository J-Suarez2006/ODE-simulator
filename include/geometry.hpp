#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "math.hpp"

#include <vector>

struct Point2D
{
    int x;
    int y;
    ColorRGB color;
};

struct Vertex
{
    Vector3 pos;
    ColorRGB color;
};

class Transform
{
    public:
    Transform(Vector3 p, Vector3 s, Quaternion r)
    : pos(p), scale(s), rotation(r) {}

    Matrix4x4 translationMatrix() const;
    Matrix4x4 scaleMatrix() const;
    Matrix4x4 rotationMatrix() const;
    Matrix4x4 transformMatrix() const;

    Vector3 pos;
    Vector3 scale;
    Quaternion rotation;
};

inline Matrix4x4 Transform::translationMatrix() const
{
    return Matrix4x4{
        {1, 0, 0, pos.x()},
        {0, 1, 0, pos.y()},
        {0, 0, 1, pos.z()},
        {0, 0, 0, 1}
    };
}

inline Matrix4x4 Transform::scaleMatrix() const
{
    return Matrix4x4{
        {scale.x(), 0, 0, 0},
        {0, scale.y(), 0, 0},
        {0, 0, scale.z(), 0},
        {0, 0, 0, 1}
    };
}

inline Matrix4x4 Transform::rotationMatrix() const
{
    float w = rotation.w(), x = rotation.x(), y = rotation.y(), z = rotation.z();

    return Matrix4x4{
        {1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y), 0},
        {2 * (x * y + w * z), 1 - 2 * (x * x  + z * z), 2 * (y * z - 2 * x), 0},
        {2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y), 0},
        {0, 0, 0, 1},
    };
}

inline Matrix4x4 Transform::transformMatrix() const
{
    return translationMatrix().MatMult(rotationMatrix()).MatMult(scaleMatrix());
}

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

inline int getMeshLength(const Mesh& m)
{
    return m.indices.size() / 3;
}

#endif