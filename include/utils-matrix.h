#ifndef UTILS_MATRIX_H
#define UTILS_MATRIX_H

#include "typedefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

matrix mat_init(int rows, int cols);
matrix mat_id(int size);

matrix mat_mul(matrix mat1, matrix mat2);
vec3 mat_apply(matrix mat, vec3 vertex);

matrix mat_scale(matrix target, vec3 vector);
matrix mat_translate(matrix target, vec3 vector);
matrix mat_translate_invert(matrix target, vec3 vector);

matrix mat_rotate_yaw(matrix target, float angle);
matrix mat_rotate_pitch(matrix target, float angle);
matrix mat_rotate_roll(matrix target, float angle);

void mat_print(matrix mat);

#endif
