cmake_minimum_required(VERSION 4.1)
project(rasterlib)


set(SRC main.c include/rasterlib.h
        include/typedefs.h
        include/assets_loaders.h
        include/stb_image.h
        include/utils-matrix.h
        src/assets_loaders.c
        src/utils-matrix.c
        src/rasterlib.c
        src/shaders.c
        include/utils.h
)

add_executable(rasterlib ${SRC})

target_link_libraries(rasterlib SDL2 m)
target_include_directories(rasterlib PRIVATE ./include/ )

