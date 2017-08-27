#include "GPURenderer.h"

void update_world_to_view(Camera *camera);

Camera *camera_create()
{
    Camera *camera;
    camera = malloc(sizeof *camera);

    camera->focal_length = 0.f;
    camera->width = 0.f;
    camera->height = 0.f;

    camera->background_color[0] = 0.f;
    camera->background_color[1] = 0.f;
    camera->background_color[2] = 0.f;

    camera->eye[0] = 0.f;
    camera->eye[1] = 0.f;
    camera->eye[2] = 0.f;

    camera->look_at[0] = 0.f;
    camera->look_at[1] = 0.f;
    camera->look_at[2] = 0.f;

    camera->up[0] = 0.f;
    camera->up[1] = 1.f;
    camera->up[2] = 0.f;

    update_world_to_view(camera);

    return camera;
}

void camera_set_eye(Camera *camera, float x, float y, float z)
{
    camera->eye[0] = x;
    camera->eye[1] = y;
    camera->eye[2] = z;

    update_world_to_view(camera);
}

void camera_set_look_at(Camera *camera, float x, float y, float z)
{
    camera->look_at[0] = x;
    camera->look_at[1] = y;
    camera->look_at[2] = z;

    update_world_to_view(camera);
}

void camera_set_up(Camera *camera, float x, float y, float z)
{
    camera->up[0] = x;
    camera->up[1] = y;
    camera->up[2] = z;

    update_world_to_view(camera);
}

void camera_destroy(Camera *camera)
{
    if (camera)
        free(camera);
}

void update_world_to_view(Camera *camera)
{
    float x_axis[3], y_axis[3], z_axis[3];

    am_matrix_clear(camera->world_to_view);

    z_axis[0] = camera->eye[0] - camera->look_at[0];
    z_axis[1] = camera->eye[1] - camera->look_at[1];
    z_axis[2] = camera->eye[2] - camera->look_at[2];
    am_normalize3(z_axis);

    am_cross3(x_axis, camera->up, z_axis);
    am_normalize3(x_axis);

    am_cross3(y_axis, z_axis, x_axis);

    camera->world_to_view[0] = x_axis[0];
    camera->world_to_view[1] = x_axis[1];
    camera->world_to_view[2] = x_axis[2];

    camera->world_to_view[4] = y_axis[0];
    camera->world_to_view[5] = y_axis[1];
    camera->world_to_view[6] = y_axis[2];

    camera->world_to_view[8] = z_axis[0];
    camera->world_to_view[9] = z_axis[1];
    camera->world_to_view[10] = z_axis[2];

    camera->world_to_view[3] = -am_dot3(x_axis, camera->eye);
    camera->world_to_view[7] = -am_dot3(y_axis, camera->eye);
    camera->world_to_view[11] = -am_dot3(z_axis, camera->eye);
}
