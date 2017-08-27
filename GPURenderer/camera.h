#pragma once

#include "types.h"

extern Camera  *camera_create();
extern void     camera_set_eye(Camera *camera, float x, float y, float z);
extern void     camera_set_look_at(Camera *camera, float x, float y, float z);
extern void     camera_set_up(Camera *camera, float x, float y, float z);
extern void     camera_destroy(Camera *camera);
