#include "GPURenderer.h"

#define T(x) (model->triangles[x])

Group *find_group(Model *model, char *name);
Group *add_group(Model *model, char *name);
void first_pass(Model *model, FILE *file);
void second_pass(Model *model, FILE *file);

void update_model_to_world_matrix(Model *model);

Model *model_create()
{
    Model *model;
    model = malloc(sizeof *model);

    model->path = NULL;
    model->model_name = NULL;
    model->num_vertices = 0;
    model->vertices = NULL;
    model->num_texcoords = 0;
    model->texcoords = NULL;
    model->num_normals = 0;
    model->normals = NULL;
    model->num_triangles = 0;
    model->triangles = NULL;
    model->num_groups = 0;
    model->groups = NULL;

    model->translation[0] = 0.f;
    model->translation[1] = 0.f;
    model->translation[2] = 0.f;

    model->rotation[0] = 0.f;
    model->rotation[1] = 0.f;
    model->rotation[2] = 0.f;

    model->scale[0] = 1.f;
    model->scale[1] = 1.f;
    model->scale[2] = 1.f;

    update_model_to_world_matrix(model);

    return model;
}

void model_load(Model *model, char *path, char *model_name)
{
    FILE *file;

    char filename[1024];
    strncpy_s(filename, 1024, path, 768);
    strncat_s(filename, 1024, model_name, 256);

    fopen_s(&file, filename, "r");

    size_t path_length = strlen(path);
    model->path = malloc(path_length + 1);
    strcpy_s(model->path, path_length + 1, path);

    size_t name_length = strlen(model_name);
    model->model_name = malloc(name_length + 1);
    strcpy_s(model->model_name, name_length + 1, model_name);

    first_pass(model, file);
    rewind(file);

    model->vertices = (float *)malloc((sizeof *model->vertices) * 3 * (model->num_vertices + 1));

    if (model->num_normals)
        model->normals = (float *)malloc((sizeof *model->normals) * 3 * (model->num_normals + 1));

    if (model->num_texcoords)
        model->texcoords = (float *)malloc((sizeof *model->texcoords) * 2 * (model->num_texcoords + 1));

    model->triangles = (Triangle *)malloc((sizeof *model->triangles) * model->num_triangles);

    second_pass(model, file);
    fclose(file);
}

void model_set_translation(Model *model, float x, float y, float z)
{
    model->translation[0] = x;
    model->translation[1] = y;
    model->translation[2] = z;

    update_model_to_world_matrix(model);
}

void model_set_rotate(Model *model, float x, float y, float z)
{
    model->rotation[0] = x * M_PI / 180.f;
    model->rotation[1] = y * M_PI / 180.f;
    model->rotation[2] = z * M_PI / 180.f;

    update_model_to_world_matrix(model);
}

void model_set_scale(Model *model, float x, float y, float z)
{
    model->scale[0] = x;
    model->scale[1] = y;
    model->scale[2] = z;

    update_model_to_world_matrix(model);
}

void model_destroy(Model *model)
{
    if (model != NULL) {
        Group *group = model->groups;
        while (group) {
            Group *prev_group = group;
            group = group->next;

            free(prev_group->triangles);
            free(prev_group);
        }
        free(model);
    }
}

void update_model_to_world_matrix(Model *model)
{
    float temp_matrix[16], translation_matrix[16], scale_matrix[16], rotation_matrix[16];

    am_matrix_clear(translation_matrix);
    am_matrix_clear(scale_matrix);
    am_matrix_clear(rotation_matrix);

    translation_matrix[3] = model->translation[0];
    translation_matrix[7] = model->translation[1];
    translation_matrix[11] = model->translation[2];

    scale_matrix[0] = model->scale[0];
    scale_matrix[5] = model->scale[1];
    scale_matrix[10] = model->scale[2];

    am_matrix_multiply(temp_matrix, translation_matrix, scale_matrix);

    rotation_matrix[5] = (float)cos((double)model->rotation[0]);
    rotation_matrix[6] = -(float)sin((double)model->rotation[0]);
    rotation_matrix[9] = (float)sin((double)model->rotation[0]);
    rotation_matrix[10] = (float)cos((double)model->rotation[0]);

    am_matrix_multiply(model->model_to_world, temp_matrix, rotation_matrix);
    am_matrix_clear(rotation_matrix);

    rotation_matrix[0] = (float)cos((double)model->rotation[1]);
    rotation_matrix[2] = -(float)sin((double)model->rotation[1]);
    rotation_matrix[8] = (float)sin((double)model->rotation[1]);
    rotation_matrix[10] = (float)cos((double)model->rotation[1]);

    am_matrix_multiply(temp_matrix, model->model_to_world, rotation_matrix);
    am_matrix_clear(rotation_matrix);

    rotation_matrix[0] = (float)cos((double)model->rotation[2]);
    rotation_matrix[1] = -(float)sin((double)model->rotation[2]);
    rotation_matrix[4] = (float)sin((double)model->rotation[2]);
    rotation_matrix[5] = (float)cos((double)model->rotation[2]);

    am_matrix_multiply(model->model_to_world, temp_matrix, rotation_matrix);
}

Group *find_group(Model *model, char *name)
{
    Group *group = model->groups;

    while (group) {
        if (!strcmp(name, group->name))
            break;

        group = group->next;
    }

    return group;
}

Group *add_group(Model *model, char *name)
{
    Group *group = find_group(model, name);

    if (group == NULL) {
        group = malloc(sizeof *group);

        size_t name_length = strlen(name);
        group->name = malloc(name_length + 1);
        strcpy_s(group->name, name_length + 1, name);

        group->material = 0;
        group->num_triangles = 0;
        group->triangles = NULL;
        group->next = model->groups;

        model->groups = group;
        model->num_groups++;
    }

    return group;
}

void first_pass(Model *model, FILE *file)
{
    Group *group = NULL;
    uint num_vertices = 0;
    uint num_normals = 0;
    uint num_texcoords = 0;
    uint num_triangles = 0;
    uint v, n, t;
    char buf[256];

    while (fscanf(file, "%s", buf) != EOF) {
        switch (buf[0]) {
        case '#': {											// Comment
            fgets(buf, sizeof(buf), file);
        } break;

        case 'v': {
            switch (buf[1]) {
            case '\0': {									// Vertex
                fgets(buf, sizeof(buf), file);
                num_vertices++;
            } break;

            case 'n': {										// Normal
                fgets(buf, sizeof(buf), file);
                num_normals++;
            } break;

            case 't': {										// Texcoord
                fgets(buf, sizeof(buf), file);
                num_texcoords++;
            } break;

            default: {
                printf("loadObj(): Unknown token \"%s\".\n", buf);
            }
            }
        } break;

        case 'm': {
            fgets(buf, sizeof buf, file);
            sscanf(buf, "%s %s", buf, buf);
            //TODO: Read Material file and add it to a material list
        } break;

        case 'u': {
            fgets(buf, sizeof buf, file);
        } break;

        case 'g': {
            fgets(buf, sizeof buf, file);
            sscanf(buf, "%s", buf);
            group = add_group(model, buf);
        } break;

        case 'f': {												// Face
            if (group == NULL)
                group = add_group(model, "default");

            v = n = t = 0;
            fscanf(file, "%s", buf);
            if (strstr(buf, "//")) {								// "v//n"
                sscanf(buf, "%d//%d", &v, &n);
                fscanf(file, "%d//%d", &v, &n);
                fscanf(file, "%d//%d", &v, &n);
                num_triangles++;
                group->num_triangles++;
                while (fscanf(file, "%d//%d", &v, &n) > 0) {
                    num_triangles++;
                    group->num_triangles++;
                }
            } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n)) {	// "v/t/n"
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                num_triangles++;
                group->num_triangles++;
                while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                    num_triangles++;
                    group->num_triangles++;
                }
            } else if (sscanf(buf, "%d/%d", &v, &n)) {			// "v/n"
                fscanf(file, "%d/%d", &v, &n);
                fscanf(file, "%d/%d", &v, &n);
                num_triangles++;
                group->num_triangles++;
                while (fscanf(file, "%d/%d", &v, &n) > 0) {
                    num_triangles++;
                    group->num_triangles++;
                }
            } else {											// "v"
                fscanf(file, "%d", &v);
                fscanf(file, "%d", &v);
                num_triangles++;
                group->num_triangles++;
                while (fscanf(file, "%d", &v) > 0) {
                    num_triangles++;
                    group->num_triangles++;
                }
            }
        } break;

        default: {
            fgets(buf, sizeof(buf), file);
        } break;
        }
    }

    model->num_vertices = num_vertices;
    model->num_normals = num_normals;
    model->num_texcoords = num_texcoords;
    model->num_triangles = num_triangles;

    group = model->groups;
    while (group) {
        group->triangles = malloc((sizeof *group->triangles) * group->num_triangles);
        group->num_triangles = 0;
        group = group->next;
    }
}

void second_pass(Model *model, FILE *file)
{
    Group *group = find_group(model, "default");

    float *vertices = model->vertices;
    float *normals = model->normals;
    float *texcoords = model->texcoords;

    uint num_vertices = 1;
    uint num_normals = 1;
    uint num_texcoords = 1;
    uint num_triangles = 0;
    uint material = 0;
    uint v, n, t;

    char buf[256];
    while (fscanf(file, "%s", buf) != EOF) {
        switch (buf[0]) {
        case '#': {												// Comment
            fgets(buf, sizeof(buf), file);
        } break;

        case 'v': {
            switch (buf[1]) {
            case '\0': {									// Vertex
                fscanf(file, "%f %f %f", &vertices[3 * num_vertices + 0],
                    &vertices[3 * num_vertices + 1],
                    &vertices[3 * num_vertices + 2]);
                num_vertices++;
            } break;

            case 'n': {										// Normal
                fscanf(file, "%f %f %f", &normals[3 * num_normals + 0],
                    &normals[3 * num_normals + 1],
                    &normals[3 * num_normals + 2]);
                num_normals++;
            } break;

            case 't': {										// Texcoord
                fscanf(file, "%f %f", &texcoords[2 * num_texcoords + 0],
                    &texcoords[2 * num_texcoords + 1]);
                num_texcoords++;
            } break;

            default: {
                printf("loadObj(): Unknown token \"%s\".\n", buf);
            }
            }
        } break;

        case 'm': {
            fgets(buf, sizeof buf, file);
            sscanf(buf, "%s %s", buf, buf);
            //TODO: Read Material file and add it to a material list
        } break;

        case 'u': {
            fgets(buf, sizeof buf, file);
        } break;

        case 'g': {
            fgets(buf, sizeof buf, file);
            sscanf(buf, "%s", buf);
            group = find_group(model, buf);
        } break;

        case 'f': {												// Face
            v = n = t = 0;
            fscanf(file, "%s", buf);
            if (strstr(buf, "//")) {							// "v//n"
                sscanf(buf, "%d//%d", &v, &n);
                T(num_triangles).v[0] = v;
                T(num_triangles).n[0] = n;
                fscanf(file, "%d//%d", &v, &n);
                T(num_triangles).v[1] = v;
                T(num_triangles).n[1] = n;
                fscanf(file, "%d//%d", &v, &n);
                T(num_triangles).v[2] = v;
                T(num_triangles).n[2] = n;
                group->triangles[group->num_triangles++] = num_triangles;
                num_triangles++;
                while (fscanf(file, "%d//%d", &v, &n) > 0) {
                    T(num_triangles).v[0] = T(num_triangles - 1).v[0];
                    T(num_triangles).n[0] = T(num_triangles - 1).n[0];
                    T(num_triangles).v[1] = T(num_triangles - 1).v[2];
                    T(num_triangles).n[1] = T(num_triangles - 1).n[2];
                    T(num_triangles).v[2] = v;
                    T(num_triangles).n[2] = n;
                    group->triangles[group->num_triangles++] = num_triangles;
                    num_triangles++;
                }
            }
            else if (sscanf(buf, "%d/%d/%d", &v, &t, &n)) {	// "v/t/n"
                T(num_triangles).v[0] = v;
                T(num_triangles).t[0] = t;
                T(num_triangles).n[0] = n;
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                T(num_triangles).v[1] = v;
                T(num_triangles).t[1] = t;
                T(num_triangles).n[1] = n;
                fscanf(file, "%d/%d/%d", &v, &t, &n);
                T(num_triangles).v[2] = v;
                T(num_triangles).t[2] = t;
                T(num_triangles).n[2] = n;
                group->triangles[group->num_triangles++] = num_triangles;
                num_triangles++;
                while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                    T(num_triangles).v[0] = T(num_triangles - 1).v[0];
                    T(num_triangles).t[0] = T(num_triangles - 1).t[0];
                    T(num_triangles).n[0] = T(num_triangles - 1).n[0];
                    T(num_triangles).v[1] = T(num_triangles - 1).v[2];
                    T(num_triangles).t[1] = T(num_triangles - 1).t[2];
                    T(num_triangles).n[1] = T(num_triangles - 1).n[2];
                    T(num_triangles).v[2] = v;
                    T(num_triangles).t[2] = t;
                    T(num_triangles).n[2] = n;
                    group->triangles[group->num_triangles++] = num_triangles;
                    num_triangles++;
                }
            }
            else if (sscanf(buf, "%d/%d", &v, &n)) {			// "v/n"
                T(num_triangles).v[0] = v;
                T(num_triangles).n[0] = n;
                fscanf(file, "%d/%d", &v, &n);
                T(num_triangles).v[1] = v;
                T(num_triangles).n[1] = n;
                fscanf(file, "%d/%d", &v, &n);
                T(num_triangles).v[2] = v;
                T(num_triangles).n[2] = n;
                group->triangles[group->num_triangles++] = num_triangles;
                num_triangles++;
                while (fscanf(file, "%d/%d", &v, &n) > 0) {
                    T(num_triangles).v[0] = T(num_triangles - 1).v[0];
                    T(num_triangles).n[0] = T(num_triangles - 1).n[0];
                    T(num_triangles).v[1] = T(num_triangles - 1).v[2];
                    T(num_triangles).n[1] = T(num_triangles - 1).n[2];
                    T(num_triangles).v[2] = v;
                    T(num_triangles).n[2] = n;
                    group->triangles[group->num_triangles++] = num_triangles;
                    num_triangles++;
                }
            }
            else {											// "v"
                fscanf(file, "%d", &v);
                T(num_triangles).v[0] = v;
                fscanf(file, "%d", &v);
                T(num_triangles).v[1] = v;
                fscanf(file, "%d", &v);
                T(num_triangles).v[2] = v;
                group->triangles[group->num_triangles++] = num_triangles;
                num_triangles++;
                while (fscanf(file, "%d", &v) > 0) {
                    T(num_triangles).v[0] = T(num_triangles - 1).v[0];
                    T(num_triangles).v[1] = T(num_triangles - 1).v[2];
                    T(num_triangles).v[2] = v;
                    group->triangles[group->num_triangles++] = num_triangles;
                    num_triangles++;
                }
            }
        } break;

        default: {
            fgets(buf, sizeof(buf), file);
        } break;
        }
    }
}

