#ifndef UTILS_MATRIX_H
#define UTILS_MATRIX_H

#include "typedefs.h"

matrix mat_init(int rows, int cols);
matrix mat_id(int size);
void mat_id_no_alloc(matrix *mat, int size);

matrix mat_mul(matrix mat1, matrix mat2);
vec3 mat_apply(matrix mat, vec3 vertex);
triangle3 mat_apply_triangle3(matrix mat, triangle3 tri);

void mat_scale(matrix *target, vec3 vector);
void mat_translate(matrix *target, vec3 vector);
void mat_translate_invert(matrix *target, vec3 vector);

void mat_rotate_yaw(matrix *target, float angle);
void mat_rotate_pitch(matrix *target, float angle);
void mat_rotate_roll(matrix *target, float angle);

matrix projection_matrix(float fovy, float aspect, float near, float far);
void mat_project(matrix *target, float fovy, float aspect, float near, float far);

void mat_print(matrix mat);

#endif
