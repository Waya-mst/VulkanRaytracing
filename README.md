## libs
libs’¼‰º‚Éimgui‚ğclone
libs/imgui“à‚ÉCMakeLists.txt‚ğì¬
CMakeLists.txt
```
cmake_minimum_required(VERSION 3.14)
project(imgui)

add_library(imgui
    imgui.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

```