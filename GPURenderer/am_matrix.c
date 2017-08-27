void am_matrix_clear(float *matrix)
{
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 1;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

void am_matrix_multiply(float *output, float *a, float *b)
{
    output[ 0] = (a[ 0] * b[ 0]) + (a[ 1] * b[ 4]) + (a[ 2] * b[ 8]) + (a[ 3] * b[12]);
    output[ 1] = (a[ 0] * b[ 1]) + (a[ 1] * b[ 5]) + (a[ 2] * b[ 9]) + (a[ 3] * b[13]);
    output[ 2] = (a[ 0] * b[ 2]) + (a[ 1] * b[ 6]) + (a[ 2] * b[10]) + (a[ 3] * b[14]);
    output[ 3] = (a[ 0] * b[ 3]) + (a[ 1] * b[ 7]) + (a[ 2] * b[11]) + (a[ 3] * b[15]);
    output[ 4] = (a[ 4] * b[ 0]) + (a[ 5] * b[ 4]) + (a[ 6] * b[ 8]) + (a[ 7] * b[12]);
    output[ 5] = (a[ 4] * b[ 1]) + (a[ 5] * b[ 5]) + (a[ 6] * b[ 9]) + (a[ 7] * b[13]);
    output[ 6] = (a[ 4] * b[ 2]) + (a[ 5] * b[ 6]) + (a[ 6] * b[10]) + (a[ 7] * b[14]);
    output[ 7] = (a[ 4] * b[ 3]) + (a[ 5] * b[ 7]) + (a[ 6] * b[11]) + (a[ 7] * b[15]);
    output[ 8] = (a[ 8] * b[ 0]) + (a[ 9] * b[ 4]) + (a[10] * b[ 8]) + (a[11] * b[12]);
    output[ 9] = (a[ 8] * b[ 1]) + (a[ 9] * b[ 5]) + (a[10] * b[ 9]) + (a[11] * b[13]);
    output[10] = (a[ 8] * b[ 2]) + (a[ 9] * b[ 6]) + (a[10] * b[10]) + (a[11] * b[14]);
    output[11] = (a[ 8] * b[ 3]) + (a[ 9] * b[ 7]) + (a[10] * b[11]) + (a[11] * b[15]);
    output[12] = (a[12] * b[ 0]) + (a[13] * b[ 4]) + (a[14] * b[ 8]) + (a[15] * b[12]);
    output[13] = (a[12] * b[ 1]) + (a[13] * b[ 5]) + (a[14] * b[ 9]) + (a[15] * b[13]);
    output[14] = (a[12] * b[ 2]) + (a[13] * b[ 6]) + (a[14] * b[10]) + (a[15] * b[14]);
    output[15] = (a[12] * b[ 3]) + (a[13] * b[ 7]) + (a[14] * b[11]) + (a[15] * b[15]);
}

void am_matrix_inverse(float *output, float *matrix)
{
    float det = matrix[0] * matrix[5] * matrix[10] * matrix[15]
        + matrix[0] * matrix[6] * matrix[11] * matrix[13]
        + matrix[0] * matrix[7] * matrix[9] * matrix[14]
        + matrix[1] * matrix[4] * matrix[11] * matrix[14]
        + matrix[1] * matrix[6] * matrix[8] * matrix[15]
        + matrix[1] * matrix[7] * matrix[10] * matrix[12]
        + matrix[2] * matrix[4] * matrix[9] * matrix[15]
        + matrix[2] * matrix[5] * matrix[11] * matrix[12]
        + matrix[2] * matrix[7] * matrix[8] * matrix[13]
        + matrix[3] * matrix[4] * matrix[10] * matrix[13]
        + matrix[3] * matrix[5] * matrix[8] * matrix[14]
        + matrix[3] * matrix[6] * matrix[9] * matrix[12]
        - matrix[0] * matrix[5] * matrix[11] * matrix[14]
        - matrix[0] * matrix[6] * matrix[9] * matrix[15]
        - matrix[0] * matrix[7] * matrix[10] * matrix[13]
        - matrix[1] * matrix[4] * matrix[10] * matrix[15]
        - matrix[1] * matrix[6] * matrix[11] * matrix[12]
        - matrix[1] * matrix[7] * matrix[8] * matrix[14]
        - matrix[2] * matrix[4] * matrix[11] * matrix[13]
        - matrix[2] * matrix[5] * matrix[8] * matrix[15]
        - matrix[2] * matrix[7] * matrix[9] * matrix[12]
        - matrix[3] * matrix[4] * matrix[9] * matrix[14]
        - matrix[3] * matrix[5] * matrix[10] * matrix[12]
        - matrix[3] * matrix[6] * matrix[8] * matrix[13];

    if (det) {
        output[0] = (1.f / det) * (matrix[5] * matrix[10] * matrix[15] + matrix[6] * matrix[11] * matrix[13] + matrix[7] * matrix[9] * matrix[14] - matrix[5] * matrix[11] * matrix[14] - matrix[6] * matrix[9] * matrix[15] - matrix[7] * matrix[10] * matrix[13]);
        output[1] = (1.f / det) * (matrix[1] * matrix[11] * matrix[14] + matrix[2] * matrix[9] * matrix[15] + matrix[3] * matrix[10] * matrix[13] - matrix[1] * matrix[10] * matrix[15] - matrix[2] * matrix[11] * matrix[13] - matrix[3] * matrix[9] * matrix[14]);
        output[2] = (1.f / det) * (matrix[1] * matrix[6] * matrix[15] + matrix[2] * matrix[7] * matrix[13] + matrix[3] * matrix[5] * matrix[14] - matrix[1] * matrix[7] * matrix[14] - matrix[2] * matrix[5] * matrix[15] - matrix[3] * matrix[6] * matrix[13]);
        output[3] = (1.f / det) * (matrix[1] * matrix[7] * matrix[10] + matrix[2] * matrix[5] * matrix[11] + matrix[3] * matrix[6] * matrix[9] - matrix[1] * matrix[6] * matrix[11] - matrix[2] * matrix[7] * matrix[9] - matrix[3] * matrix[5] * matrix[10]);
        output[4] = (1.f / det) * (matrix[4] * matrix[11] * matrix[14] + matrix[6] * matrix[8] * matrix[15] + matrix[7] * matrix[10] * matrix[12] - matrix[4] * matrix[10] * matrix[15] - matrix[6] * matrix[11] * matrix[12] - matrix[7] * matrix[8] * matrix[14]);
        output[5] = (1.f / det) * (matrix[0] * matrix[10] * matrix[15] + matrix[2] * matrix[11] * matrix[12] + matrix[3] * matrix[8] * matrix[14] - matrix[0] * matrix[11] * matrix[14] - matrix[2] * matrix[8] * matrix[15] - matrix[3] * matrix[10] * matrix[12]);
        output[6] = (1.f / det) * (matrix[0] * matrix[7] * matrix[14] + matrix[2] * matrix[4] * matrix[15] + matrix[3] * matrix[6] * matrix[12] - matrix[0] * matrix[6] * matrix[15] - matrix[2] * matrix[7] * matrix[12] - matrix[3] * matrix[4] * matrix[14]);
        output[7] = (1.f / det) * (matrix[0] * matrix[6] * matrix[11] + matrix[2] * matrix[7] * matrix[8] + matrix[3] * matrix[4] * matrix[10] - matrix[0] * matrix[7] * matrix[10] - matrix[2] * matrix[4] * matrix[11] - matrix[3] * matrix[6] * matrix[8]);
        output[8] = (1.f / det) * (matrix[4] * matrix[9] * matrix[15] + matrix[5] * matrix[11] * matrix[12] + matrix[7] * matrix[8] * matrix[13] - matrix[4] * matrix[11] * matrix[13] - matrix[5] * matrix[8] * matrix[15] - matrix[7] * matrix[9] * matrix[12]);
        output[9] = (1.f / det) * (matrix[0] * matrix[11] * matrix[13] + matrix[1] * matrix[8] * matrix[15] + matrix[3] * matrix[9] * matrix[12] - matrix[0] * matrix[9] * matrix[15] - matrix[1] * matrix[11] * matrix[12] - matrix[3] * matrix[8] * matrix[13]);
        output[10] = (1.f / det) * (matrix[0] * matrix[5] * matrix[15] + matrix[1] * matrix[7] * matrix[12] + matrix[3] * matrix[4] * matrix[13] - matrix[0] * matrix[7] * matrix[13] - matrix[1] * matrix[4] * matrix[15] - matrix[3] * matrix[5] * matrix[12]);
        output[11] = (1.f / det) * (matrix[0] * matrix[7] * matrix[9] + matrix[1] * matrix[4] * matrix[11] + matrix[3] * matrix[5] * matrix[8] - matrix[0] * matrix[5] * matrix[11] - matrix[1] * matrix[7] * matrix[8] - matrix[3] * matrix[4] * matrix[9]);
        output[12] = (1.f / det) * (matrix[4] * matrix[10] * matrix[13] + matrix[5] * matrix[8] * matrix[14] + matrix[6] * matrix[9] * matrix[12] - matrix[4] * matrix[9] * matrix[14] - matrix[5] * matrix[10] * matrix[12] - matrix[6] * matrix[8] * matrix[13]);
        output[13] = (1.f / det) * (matrix[0] * matrix[9] * matrix[14] + matrix[1] * matrix[10] * matrix[12] + matrix[2] * matrix[8] * matrix[13] - matrix[0] * matrix[10] * matrix[13] - matrix[1] * matrix[8] * matrix[14] - matrix[2] * matrix[9] * matrix[12]);
        output[14] = (1.f / det) * (matrix[0] * matrix[6] * matrix[13] + matrix[1] * matrix[4] * matrix[14] + matrix[2] * matrix[5] * matrix[12] - matrix[0] * matrix[5] * matrix[14] - matrix[1] * matrix[6] * matrix[12] - matrix[2] * matrix[4] * matrix[13]);
        output[15] = (1.f / det) * (matrix[0] * matrix[5] * matrix[10] + matrix[1] * matrix[6] * matrix[8] + matrix[2] * matrix[4] * matrix[9] - matrix[0] * matrix[6] * matrix[9] - matrix[1] * matrix[4] * matrix[10] - matrix[2] * matrix[5] * matrix[8]);
    }
}

void am_matrix_transform3(float *output, float *matrix, float x, float y, float z)
{
    output[0] = (matrix[0] * x) + (matrix[1] * y) + (matrix[2] * z) + matrix[3];
    output[1] = (matrix[4] * x) + (matrix[5] * y) + (matrix[6] * z) + matrix[7];
    output[2] = (matrix[8] * x) + (matrix[9] * y) + (matrix[10] * z) + matrix[11];
}

void am_matrix_transform4(float *output, float *matrix, float x, float y, float z, float w)
{
    output[0] = (matrix[ 0] * x) + (matrix[ 1] * y) + (matrix[ 2] * z) + (matrix[ 3] * w);
    output[1] = (matrix[ 4] * x) + (matrix[ 5] * y) + (matrix[ 6] * z) + (matrix[ 7] * w);
    output[2] = (matrix[ 8] * x) + (matrix[ 9] * y) + (matrix[10] * z) + (matrix[11] * w);
    output[3] = (matrix[12] * x) + (matrix[13] * y) + (matrix[14] * z) + (matrix[15] * w);
}
