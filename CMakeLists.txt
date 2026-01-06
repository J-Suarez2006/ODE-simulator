cmake_minimum_required(VERSION 3.26)
project(main C CXX)

if (MSVC)
    add_compile_options(/bigobj)
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_package(SDL3 REQUIRED)

add_library(imgui
    include/imgui/imgui.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/imgui_demo.cpp
    include/imgui/backends/imgui_impl_sdl3.cpp
    include/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui PUBLIC include/imgui include/imgui/backends)

target_link_libraries(imgui PUBLIC SDL3::SDL3)

add_executable(main)

target_sources(main
PRIVATE
    main.cpp
)

target_link_libraries(main imgui SDL3::SDL3)
