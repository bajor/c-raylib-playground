cmake_minimum_required(VERSION 3.10)
project(raylib_example C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add your source file(s) here
add_executable(main main.c)

# Find raylib
find_package(raylib REQUIRED)

# Link raylib to your app
target_link_libraries(main raylib)
