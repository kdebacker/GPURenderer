/**
* Implementation of John Carmack's fast inverse square root approximation.
* Source: https://github.com/id-Software/Quake-III-Arena/blob/master/code/game/q_math.c#L552
*/
float am_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;                            // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1);                  // what the fuck? 
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y));        // 1st iteration
                                                //  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    return y;
}

float am_sqrt(float num)
{
    return num * am_rsqrt(num);
}

float am_length2(float *vec) {
    return am_sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
}

float am_distance2(float *a, float *b) {
    float tmp[2];
    tmp[0] = a[0] - b[0];
    tmp[1] = a[1] - b[1];
    return am_sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1]);
}

float am_dot2(float *a, float *b) {
    return a[0] * b[0] + a[1] * b[1];
}

void am_cross2(float *output, float *a) {
    output[0] = a[1];
    output[1] = -a[0];
}

void am_normalize2(float *vec) {
    float f = am_sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
    vec[0] /= f;
    vec[1] /= f;
}

float am_length3(float *vec)
{
    return am_sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

float am_distance3(float *a, float *b)
{
    float tmp[3];
    tmp[0] = a[0] - b[0];
    tmp[1] = a[1] - b[1];
    tmp[2] = a[2] - b[2];
    return am_sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
}

float am_dot3(float *a, float *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void am_cross3(float *output, float *a, float *b)
{
    output[0] = a[1] * b[2] - a[2] * b[1];
    output[1] = a[2] * b[0] - a[0] * b[2];
    output[2] = a[0] * b[1] - a[1] * b[0];
}

void am_normalize3(float *vec)
{
    float f = am_sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    vec[0] /= f;
    vec[1] /= f;
    vec[2] /= f;
}