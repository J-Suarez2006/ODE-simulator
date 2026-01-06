#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "geometry.hpp"
#include "camera.hpp"

#include <vector>
#include <utility>
#include <cmath>
#include <memory>

SDL_Vertex getSDLVertex(const PointNDC& point);

struct Renderable
{
    std::shared_ptr<Mesh> mesh;
    Transform transform;
};

struct CachedCamera
{
    float far_plane;
    float near_plane;
    float fov;
    Matrix4x4 view_matrix;
};

class Renderer
{
    public:
    Renderer() = delete;
    Renderer(SDL_Window* w, SDL_Renderer* r, Camera* c) 
    : m_window(w), m_renderer(r), camera(c), debug_mode(false)
    {
        int width = 0, height = 0;
        SDL_GetWindowSize(w, &width, &height);
        m_width = width;
        m_height = height;
    }

    inline void RenderPoint(Vertex v, float radius);
    inline void Rendermesh(const Mesh& m);
    inline void RenderObject(const Renderable& r);
    inline void RenderLine(Vertex v1, Vertex v2, float alpha);
    inline void toggleDebug();

    private:
    SDL_Renderer* m_renderer;
    SDL_Window* m_window;
    Camera* camera;
    int m_width;
    int m_height;
    bool debug_mode;

    inline CachedCamera cacheCamera();
    inline PointNDC getNDC(const Vertex& point, const CachedCamera& c);
    inline bool inFrustrum(const Vertex& v, const CachedCamera& c);
    inline bool inScreen(const PointNDC& p);
};


inline void Renderer::toggleDebug()
{
    debug_mode = !debug_mode;
}

inline SDL_FPoint getSDLPt(const PointNDC& point, SDL_Window* w)
{
    int width, height;
    SDL_GetWindowSize(w, &width, &height);

    float x_screen {((point.x() + 1.0f) / 2.0f) * width};
    float y_screen {(1.0f - (1.0f + point.y()) * 0.5f) * height};

    return SDL_FPoint{
        x_screen, y_screen
    };
}

inline SDL_Vertex getSDLVertex(const PointNDC& point, SDL_Window* w)
{
    int width, height;
    SDL_GetWindowSize(w, &width, &height);

    float x_screen {((point.x() + 1.0f) / 2.0f) * width};
    float y_screen {(1.0f - (1.0f + point.y()) * 0.5f) * height};

    return SDL_Vertex{
        {x_screen, y_screen},
        {
            point.color.r(),
            point.color.g(),
            point.color.b(),
            1.0f
        }
    };
}

inline bool Renderer::inFrustrum(const Vertex& v, const CachedCamera& c)
{
    return (
        v.pos.z() <= -c.near_plane && v.pos.z() >= -c.far_plane
    );
};

inline bool Renderer::inScreen(const PointNDC& p)
{
    return (
        p.x() >= -1.0f && p.x() <= 1.0f &&
        p.y() >= -1.0f && p.y() <= 1.0f
    );
}

inline CachedCamera Renderer::cacheCamera()
{
    return CachedCamera{
        camera->far_plane,
        camera->near_plane,
        camera->fov,
        camera->viewMatrix()
    };
}

inline PointNDC Renderer::getNDC(const Vertex& point, const CachedCamera& c)
{
    constexpr float PI = 3.14159263f;

    float aspect_ratio {(float)m_width / (float)m_height};

    float f = 1.0f / std::tan(c.fov * 0.5f * PI / 180.0f);
    float x_ndc {point.pos.x() / point.pos.z() * f / aspect_ratio};
    float y_ndc {point.pos.y() / point.pos.z() * f};

    return PointNDC(Vector2(x_ndc, y_ndc), point.color);
};

inline void Renderer::RenderPoint(Vertex v, float radius)
{
    int num_triangles = 5;
    constexpr float PI = 3.14159263f;

    auto polar = [](float r, float theta) -> std::pair<float, float>
    {
        return std::pair<float, float>{std::cos(theta) * r, std::sin(theta) * r};
    };

    CachedCamera cam_data {cacheCamera()};

    v.pos = cam_data.view_matrix.MatMult(v.pos);

    if (!inFrustrum(v, cam_data)) return;

    PointNDC pt {getNDC(v, cam_data)};

    if (!inScreen(pt)) return;

    SDL_Vertex center = getSDLVertex(pt, m_window);

    for (int i = 0; i < num_triangles; ++i)
    {
        std::pair<float, float> coord_1 = polar(radius, 2 * PI * i / num_triangles);
        std::pair<float, float> coord_2 = polar(radius, 2 * PI * (i + 1) / num_triangles);
        
        SDL_Vertex p2 = SDL_Vertex {
            {center.position.x + coord_1.first, center.position.y + coord_1.second},
            {
                center.color
            }
        };

        SDL_Vertex p3 = SDL_Vertex {
            {center.position.x + coord_2.first, center.position.y + coord_2.second},
            {
                center.color
            }
        };

        SDL_Vertex vertices[] = {center, p2, p3};
        SDL_RenderGeometry(m_renderer, nullptr, vertices, 3, nullptr, 0);
    }
}

inline void Renderer::RenderLine(Vertex v1, Vertex v2, float alpha = 255)
{
    CachedCamera cam_data {cacheCamera()};

    // Global to camera transform
    v1.pos = cam_data.view_matrix.MatMult(v1.pos);
    v2.pos = cam_data.view_matrix.MatMult(v2.pos);
    
    // Frustrum cull 
    if (!inFrustrum(v1, cam_data) && !inFrustrum(v2, cam_data)) return;

    // NDC conversion
    PointNDC p1 {getNDC(v1, cam_data)};
    PointNDC p2 {getNDC(v2, cam_data)};

    SDL_FPoint points[] = {getSDLPt(p1, m_window), getSDLPt(p2, m_window)};

    Uint8 r, g, b, a;

    SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(m_renderer, v1.color.r(), v1.color.g(), v1.color.b(), alpha);

    SDL_RenderLines(m_renderer, points, 2);
    
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}

inline void Renderer::Rendermesh(const Mesh& m)
{
    CachedCamera cam_data {cacheCamera()};
    
    for (int i = 0; i < getMeshLength(m); ++i)
    {
        Vertex v1 {m.vertices[m.indices[i * 3 + 0]]};
        Vertex v2 {m.vertices[m.indices[i * 3 + 1]]};
        Vertex v3 {m.vertices[m.indices[i * 3 + 2]]};

        // Global to camera transform
        v1.pos = cam_data.view_matrix.MatMult(v1.pos);
        v2.pos = cam_data.view_matrix.MatMult(v2.pos);
        v3.pos = cam_data.view_matrix.MatMult(v3.pos);

        // Frustrum cull
        if (!inFrustrum(v1, cam_data) && !inFrustrum(v2, cam_data) && !inFrustrum(v3, cam_data)) continue;

        // NDC conversion
        PointNDC p1 {getNDC(v1, cam_data)};
        PointNDC p2 {getNDC(v2, cam_data)};
        PointNDC p3 {getNDC(v3, cam_data)};

        // Cull triangles fully offscreen
        if (!inScreen(p1) && !inScreen(p2) && !inScreen(p3)) continue;

        // Viewport transform
        SDL_Vertex vertices[] = {getSDLVertex(p1, m_window), getSDLVertex(p2, m_window), getSDLVertex(p3, m_window)};

        // Render
        if (debug_mode)
        {
            SDL_FPoint points[] = {getSDLPt(p1, m_window), getSDLPt(p2, m_window), getSDLPt(p3, m_window)};
            Uint8 r, g, b, a;
            SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(m_renderer, 0, 0, 200, 255);
            SDL_RenderLines(m_renderer, points, 3);
            SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
        }
        else
            SDL_RenderGeometry(m_renderer, nullptr, vertices, 3, nullptr, 0);
    }
}

inline void Renderer::RenderObject(const Renderable& r)
{
    CachedCamera cam_data {cacheCamera()};
    Matrix4x4 transform_matrix = r.transform.transformMatrix();
    const auto& m = *r.mesh;

    for (int i = 0; i < getMeshLength(m); ++i)
    {
        Vertex v1 {m.vertices[m.indices[i * 3 + 0]]};
        Vertex v2 {m.vertices[m.indices[i * 3 + 1]]};
        Vertex v3 {m.vertices[m.indices[i * 3 + 2]]};

        // Local to global transform
        v1.pos = transform_matrix.MatMult(v1.pos);
        v2.pos = transform_matrix.MatMult(v2.pos);
        v3.pos = transform_matrix.MatMult(v3.pos);

        // Global to camera transform
        v1.pos = cam_data.view_matrix.MatMult(v1.pos);
        v2.pos = cam_data.view_matrix.MatMult(v2.pos);
        v3.pos = cam_data.view_matrix.MatMult(v3.pos);

        // Frustrum cull
        if (!inFrustrum(v1, cam_data) && !inFrustrum(v2, cam_data) && !inFrustrum(v3, cam_data)) continue;

        // Clamping z
        v1.pos.setZ(std::min(v1.pos.z(), -.01f));
        v2.pos.setZ(std::min(v2.pos.z(), -.01f));
        v3.pos.setZ(std::min(v3.pos.z(), -.01f));

        // NDC conversion
        PointNDC p1 {getNDC(v1, cam_data)};
        PointNDC p2 {getNDC(v2, cam_data)};
        PointNDC p3 {getNDC(v3, cam_data)};

        // Cull triangles fully offscreen
        if (!inScreen(p1) && !inScreen(p2) && !inScreen(p3)) continue;

        // Viewport transform
        SDL_Vertex vertices[] = {getSDLVertex(p1, m_window), getSDLVertex(p2, m_window), getSDLVertex(p3, m_window)};

        // Render
        if (debug_mode)
        {
            SDL_FPoint points[] = {getSDLPt(p1, m_window), getSDLPt(p2, m_window), getSDLPt(p3, m_window)};
            Uint8 r, g, b, a;
            SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(m_renderer, 0, 0, 200, 255);
            SDL_RenderLines(m_renderer, points, 3);
            SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
        }
        else
            SDL_RenderGeometry(m_renderer, nullptr, vertices, 3, nullptr, 0);
    }
}

#endif