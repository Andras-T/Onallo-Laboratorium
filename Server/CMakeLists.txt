cmake_minimum_required(VERSION 3.21)
project(Server)

set(CMAKE_TOOLCHAIN_FILE C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE OFF)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(IMGUI_DIR ../lib/imgui)
set(STB_DIR ../lib/stb)
set(LOGGER_DIR ../lib/logger)
set(SOURCES
        src/main.cpp
        src/Vulkan/Core.cpp
        src/Networking/ServerNetworking.cpp

        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp 
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/imgui.cpp 
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp 
        ${IMGUI_DIR}/imgui_tables.cpp 
        ${IMGUI_DIR}/imgui_widgets.cpp

        ${STB_DIR}/stb_image.cpp
        ${STB_DIR}/stb_image.h
        ${LOGGER_DIR}/Logger.h
        ${LOGGER_DIR}/Logger.cpp)

add_executable(Server ${SOURCES} ../res/app.o)
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

find_package(GameNetworkingSockets CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ${STB_DIR} ${LOGGER_DIR} src)
target_link_libraries(Server glfw glm Vulkan::Vulkan 
GameNetworkingSockets::GameNetworkingSockets GameNetworkingSockets::static GameNetworkingSockets::shared GameNetworkingSockets::GameNetworkingSockets_s)

# Set a macro for Debug mode
set_property(
    TARGET Server 
    APPEND 
    PROPERTY COMPILE_DEFINITIONS 
    $<$<CONFIG:Debug>:DEBUG_MACRO>
)

# Set a macro for Release mode
set_property(
    TARGET Server
    APPEND 
    PROPERTY COMPILE_DEFINITIONS 
    $<$<CONFIG:Release>:RELEASE_MACRO>
)