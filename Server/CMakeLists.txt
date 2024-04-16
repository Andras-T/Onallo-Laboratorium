cmake_minimum_required(VERSION 3.21)
project(Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE OFF)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(IMGUI_DIR ../lib/imgui)
set(STB_DIR ../lib/stb)
set(LOGGER_DIR ../lib/logger)
set(SOURCES
        src/main.cpp
        src/Vulkan/Core.cpp
        src/Vulkan/HeadlessCore.cpp
        src/Vulkan/WindowedCore.cpp
        src/Vulkan/CommandPool/CommandPoolManager.cpp
        src/Vulkan/Descriptor/DescriptorManager.cpp
        src/Vulkan/Descriptor/Descriptor.cpp
        src/Vulkan/DeviceManager/DeviceManager.cpp
        src/Vulkan/Pipeline/Pipeline.cpp
        src/Vulkan/Pipeline/PipelineManager.cpp
        src/Vulkan/ResourceManager/Images.cpp
        src/Vulkan/ResourceManager/ResourceManager.cpp
        src/Vulkan/Renderer/Renderer.cpp
        src/Vulkan/Renderer/OffScreenRenderer.cpp
        src/Vulkan/Renderer/WindowedRenderer.cpp
        src/Vulkan/SwapChain/SwapChainManager.cpp
        src/Vulkan/Utils/Constants.h
        src/Vulkan/Window/Window.cpp
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

add_executable(Server ${SOURCES} ../res/app.o "$ENV{COMPRESSONATOR_ROOT}/lib/bin/x64/CMP_Core_MDd_DLL.dll" "$ENV{COMPRESSONATOR_ROOT}/lib/bin/x64/Compressonator_MDd_DLL.dll")
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Server/shaders")
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ${STB_DIR} ${LOGGER_DIR} src "$ENV{COMPRESSONATOR_ROOT}/include")
target_link_libraries(Server glfw glm Vulkan::Vulkan GameNetworkingSockets::GameNetworkingSockets "$ENV{COMPRESSONATOR_ROOT}/lib/bin/x64/CMP_Core_MDd.lib" "$ENV{COMPRESSONATOR_ROOT}/lib/bin/x64/Compressonator_MDd.lib")