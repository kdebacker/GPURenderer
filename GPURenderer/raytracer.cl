typedef struct {
    uint width;
    uint height;
    uint num_triangles;
    uint num_models;
    float aspect_ratio;
    float fov;
    float scale;
    float padding;
    float4 background_color;
    float camera_to_world[16];
} Options;

typedef struct {
    uint triangle_start, num_triangles;
} Model;

typedef struct {
    float4 v, n, t;
} Triangle;

void matrix_trans3(float4 *output, __global float *mat, float4 vec);

float4 cast_ray(__global Options *options, __global Model *models, __global Triangle *triangles, __global float4 *vertices, float4 origin, float4 direction);
__global Model *trace(__global Options *options, __global Model *models, __global Triangle *triangles, __global float4 *vertices, float4 origin, float4 direction, float *t_near, float2 *uv, uint *index);
bool intersect(Model model, __global Triangle *triangles, __global float4 *vertices, float4 origin, float4 direction, float *t_near, float2 *uv, uint *index);
bool ray_triangle_intersect(float4 origin, float4 direction, float4 v0, float4 v1, float4 v2, float *t, float *u, float *v);

__kernel void render(__global Options *options, __global Model *models, __global Triangle *triangles, __global float4 *vertices, __write_only image2d_t output)
{
    const int2 pos = (int2)(get_global_id(0), get_global_id(1));

    float x = (2.f * (pos.x + 0.5f) / (float)options->width - 1) * options->aspect_ratio * options->scale;
    float y = (1.f - 2.f * (pos.y + 0.5f) / (float)options->height) * options->scale;

    float4 origin, direction;
    matrix_trans3(&origin, options->camera_to_world, (float4)(0));
    matrix_trans3(&direction, options->camera_to_world, (float4)(x, y, -1.f, 0.f));
    direction = normalize(direction);

    float4 color = cast_ray(options, models, triangles, vertices, origin, direction);
    write_imagef(output, pos, (float4)(color.x, color.y, color.z, 1.f));
}

float4 cast_ray(__global Options *options, __global Model *models, __global Triangle *triangles, __global float4 *vertices, float4 origin, float4 direction)
{
    float4 color = options->background_color;

    uint index;
    float t_near = 1e-8;
    float2 uv;

    __global Model *hit_model = trace(options, models, triangles, vertices, origin, direction, &t_near, &uv, &index);

    if (hit_model != NULL) {
        color = (float4)(1, 1, 1, 1);
    }

    return color;
}

__global Model *trace(__global Options *options, __global Model *models, __global Triangle *triangles, __global float4 *vertices, float4 origin, float4 direction, float *t_near, float2 *uv, uint *index)
{
    __global Model *model = NULL;

    for (uint i = 0; i < options->num_models; i++) {
        uint index_triangle;
        float t_near_triangle = 1e-8;
        float2 uv_triangle;
        if (intersect(models[i], triangles, vertices, origin, direction, &t_near_triangle, &uv_triangle, &index_triangle) && t_near_triangle < *t_near) {
            model = &models[i];
            *t_near = t_near_triangle;
            *index = index_triangle;
            *uv = uv_triangle;
        }
    }

    return model;
}

bool intersect(Model model, __global Triangle *triangles, __global float4 *vertices, float4 origin, float4 direction, float *t_near, float2 *uv, uint *index)
{
    bool isect = false;
    
    for (uint i = model.triangle_start; i < model.triangle_start + model.num_triangles; i++) {
        float4 v0 = vertices[(int)triangles[i].v.x];
        float4 v1 = vertices[(int)triangles[i].v.y];
        float4 v2 = vertices[(int)triangles[i].v.z];
        float t = 1e-8, u, v;
        if (ray_triangle_intersect(origin, direction, v0, v1, v2, &t, &u, &v) && t < *t_near) {
            *t_near = t;
            uv->x = u;
            uv->y = v;
            *index = i;
            isect = true;
        }
    }

    return isect;
}

// implementation of the Moller Trumbore triangle intersection algorithm
bool ray_triangle_intersect(float4 origin, float4 direction, float4 v0, float4 v1, float4 v2, float *t, float *u, float *v)
{
    float4 v0v1 = v1 - v0;
    float4 v0v2 = v0 - v2;
    float4 pvec = cross(v0v2, direction);
    float det = dot(v0v1, pvec);

    if (fabs(det) < 1e-8f) {
        return false;
    }

    float inv_det = 1.f / det;

    float4 tvec = origin - v0;
    *u = dot(tvec, pvec) * inv_det;
    if (*u < 0.f || *u > 1.f) {
        return false;
    }

    float4 qvec = cross(tvec, v0v1);
    *v = dot(direction, qvec) * inv_det;
    if (*v < 0.f || (*u + *v) > 1.f) {
        return false;
    }

    *t = dot(v0v2, qvec) * inv_det;

    return true;
}

void matrix_trans3(float4 *output, __global float *mat, float4 vec)
{
    output->x = (mat[0] * vec.x) + (mat[1] * vec.y) + (mat[2] * vec.z) + mat[3];
    output->y = (mat[4] * vec.x) + (mat[5] * vec.y) + (mat[6] * vec.z) + mat[7];
    output->z = (mat[8] * vec.x) + (mat[9] * vec.y) + (mat[10] * vec.z) + mat[11];
}