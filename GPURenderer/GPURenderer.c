#include "GPURenderer.h"

int main(int argc, char *argv)
{
    // Create model and camera
    Model  *model;
    Camera *camera;

    // Initialize model and camera
    model = model_create();
    camera = camera_create();

    //// Set camera settings
    camera_set_look_at(camera, 0, 0, -1.5);

    // Load model
    //model_load(model, "models/", "cow.obj");
    //model_set_translation(model, 0, -5, -30);
    //model_set_rotate(model, 0, 30, 0);
    model_load(model, "models/teapot/", "teapot.obj");
    model_set_translation(model, 0, -40, -140);

    // Render image
    r_raytrace(camera, model, 1);
    getchar();

    // Destroy model and camera objects
    model_destroy(model);
    camera_destroy(camera);

    return 0;
}