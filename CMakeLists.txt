cmake_minimum_required(VERSION 4.1)
project(rasterlib)

add_compile_options(-Wall -Wextra)

set(SRC main.c include/rasterlib.h
        include/typedefs.h
        include/assets_loaders.h
        include/stb_image.h
        include/utils-matrix.h
        src/assets_loaders.c
        src/utils-matrix.c
        src/rasterlib.c
        include/utils.h
        src/utils.c
        include/utils-da.h
        include/threads.h
)

add_executable(rasterlib ${SRC})
target_link_directories(rasterlib PRIVATE ./lib/)
target_include_directories(rasterlib PRIVATE ./include/ )
target_link_libraries(rasterlib SDL2 m)

