#include "GPURenderer.h"

typedef struct {
    cl_uint width;
    cl_uint height;
    cl_uint num_triangles;
    cl_uint num_models;
    cl_float aspect_ratio;
    cl_float fov;
    cl_float scale;
    cl_float padding;
    cl_float4 background_color;
    cl_float camera_to_world[16];
} RT_Options;

typedef struct {
    cl_uint triangle_start, num_triangles;
} RT_Model;

typedef struct {
    cl_float4 v, n, t;
} RT_Triangle;

float deg2rad(const float deg)
{
    return deg * M_PI / 180.f;
}

void debug_check_error(cl_int err, char *name, char *file, long line);
char *get_error_string(cl_int err);

cl_device_id select_device();
cl_program build_program(cl_context context, cl_device_id device, char *filename);


#define check_error(err, name) debug_check_error(err, name, __FILE__, __LINE__ - 1)

void r_raytrace(Camera *camera, Model *models, uint num_models)
{
    cl_int err;

    cl_device_id device = select_device();

    char device_vendor[128], device_name[128];
    err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 128, device_vendor, NULL);
    check_error(err, "clGetDeviceInfo");
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, 128, device_name, NULL);
    check_error(err, "clGetDeviceInfo");

    fprintf(stdout, "Running raytracer on %s %s\n", device_vendor, device_name);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    check_error(err, "clCreateContext");

    cl_program program = build_program(context, device, "raytracer.cl");

    cl_kernel kernel = clCreateKernel(program, "render", &err);
    check_error(err, "clCreateKernel");

    cl_command_queue queue = clCreateCommandQueue(context, device, NULL, &err);
    check_error(err, "clCreateCommandQueue");

    uint total_triangles = 0, total_vertices = 0;
    for (int i = 0; i < num_models; i++) {
        total_triangles += models[i].num_triangles;
        total_vertices += models[i].num_vertices + 1;
    }

    RT_Options options = { 0 };
    options.width = 1280;
    options.height = 720;
    options.num_triangles = total_triangles;
    options.num_models = num_models;
    options.aspect_ratio = (float)options.width / (float)options.height;
    options.fov = 51.52f;
    options.scale = tan(deg2rad(options.fov * 0.5f));
    options.background_color.x = 0.18f;
    options.background_color.y = 0.18f;
    options.background_color.z = 0.18f;
    options.background_color.w = 1.f;
    am_matrix_inverse(options.camera_to_world, camera->world_to_view);

    RT_Model *internal_models;
    internal_models = malloc((sizeof *internal_models) * num_models);

    RT_Triangle *triangles;
    triangles = malloc((sizeof *triangles) * total_triangles);

    cl_float4 *vertices;
    vertices = malloc((sizeof *vertices) * total_vertices);

    uint triangle_index = 0, vertex_index = 1;
    for (int i = 0; i < num_models; i++) {
        internal_models[i].triangle_start = triangle_index;
        internal_models[i].num_triangles = models[i].num_triangles;

        for (int j = 0; j < models[i].num_triangles; j++) {
            triangles[triangle_index].v.x = models[i].triangles[j].v[0] + vertex_index - 1;
            triangles[triangle_index].v.y = models[i].triangles[j].v[1] + vertex_index - 1;
            triangles[triangle_index].v.z = models[i].triangles[j].v[2] + vertex_index - 1;

            triangles[triangle_index].n.x = models[i].triangles[j].n[0];
            triangles[triangle_index].n.y = models[i].triangles[j].n[1];
            triangles[triangle_index].n.z = models[i].triangles[j].n[2];

            triangles[triangle_index].t.x = models[i].triangles[j].t[0];
            triangles[triangle_index].t.y = models[i].triangles[j].t[1];
            triangles[triangle_index].t.z = models[i].triangles[j].t[2];

            triangle_index++;
        }

        for (int j = 1; j <= models[i].num_vertices; j++) {
            float vertex[3];

            am_matrix_transform3(vertex, models[i].model_to_world, models[i].vertices[(j * 3) + 0], models[i].vertices[(j * 3) + 1], models[i].vertices[(j * 3) + 2]);

            vertices[vertex_index].x = vertex[0];
            vertices[vertex_index].y = vertex[1];
            vertices[vertex_index].z = vertex[2];

            vertex_index++;
        }
    }

    cl_image_format image_format = { 0 };
    image_format.image_channel_order = CL_RGBA;
    image_format.image_channel_data_type = CL_UNORM_INT8;

    cl_image_desc image_desc = { 0 };
    image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
    image_desc.image_width = options.width;
    image_desc.image_height = options.height;
    image_desc.image_depth = 1;
    image_desc.image_array_size = 0;
    image_desc.image_row_pitch = image_desc.image_width * 4;
    image_desc.image_slice_pitch = 0;
    image_desc.num_mip_levels = 0;
    image_desc.num_samples = 0;
    image_desc.buffer = NULL;

    unsigned char *image = malloc(image_desc.image_row_pitch * image_desc.image_height);

    cl_mem *options_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof options, NULL, &err);
    check_error(err, "clCreateBuffer");

    err = clEnqueueWriteBuffer(queue, options_buffer, CL_TRUE, 0, sizeof options, &options, 0, NULL, NULL);
    check_error(err, "clEnqueueWriteBuffer");

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &options_buffer);
    check_error(err, "clSetKernelArg");

    cl_mem *models_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, (sizeof *internal_models) * num_models, NULL, &err);
    check_error(err, "clCreateBuffer");

    err = clEnqueueWriteBuffer(queue, models_buffer, CL_TRUE, 0, (sizeof *internal_models) * num_models, internal_models, 0, NULL, NULL);
    check_error(err, "clEnqueueWriteBuffer");

    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &models_buffer);
    check_error(err, "clSetKernelArg");

    cl_mem triangles_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, (sizeof *triangles) * total_triangles, NULL, &err);
    check_error(err, "clCreateBuffer");

    err = clEnqueueWriteBuffer(queue, triangles_buffer, CL_TRUE, 0, (sizeof *triangles) * total_triangles, triangles, 0, NULL, NULL);
    check_error(err, "clEnqueueWriteBuffer");

    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &triangles_buffer);
    check_error(err, "clSetKernelArg");

    cl_mem vertices_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, (sizeof *vertices) * total_vertices, NULL, &err);
    check_error(err, "clCreateBuffer");

    err = clEnqueueWriteBuffer(queue, vertices_buffer, CL_TRUE, 0, (sizeof *vertices) * total_vertices, vertices, 0, NULL, NULL);
    check_error(err, "clEnqueueWriteBuffer");

    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &vertices_buffer);
    check_error(err, "clSetKernelArg");

    cl_mem *image_buffer = clCreateImage(context, NULL, &image_format, &image_desc, image, &err);
    check_error(err, "clCreateImage");

    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &image_buffer);
    check_error(err, "clSetKernelArg");

    fprintf(stdout, "Running raytracer ...");

    const size_t gdim[] = { options.width, options.height };
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, gdim, NULL, 0, NULL, NULL);
    check_error(err, "clEnqueueNDRangeKernel");

    const size_t origin[] = { 0, 0, 0 };
    const size_t region[] = { image_desc.image_width, image_desc.image_height, image_desc.image_depth };

    err = clEnqueueReadImage(queue, image_buffer, CL_TRUE, origin, region, image_desc.image_row_pitch, image_desc.image_slice_pitch, image, 0, NULL, NULL);
    check_error(err, "clEnqueueReadImage");

    fprintf(stdout, "Done\n");

    FILE *fp;
    fopen_s(&fp, "out.ppm", "wb");
    fprintf_s(fp, "P6 %d %d 255 ", image_desc.image_width, image_desc.image_height);
    for (int i = 0; i < image_desc.image_width * image_desc.image_height; i++) {
        putc(image[i * 4 + 0], fp);
        putc(image[i * 4 + 1], fp);
        putc(image[i * 4 + 2], fp);
    }
    fclose(fp);

    clReleaseMemObject(vertices_buffer);
    clReleaseMemObject(triangles_buffer);
    clReleaseMemObject(models_buffer);
    clReleaseMemObject(options_buffer);

    free(image);
    free(vertices);
    free(triangles);
    free(internal_models);

    clFlush(queue);
    clFinish(queue);
    clReleaseCommandQueue(queue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device);
}

void debug_check_error(cl_int err, char *function, char *file, long line)
{
    if (err != CL_SUCCESS) {
        fprintf(stderr, "%s() failed in file %s at line #%i: %s\n", function, file, line, get_error_string(err));
        getchar();
        exit(EXIT_FAILURE);
    }
}

char *get_error_string(cl_int err)
{
    switch (err) {
        case   0: return "CL_SUCCESS";
        case  -1: return "CL_DEVICE_NOT_FOUND";
        case  -2: return "CL_DEVICE_NOT_AVAILABLE";
        case  -3: return "CL_COMPILER_NOT_AVAILABLE";
        case  -4: return "CL_MEM_OBJECT_ALLOCATION_FAILUR";
        case  -5: return "CL_OUT_OF_RESOURCES";
        case  -6: return "CL_OUT_OF_HOST_MEMORY";
        case  -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case  -8: return "CL_MEM_COPY_OVERLAP";
        case  -9: return "CL_IMAGE_FORMAT_MISMATCH";
        case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -11: return "CL_BUILD_PROGRAM_FAILURE";
        case -12: return "CL_MAP_FAILURE";
        case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case -15: return "CL_COMPILE_PROGRAM_FAILURE";
        case -16: return "CL_LINKER_NOT_AVAILABLE";
        case -17: return "CL_LINK_PROGRAM_FAILURE";
        case -18: return "CL_DEVICE_PARTITION_FAILED";
        case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
        case -30: return "CL_INVALID_VALUE";
        case -31: return "CL_INVALID_DEVICE_TYPE";
        case -32: return "CL_INVALID_PLATFORM";
        case -33: return "CL_INVALID_DEVICE";
        case -34: return "CL_INVALID_CONTEXT";
        case -35: return "CL_INVALID_QUEUE_PROPERTIES";
        case -36: return "CL_INVALID_COMMAND_QUEUE";
        case -37: return "CL_INVALID_HOST_PTR";
        case -38: return "CL_INVALID_MEM_OBJECT";
        case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40: return "CL_INVALID_IMAGE_SIZE";
        case -41: return "CL_INVALID_SAMPLER";
        case -42: return "CL_INVALID_BINARY";
        case -43: return "CL_INVALID_BUILD_OPTIONS";
        case -44: return "CL_INVALID_PROGRAM";
        case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46: return "CL_INVALID_KERNEL_NAME";
        case -47: return "CL_INVALID_KERNEL_DEFINITION";
        case -48: return "CL_INVALID_KERNEL";
        case -49: return "CL_INVALID_ARG_INDEX";
        case -50: return "CL_INVALID_ARG_VALUE";
        case -51: return "CL_INVALID_ARG_SIZE";
        case -52: return "CL_INVALID_KERNEL_ARGS";
        case -53: return "CL_INVALID_WORK_DIMENSION";
        case -54: return "CL_INVALID_WORK_GROUP_SIZE";
        case -55: return "CL_INVALID_WORK_ITEM_SIZE";
        case -56: return "CL_INVALID_GLOBAL_OFFSET";
        case -57: return "CL_INVALID_EVENT_WAIT_LIST";
        case -58: return "CL_INVALID_EVENT";
        case -59: return "CL_INVALID_OPERATION";
        case -60: return "CL_INVALID_GL_OBJECT";
        case -61: return "CL_INVALID_BUFFER_SIZE";
        case -62: return "CL_INVALID_MIP_LEVEL";
        case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64: return "CL_INVALID_PROPERTY";
        case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
        case -66: return "CL_INVALID_COMPILER_OPTIONS";
        case -67: return "CL_INVALID_LINKER_OPTIONS";
        case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";
        default:  return "Unknown OpenCL error";
    }
}

cl_device_id select_device()
{
    cl_int err;
    cl_uint num_platforms, num_devices;

    cl_platform_id *platforms;
    cl_device_id *devices = NULL, device;

    err = clGetPlatformIDs(0, NULL, &num_platforms);
    check_error(err, "clGetPlatformIDs");

    platforms = malloc((sizeof *platforms) * num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    check_error(err, "clGetPlatformIDs");

    for (uint i = 0; i < num_platforms; i++) {
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
        check_error(err, "clGetDeviceIDs");

        devices = malloc((sizeof *devices) * num_devices);
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
        check_error(err, "clGetDeviceIDs");

        if (num_platforms > 1 || num_devices > 1) {
            for (uint j = 0; j < num_devices; j++) {
                char device_vendor[128], device_name[128];
                err = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, 128, device_vendor, NULL);
                check_error(err, "clGetDeviceInfo");
                err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 128, device_name, NULL);
                check_error(err, "clGetDeviceInfo");

                fprintf(stdout, "%d: %s %s\n", j, device_vendor, device_name);
            }
        }
    }

    free(platforms);

    if (num_platforms > 1 || num_devices > 1) {
        int index;
        scanf("%i", &index);
        device = (devices[index]);
    } else {
        device = (devices[0]);
    }

    free(devices);

    return device;
}

cl_program build_program(cl_context context, cl_device_id device, char *filename)
{
    cl_int err;
    FILE *fp = NULL;
    fp = fopen(filename, "r");

    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    rewind(fp);

    char *source;
    source = calloc((sizeof *source), length);

    fread(source, sizeof *source, length, fp);
    fclose(fp);

    cl_program program = clCreateProgramWithSource(context, 1, &source, &length, &err);
    check_error(err, "clCreateProgramWithSource");

    free(source);

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    if (err != CL_SUCCESS) {
        char log[8192];
        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 8192, log, NULL);
        fprintf(stderr, "Build error in file \"%s\":\n%s", filename, log);
        getchar();
        exit(EXIT_FAILURE);
    }

    return program;
}
