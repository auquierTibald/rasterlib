#include "utils-matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

matrix mat_init(int rows, int cols) 
{
	matrix res = {.data = NULL, .rows = rows, .cols = cols};
	res.data = (float*)malloc(rows * cols * sizeof(float));
	for(int i = 0; i < rows; i++)
	{
		for(int j = 0; j < cols; j++)
		{
			res.data[i * cols + j] = 0;
		}
	}
	return res;
}

matrix mat_id(int size) 
{
	matrix res = {.rows = size, .cols = size};
	res.data = (float*)malloc(size * size * sizeof(float));
	for(int i = 0; i < size; i++)
	{
		for(int j = 0; j < size; j++)
		{
			if(i == j) res.data[i * size + j] = 1; 
			else	   res.data[i * size + j] = 0;
		}
	}
	return res;
}


void mat_id_no_alloc(matrix *mat, int size)
{
	mat->rows = size; mat->cols = size;
	for(int i = 0; i < size; i++)
	{
		for(int j = 0; j < size; j++)
		{
			if(i == j) mat->data[i * size + j] = 1;
			else	   mat->data[i * size + j] = 0;
		}
	}
}

matrix mat_mul(matrix mat1, matrix mat2)
{
	if(mat1.cols != mat2.rows) return (matrix){.data = NULL, .rows = 0, .cols = 0};
	matrix res = mat_init(mat1.rows, mat2.cols);
	for(int i = 0; i < res.rows; i++)
	{
		for(int j = 0; j < res.cols; j++)
		{
			float s = 0;
			for(int k = 0; k < mat1.cols; k++) { s += mat1.data[i * mat1.cols + k] * mat2.data[k * mat2.cols + j];}
			res.data[i * res.cols + j] = s;
		}
	}
	return res;
	
}

vec3 mat_apply(matrix mat, vec3 vertex)
{
	vec3 res;
	float data[4] = {vertex.x, vertex.y, vertex.z, 1};
	matrix vec = { .data = data, .rows = 4, .cols = 1 };

	vec = mat_mul(mat, vec);
	if (!vec.data) return (vec3){0, 0, 0};
	res = (vec3){vec.data[0], vec.data[1], vec.data[2]};
	free(vec.data);
	return res;
}

triangle3 mat_apply_triangle3(matrix mat, triangle3 tri) {
	return (triangle3){mat_apply(mat, tri.a), mat_apply(mat, tri.b), mat_apply(mat, tri.c)};
}

void mat_scale(matrix *target, vec3 vector)
{
	matrix mat = mat_id(4);

	mat.data[0] = vector.x;
	mat.data[5] = vector.y;
	mat.data[10] = vector.z;

	matrix res = mat_mul(mat, *target);
	if (!res.data) return;
	free(mat.data);
	free(target->data);
	target->data = res.data;
}

void mat_translate(matrix *target, vec3 vector)
{
	matrix mat = mat_id(4);

	mat.data[3] = vector.x;
	mat.data[7] = vector.y;
	mat.data[11] = vector.z;

	matrix res = mat_mul(mat, *target);
	if (!res.data) return;
	free(mat.data);
	free(target->data);
	target->data = res.data;
}

void mat_translate_invert(matrix *target, vec3 vector)
{
	matrix mat = mat_id(4);

	mat.data[3] -= vector.x;
	mat.data[7] -= vector.y;
	mat.data[11] -= vector.z;

	matrix res = mat_mul(mat, *target);
	if (!res.data) return;
	free(mat.data);
	free(target->data);
	target->data = res.data;
}

void mat_rotate_yaw(matrix *target, float angle)
{
	float c = cos(angle), s = sin(angle);
	float data[4*4] = { c, -s, 0, 0,
			    s, c, 0, 0,
			    0, 0, 1, 0,
			    0, 0, 0, 1 };

	matrix mat = { .data = data, .rows = 4, .cols = 4};

	matrix res = mat_mul(mat, *target);
	if (!res.data) return;
	free(target->data);
	target->data = res.data;
}

void mat_rotate_pitch(matrix *target, float angle)
{
	float c = cos(angle), s = sin(angle);
	float data[4*4] = { c, 0, s, 0,
			    0, 1, 0, 0,
			    -s, 0, c, 0,
			    0, 0, 0, 1 };

	matrix mat = { .data = data, .rows = 4, .cols = 4};

	matrix res = mat_mul(mat, *target);
	if (!res.data) return;
	free(target->data);
	target->data = res.data;
}


void mat_rotate_roll(matrix *target, float angle)
{
	float c = cos(angle), s = sin(angle);
	float data[4*4] = { 1, 0, 0, 0,
			    0, c, -s, 0,
			    0, s, c, 0,
			    0, 0, 0, 1 };

	matrix mat = { .data = data, .rows = 4, .cols = 4};

	matrix res = mat_mul(mat, *target);
	if (!res.data) return;
	free(target->data);
	target->data = res.data;
}

matrix projection_matrix(float fovy, float aspect, float near, float far) {
	const float f = 1.0f / tanf(fovy/2);

	const matrix projection_matrix = mat_init(4, 4);
	projection_matrix.data[0] = f / aspect;
	projection_matrix.data[5] = f;
	projection_matrix.data[10] = - near - far / (near - far);
	projection_matrix.data[11] = 2 * far * near  / (near - far);
	projection_matrix.data[14] = -1;

	return projection_matrix;
}

void mat_project(matrix *target, float fovy, float aspect, float near, float far) {
	matrix proj = projection_matrix(fovy, aspect, near, far);

	matrix res = mat_mul(proj, *target);
	free(proj.data);
	free(target->data);
	target->data = res.data;
}


void mat_print(matrix mat)
{
	for(int i = 0; i < mat.rows; i++)
	{
		for(int j = 0; j < mat.cols; j++)
		{
			printf("%3.2f, ", mat.data[i * mat.cols + j]);
		}
		printf("\n");
	}
} 
