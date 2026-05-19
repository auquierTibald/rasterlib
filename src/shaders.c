#ifndef RASTERLIB_SHADERS_H
#define RASTERLIB_SHADERS_H

#include "rasterlib.h"
#include "../include/typedefs.h"
#include "../include/utils-matrix.h"


void default_vs(struct RL_Context_t *context, RL_Triangle *triangle) {
    triangle3 mv = {
        .a = mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.a)),
        .b = mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.b)),
        .c = mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.c))
    };
    triangle->pos = mv;
}

void default_fs(struct RL_Context_t *context, RL_Triangle *triangle) {
    
}

#endif //RASTERLIB_SHADERS_H