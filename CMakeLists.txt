cmake_minimum_required(VERSION 3.10)

project(fractals)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_subdirectory(utils)

add_subdirectory(websocket)

add_subdirectory(controller)

add_subdirectory(node)
