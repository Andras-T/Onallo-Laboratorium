cmake_minimum_required(VERSION 3.21)
project(Client)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_VERBOSE_MAKEFILE OFF)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(IMGUI_DIR ../lib/imgui)
set(STB_DIR ../lib/stb)
set(LOGGER_DIR ../lib/logger)
set(SOURCES
        src/main.cpp
        src/Vulkan/Core.cpp
        src/Vulkan/Window/Window.cpp
        src/Vulkan/DeviceManager.cpp
        src/Vulkan/Swapchain/SwapChainManager.cpp
        src/Vulkan/Swapchain/Images.cpp
        src/Vulkan/Descriptor/DescriptorManager.cpp
        src/Vulkan/Descriptor/Descriptor.cpp
        src/Vulkan/Pipeline/PipelineManager.cpp
        src/Vulkan/Pipeline/Pipeline.cpp
        src/Vulkan/CommandPool/CommandPoolManager.cpp

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

add_executable(Client ${SOURCES} ../res/app.o)
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

find_package(Vulkan REQUIRED)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ${STB_DIR} ${LOGGER_DIR} src)
target_link_libraries(Client glfw glm GameNetworkingSockets Vulkan::Vulkan)