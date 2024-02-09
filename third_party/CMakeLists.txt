add_library(glad STATIC)
target_sources(glad PRIVATE
    GLAD/glad.c)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

add_library(stb_image STATIC)
target_sources(stb_image PRIVATE
    stb_image/image.cpp)
target_include_directories(stb_image INTERFACE stb_image)

add_library(imgui STATIC)
target_sources(imgui PRIVATE
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_stdlib.cpp

    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_sdl.cpp)
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PUBLIC SDL2::SDL2)

add_library(imGuizmo STATIC)
target_sources(imGuizmo PRIVATE
    ImGuizmo/ImGuizmo.h
    ImGuizmo/ImGuizmo.cpp)
target_include_directories(imGuizmo PUBLIC imGuizmo)
target_link_libraries(imGuizmo PUBLIC imgui)