#pragma once

#include "types.h"

extern Model   *model_create();
extern void     model_load(Model *model, char *path, char *model_name);
extern void     model_set_translation(Model *model, float x, float y, float z);
extern void     model_set_rotate(Model *model, float x, float y, float z);
extern void     model_set_scale(Model *model, float x, float y, float z);
extern void     model_destroy(Model *model);
