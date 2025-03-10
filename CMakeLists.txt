cmake_minimum_required(VERSION 3.10.0)
project(LaunchPadSnake VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(${PROJECT_NAME} "app.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE winmm)