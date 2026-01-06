#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "exprtk.hpp"

#include "include/rasterizer.hpp"
#include "include/attractor.hpp"

#include <memory>
#include <cstdint>
#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cstring>

#include <random>

std::mt19937 gen(4);

std::function<Vector3(const Vector3&)> getDV(const char* dx, const char* dy, const char* dz)
{
    struct ExprData
    {
        float x, y, z;
        exprtk::symbol_table<float> symbols;
        exprtk::expression<float> dx_dt, dy_dt, dz_dt;
        exprtk::parser<float> parser;

        ExprData(const char* dx_exp, const char* dy_exp, const char* dz_exp)
        {
            symbols.add_variable("x", x);
            symbols.add_variable("y", y);
            symbols.add_variable("z", z);
            symbols.add_constants();

            dx_dt.register_symbol_table(symbols);
            dy_dt.register_symbol_table(symbols);
            dz_dt.register_symbol_table(symbols);

            parser.compile(dx_exp, dx_dt);
            parser.compile(dy_exp, dy_dt);
            parser.compile(dz_exp, dz_dt);
        }

        Vector3 eval(const Vector3& v)
        {
            x = v.x();
            y = v.y();
            z = v.z();
            return Vector3(dx_dt.value(), dy_dt.value(), dz_dt.value());
        }
    };

    auto data = std::make_shared<ExprData>(dx, dy, dz);


    return [data](const Vector3& v) mutable -> Vector3 
    {
        return data->eval(v);
    };
}

std::vector<AttractorPt> generatePoints(std::uniform_real_distribution<float>& dist, int amt = 20, float initx = 0, float inity = 0, float initz = 0)
{
    std::vector<AttractorPt> points;

    for (int i = 0; i < amt; ++i)
    {
        AttractorPt pt;
        pt.pt.color = {255, 0, 0};
        pt.pt.pos = {initx + dist(gen), inity + dist(gen), initz + dist(gen)};

        points.push_back(pt);
    }
    return points;
}

int main()
{
    std::uniform_real_distribution<float> distribution(-.1, .1);

    SDL_Window* window {nullptr};
    SDL_Renderer* renderer {nullptr};

    if(!init(window, renderer, "Chaotic Attractors"))
    {
        SDL_Log("Could not initialize! SDL Error: %s\n", SDL_GetError());
    }

    bool running {true};
    SDL_Event e;

    Camera camera (
        Vector3(37, 0, 6.5),
        80.0f,
        RAST::SCREEN_WIDTH/(float)RAST::SCREEN_HEIGHT,
        Quaternion(1, 0, -.5, 0)
    );

    Attractor a(generatePoints(distribution));

    Renderer m_renderer(window, renderer, &camera);
    m_renderer.toggleDebug();

    Mesh grid = gridPlane(20, 20, 10);
    float offset = 40 * 6 / float(2);

    Renderable grid1 {std::make_shared<Mesh>(grid), 
        Transform{
            Vector3(-offset, -offset, 0),
            Vector3(1, 1, 1),
            Quaternion(1, 0, 0, 0)
        }
    };

    Renderable grid2 {std::make_shared<Mesh>(grid), 
        Transform{
            Vector3(0, -offset, offset),
            Vector3(1, 1, 1),
            Quaternion::fromAxisAngle(Vector3(0, 1, 0), 90)
        }
    };
    
    Renderable grid3 {std::make_shared<Mesh>(grid), 
        Transform{
            Vector3(-offset, 0, -offset),
            Vector3(1, 1, 1),
            Quaternion::fromAxisAngle(Vector3(1, 0, 0), 90)
        }
    };
    

    /* ImGui setup */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Styling
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Backend setup
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    Uint64 last_tick = SDL_GetPerformanceCounter();

    bool display_grid_lines {false};
    while (running)
    {
        Uint64 current_tick = SDL_GetPerformanceCounter();
        float delta_time = float(current_tick - last_tick) / SDL_GetPerformanceFrequency();
        last_tick = current_tick;

        while(SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (e.type == SDL_EVENT_KEY_DOWN)
            {
                if (e.key.key == SDLK_0)
                {
                    std::cout << camera.position;
                    std::cout << camera.orientation;
                }
            }
        }

        const bool *keyStates = SDL_GetKeyboardState(nullptr);
        takeInput(keyStates, camera, delta_time, 10, 70);

        /* ImGui Rendering */
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        /* Viewport/Docking */
        /*
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        */


        {
            ImGui::Begin("ODE Simulator");
            
            static char bufferdx[256] = "-y";
            ImGui::InputText("dx/dt", bufferdx, sizeof(bufferdx));

            static char bufferdy[256] = "x";
            ImGui::InputText("dy/dt", bufferdy, sizeof(bufferdy));

            static char bufferdz[256] = "0.1";
            ImGui::InputText("dz/dt", bufferdz, sizeof(bufferdz));

            ImGui::Text("Point Drop Point:");
            static char xpos[64] = "0";
            ImGui::InputText("x position", xpos, sizeof(xpos));

            static char ypos[64] = "0";
            ImGui::InputText("y position", ypos, sizeof(ypos));

            static char zpos[64] = "0";
            ImGui::InputText("z position", zpos, sizeof(zpos));

            if (ImGui::Button("Simulate"))
            {
                float x = std::stof(xpos);
                float y = std::stof(ypos);
                float z = std::stof(zpos);

                std::function<Vector3(const Vector3&)> f = getDV(bufferdx, bufferdy, bufferdz);
                a.points = generatePoints(distribution, 20, x, y, z);
                a.setFunc(f);
            }

            ImGui::BeginChild("Attractor_Presets", ImVec2(200, 0), true);

            ImGui::Text("Presets:");
            if (ImGui::Button("Lorenz"))
            {
                std::strcpy(bufferdx, "10 * (y - x)");
                std::strcpy(bufferdy, "x * (28 - z) - y");
                std::strcpy(bufferdz, "x * y - 2.66667 * z");
            }
            if (ImGui::Button("Dadras"))
            {
                std::strcpy(bufferdx, "y - 3 * x + 2.7 * y * z");
                std::strcpy(bufferdy, "1.7 * y - x * z + z");
                std::strcpy(bufferdz, "2 * x * y - 9 * z");
            }
            if (ImGui::Button("Rossler"))
            {
                std::strcpy(bufferdx, "-(y + z)");
                std::strcpy(bufferdy, "x + 0.2 * y");
                std::strcpy(bufferdz, "0.2 + z * (x - 5.7)");
            }
            if (ImGui::Button("Four Wing"))
            {
                std::strcpy(bufferdx, "0.2 * x + y * z");
                std::strcpy(bufferdy, "0.01 * x + -0.4 * y - x * z");
                std::strcpy(bufferdz, "-z - x * y");
            }
            if (ImGui::Button("Chen"))
            {
                std::strcpy(bufferdx, "5 * x - y * z");
                std::strcpy(bufferdy, "-10 * y + x * z");
                std::strcpy(bufferdz, "-0.38 * z + y * x * .333333");
            }
            if (ImGui::Button("Aizawa"))
            {
                std::strcpy(bufferdx, "(z - 0.7) * x - 3.5 * y");
                std::strcpy(bufferdy, "3.5 * x + (z - 0.7) * y");
                std::strcpy(bufferdz, "(x * x + y * y) * (1 + 0.25 * z) + 0.1 * z * x * x * x");
            }

            ImGui::EndChild();

            ImGui::BeginChild("Attractor_Presets", ImVec2(200, 0), true);
            ImGui::Text("Settings:");
            if (ImGui::Button("Toggle Grid Lines")) display_grid_lines = !display_grid_lines;
            ImGui::EndChild();

            ImGui::End();
        }
        

        ImGui::Render();

        /* Normal stuff */

        SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        a.ProcessPhysics(delta_time);

        ColorRGB color1 (255, 0, 0);
        ColorRGB color2 (214, 118, 228);
        for (auto point : a.points)
        {
            m_renderer.RenderPoint(point.pt, 2);

            for (int i = 0; i < point.particle_trail.size() - 1; ++i)
            {
                float t1 = float(i) / 100;
                float t2 = float(i + 1) / 100;

                Vertex v1 = {
                    point.particle_trail[i].pos,
                    color1 * (1 - t1) + color2 * t1
                };

                Vertex v2 = {
                    point.particle_trail[i + 1].pos,
                    color1 * (1 - t2) + color2 * t2
                };

                float alpha = 255 * t1;

                m_renderer.RenderLine(v1, v2, alpha);
            }
        }
        
        if (display_grid_lines){
            m_renderer.RenderObject(grid1);
            m_renderer.RenderObject(grid2);
            m_renderer.RenderObject(grid3);
        }

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    close(window, renderer);

    ImGui::DestroyPlatformWindows();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}