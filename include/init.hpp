#ifndef INIT_HPP
#define INIT_HPP

#include <SDL3/SDL.h>

#include <string>

#include "rast.hpp"

inline bool init(SDL_Window*& w, SDL_Renderer*& r, std::string window_name = "Rasterizer")
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Could not initialize video! SDL error: %s\n", SDL_GetError());
        return false;
    }

    w = SDL_CreateWindow(
        window_name.c_str(),
        RAST::SCREEN_WIDTH,
        RAST::SCREEN_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );

    if (w == nullptr)
    {
        SDL_Log("Could not initialize window! SDL error: %s\n", SDL_GetError());
        return false;
    }

    r = SDL_CreateRenderer(w, NULL);

    if (r == nullptr)
    {
        SDL_Log("Could not initialize SDL renderer! SDL error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

inline void close(SDL_Window*& w, SDL_Renderer*& r)
{
    SDL_DestroyRenderer(r);
    r = nullptr;

    SDL_DestroyWindowSurface(w);
    w = nullptr;

    SDL_Quit();
}
#endif