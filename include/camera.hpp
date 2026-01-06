#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "math.hpp"
#include "rast.hpp"
#include "geometry.hpp"

struct PointNDC
{
    PointNDC() : pos(Vector2()), color(ColorRGB{}) {}
    PointNDC(Vector2 p, ColorRGB c) : pos(p), color(c) {}

    Vector2 pos;
    ColorRGB color;

    inline float x() const {return pos.x();}
    inline float y() const {return pos.y();} 
};

class Camera
{
    public:
    Vector3 position;
    Quaternion orientation {1, 0, 0, 0};
    float fov;
    float aspect_ratio;
    float near_plane;
    float far_plane;


    Camera(Vector3 pos, float fov, float ar, Quaternion orient = Quaternion(1, 0, 0, 0), float np = 0.1, float fp = 1000.0)
    : position(pos), fov(fov), aspect_ratio(ar), near_plane(np), far_plane(fp), orientation(orient) {}

    inline Matrix4x4 viewMatrix();
    inline PointNDC getNDC(Vertex point);

    inline void pitch(float angle);
    inline void yaw(float angle);
    inline Vector3 forward(){
        return rotate({0, 0, 1}, orientation);
    }
};

inline Matrix4x4 Camera::viewMatrix()
{
    Vector3 forward {rotate({0, 0, 1}, orientation)};
    Vector3 up {rotate({0, 1, 0}, orientation)};
    Vector3 right {forward.cross(up)};

    Matrix4x4 rotation {
        {right.x(),      right.y(),        right.z(),            0.0},
        {up.x(),           up.y(),             up.z(),                 0.0},
        {-forward.x(),     -forward.y(),       -forward.z(),           0.0},
        {0.0,              0.0,                0.0,                    1.0}
    };

    Matrix4x4 translation {
        {1.0,       0.0,        0.0,        -position.x()},
        {0.0,       1.0,        0.0,        -position.y()},
        {0.0,       0.0,        1.0,        -position.z()},
        {0.0,       0.0,        0.0,        1.0}
    };

    return rotation.MatMult(translation);
}

inline PointNDC Camera::getNDC(Vertex point)
{
    constexpr float PI = 3.14159263f;
    
    // Camera transform
    Vertex camera_point {viewMatrix().MatMult(point.pos), point.color};

    // NDC transform
    float f = 1.0f / std::tan(fov * 0.5f * PI / 180.0f);
    float x_ndc {camera_point.pos.x() / camera_point.pos.z() * f / aspect_ratio};
    float y_ndc {camera_point.pos.y() / camera_point.pos.z() * f};

    return PointNDC(Vector2(x_ndc, y_ndc), camera_point.color);
}

inline void Camera::pitch(float angle)
{
    Quaternion pitchQ {Quaternion::fromAxisAngle(Vector3(1, 0, 0), angle)};
    orientation = orientation * pitchQ;
}

inline void Camera::yaw(float angle)
{
    Quaternion yawQ {Quaternion::fromAxisAngle(Vector3(0, 1, 0), angle)};
    orientation = yawQ * orientation;
}

inline void takeInput(const bool *keyStates, Camera& camera, float time_step, float speed, float turn_speed)
{
    Vector3 forward {rotate({0, 0, 1}, camera.orientation)};
    Vector3 up {rotate({0, 1, 0}, camera.orientation)};
    Vector3 right {forward.cross(up)};

    if (keyStates[SDL_SCANCODE_UP])
    {
        camera.position += forward * time_step * speed;
    }
    if (keyStates[SDL_SCANCODE_DOWN])
    {
        camera.position -= forward * time_step * speed;
    }
    if (keyStates[SDL_SCANCODE_LEFT])
    {
        camera.position += right * time_step * speed;
    }
    if (keyStates[SDL_SCANCODE_RIGHT])
    {
        camera.position -= right * time_step * speed;
    }
    if (keyStates[SDL_SCANCODE_SPACE])
    {
        camera.position -= up * time_step * speed;
    }
    if (keyStates[SDL_SCANCODE_LSHIFT])
    {
        camera.position += up * time_step * speed;
    }
    if (keyStates[SDL_SCANCODE_J])
    {
        camera.yaw(-turn_speed * time_step);
    }
    if (keyStates[SDL_SCANCODE_L])
    {
        camera.yaw(turn_speed * time_step);
    }
    if (keyStates[SDL_SCANCODE_I])
    {
        camera.pitch(turn_speed * time_step);
    }
    if (keyStates[SDL_SCANCODE_K])
    {
        camera.pitch(-turn_speed * time_step);
    }
}

#endif