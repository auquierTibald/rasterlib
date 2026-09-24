cmake_minimum_required(VERSION 4.1)
project(rasterlib)

add_compile_options(-Wall -Wextra)

set(SRC main.c include/rasterlib.h
        include/typedefs.h
        include/assets_loaders.h
        include/stb_image.h
        include/threads.h
        include/thread_pools.h
        include/utils-matrix.h
        include/utils-da.h
        include/utils-triangles.h
        include/utils.h
        src/assets_loaders.c
        src/utils-matrix.c
        src/rasterlib.c
        src/utils.c
        src/utils-triangles.c
        src/stb_image_impl.c
        src/threads.c
        src/thread_pools.c
)

add_executable(rasterlib ${SRC})
target_link_directories(rasterlib PRIVATE ./lib/)
target_include_directories(rasterlib PRIVATE ./include/ )
target_link_libraries(rasterlib SDL2 m)

