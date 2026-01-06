#include "geometry.hpp"
#include "math.hpp"

inline Mesh sphereMesh(float radius, int vertices_per_circle)
{
    static constexpr float PI = 3.14159263f;
    ColorRGB default_color(255, 0, 0);

    auto spherical = [](float r, float theta, float phi) -> Vector3
    {
        // Mathematics convention - phi is up/down - theta is around
        // phi is 0 at the top, PI at the bottom. Theta is 0-2PI and rotates around
        return Vector3(
            r * std::cos(theta) * std::sin(phi),
            r * std::sin(theta) * std::sin(phi),
            r * std::cos(phi)
        );
    };

    Mesh sphere;
    sphere.vertices.reserve(vertices_per_circle * vertices_per_circle + 2);

    // Top vertex
    sphere.vertices.emplace_back(
        Vertex{spherical(radius, 0, 0), default_color}
    );

    // Loop
    for (int i = 1; i < vertices_per_circle; ++i)
    {
        float phi = PI * i / float(vertices_per_circle);
        for (int j = 0; j < vertices_per_circle; ++j)
        {
            float theta = 2 * PI * j / float(vertices_per_circle); 
            sphere.vertices.emplace_back(Vertex{spherical(radius, theta, phi), default_color});
        }
    }

    // Bottom vertex
    sphere.vertices.emplace_back(
        Vertex{spherical(radius, 0, PI), default_color}
    );

    // Filling the indices
    
    // Top and bottom cap
    uint32_t top = 0;
    uint32_t bottom = sphere.vertices.size() - 1;
    for (int i = 0; i < vertices_per_circle; ++i)
    {
        int next = (i + 1) % vertices_per_circle;
        
        sphere.indices.insert(sphere.indices.end(), {
            top, top + i + 1, top + next + 1,
            bottom, bottom - vertices_per_circle + next, bottom - vertices_per_circle + i
        });
    }

    
    for (int i = 0; i < vertices_per_circle - 2; ++i)
    {
        for (int j = 0; j < vertices_per_circle; ++j)
        {
            uint32_t top_left = i * vertices_per_circle + j;
            uint32_t top_right = top_left + 1;
            uint32_t bottom_left = (i + 1) * vertices_per_circle + j;
            uint32_t bottom_right = bottom_left + 1;
            
            sphere.indices.insert(sphere.indices.end(), {
                top_left, top_right, bottom_left,
                bottom_left, bottom_right, top_right
            });
        }
    }
    return sphere;
}

inline Mesh cuboid(float l, float h, float w, ColorRGB color = Vector3(0, 255, 0))
{
    Mesh cuboid;

    cuboid.vertices.push_back(Vertex{Vector3(0, 0, 0), color});
    cuboid.vertices.push_back(Vertex{Vector3(l, 0, 0), color});
    cuboid.vertices.push_back(Vertex{Vector3(l, 0, w), color});
    cuboid.vertices.push_back(Vertex{Vector3(0, 0, w), color});
    cuboid.vertices.push_back(Vertex{Vector3(0, h, 0), color});
    cuboid.vertices.push_back(Vertex{Vector3(l, h, 0), color});
    cuboid.vertices.push_back(Vertex{Vector3(l, h, w), color});
    cuboid.vertices.push_back(Vertex{Vector3(0, h, w), color});

    cuboid.indices =
    {
        // bottom
        0, 1, 2, 0, 2, 3,

        // top
        4, 5, 6, 4, 6, 7,

        // back
        3, 2, 6, 3, 6, 7,

        // front
        0, 1, 5, 0, 4, 5,

        // right
        1, 2, 6, 1, 5, 6,

        // left
        0, 3, 7, 0, 4, 7
    };


    return cuboid;
}

inline Mesh gridPlane(int l, int h, float size, ColorRGB color = Vector3(0, 255, 0))
{
    Mesh grid;

    for (int i = 0; i < l; ++i)
    {
        for (int j = 0; j < h; ++j)
        {
            grid.vertices.emplace_back(Vertex{Vector3(i * size, j * size, 0), color});
        }
    }

    for (int i = 0; i < l - 1; ++i)
    {
        for (int j = 0; j < h - 1; ++j)
        {
            uint32_t top_left = i * l + j;
            uint32_t top_right = i * l + j + 1;
            uint32_t bottom_left = (i + 1) * l + j;
            uint32_t bottom_right = (i + 1) * l + j + 1;

            grid.indices.insert(grid.indices.end(), {
                top_left, top_right, bottom_left,
                bottom_left, bottom_right, top_right
            });
        }
    }

    return grid;

}