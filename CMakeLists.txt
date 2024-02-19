cmake_minimum_required(VERSION 3.21)
project(OnalloLaboratorium)

add_subdirectory(Client)
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_subdirectory(lib/GameNetworkingSockets)
add_subdirectory(Server)