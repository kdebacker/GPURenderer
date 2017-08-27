#pragma once

#if !defined(uint)
typedef unsigned int uint;
#endif

#if !defined(NULL)
#define NULL ((void *)0);
#endif

typedef struct {
    uint v[3], n[3], t[3];
} Triangle;

typedef struct Group_t {
    char           *name;
    uint            material;
    uint            num_triangles;
    uint           *triangles;
    struct Group_t *next;
} Group;

typedef struct {
    char       *model_name;
    char       *materials_name;
    char       *path;

    uint        num_vertices;
    float      *vertices;

    uint        num_texcoords;
    float      *texcoords;

    uint        num_normals;
    float      *normals;

    uint        num_triangles;
    Triangle   *triangles;

    uint        num_groups;
    Group      *groups;

    float       translation[3], rotation[3], scale[3];
    float       model_to_world[16];
} Model;

typedef struct {
    float   focal_length;
    float   width, height;
    float   background_color[3];

    float   eye[3], look_at[3], up[3];
    float   world_to_view[16];
} Camera;
