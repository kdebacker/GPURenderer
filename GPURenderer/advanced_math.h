#pragma once

extern float am_sqrt(float num);
extern float am_rsqrt(float number);

extern float am_length2(float *vec);
extern float am_distance2(float *a, float *b);
extern float am_dot2(float *a, float *b);
extern void am_cross2(float *output, float *a);
extern void am_normalize2(float *vec);

extern float am_length3(float *vec);
extern float am_distance3(float *a, float *b);
extern float am_dot3(float *a, float *b);
extern void am_cross3(float *output, float *a, float *b);
extern void am_normalize3(float *vec);

extern void am_matrix_clear(float *matrix);
extern void am_matrix_inverse(float *output, float *matrix);
extern void am_matrix_multiply(float *output, float *a, float *b);
extern void am_matrix_transform3(float *output, float *matrix, float x, float y, float z);
extern void am_matrix_transform4(float *output, float *matrix, float x, float y, float z, float w);
