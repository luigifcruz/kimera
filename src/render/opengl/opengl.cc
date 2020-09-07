#include "kimera/render/opengl/backend.hpp"

namespace Kimera {

bool OpenGL::GetErrorGL(std::string line) {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "[RENDER] GL returned an error #" << error
                  << " at " << line << "." << std::endl;
        return true;
    }
    return false;
}

bool OpenGL::GetErrorEGL(std::string line) {
    int error = eglGetError();
    if (error != EGL_SUCCESS) {
        std::cout << "[RENDER] EGL returned an error #" << error
                  << " at " << line << "." << std::endl;
        return true;
    }
    return false;
}

// name: EGL_CLIENT_APIS, EGL_VENDOR, EGL_VERSION, EGL_EXTENSIONS
const char* OpenGL::EGLQuery(int name) {
    return eglQueryString(device.display, name);
}

// name: GL_VENDOR, GL_RENDERER, GL_VERSION, GL_SHADING_LANGUAGE_VERSION, GL_EXTENSIONS
const char* OpenGL::GLQuery(GLenum name) {
    return (const char*)glGetString(name);
}

void OpenGL::CreateTexture(unsigned int id, Format fmt) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt.pix, fmt.w, fmt.h, 0, fmt.pix, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGL::SetDrawBuffer(unsigned int attachment) {
    unsigned int targets[1] = { GL_COLOR_ATTACHMENT0 };
    targets[0] = attachment;
    glDrawBuffers(1, targets);
}

char* OpenGL::OpenShader(char* filepath) {
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("[RENDER] Can't open shader file: %s\n", filepath);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char* buffer = (char*)malloc((size + 1) * sizeof(char));
    if (buffer == NULL) {
        printf("[RENDER] Can't allocate file buffer.\n");
        fclose(fp);
        return NULL;
    }

    size_t r_size = fread(buffer, sizeof(char), size, fp);
    if (r_size != size) {
        printf("[RENDER] Failed to read file properly (S: %ld, R: %ld)", size, r_size);
        fclose(fp);
        free(buffer);
        return NULL;
    }
    buffer[size] = '\0';

    fclose(fp);
    return buffer;
}

unsigned int OpenGL::CompileShader(unsigned int type, char* code_str) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&code_str, NULL);
    glCompileShader(shader);

    int success;
    char info_log[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("[RENDER] Shader compilation error:\n%s", info_log);
        return 0;
    }

    return shader;
}

unsigned int OpenGL::LoadShader(int type, char* vs_str, char* fs_str) {
    unsigned int program = glCreateProgram();

    if (type == 0) {
        vs_str = OpenShader(vs_str);
        fs_str = OpenShader(fs_str);
        if (!vs_str || !fs_str) return 0;
    }

    unsigned int vertex_shader = CompileShader(GL_VERTEX_SHADER, vs_str);
    unsigned int fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fs_str);

    if (type == 0) {
        free(vs_str);
        free(fs_str);
    }

    if (vertex_shader == 0 || fragment_shader == 0)
        return 0;

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("[RENDER] Shader compilation error:\n%s", info_log);
        return 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}


OpenGL::OpenGL() {
    std::cout << "[BACKEND] Initializing OpenGL backend." << std::endl;

    // Generating Buffers
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &index_buffer);
    glGenFramebuffers(1, &frame_buffer);

    // Loading Vertex Buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Loading Indices Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Loading framebuffer for internal textures
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glGenTextures(MAX_PROC, &proc_tex[0]);
    for (unsigned int i = 0; i < MAX_PROC; i++)
        CreateTexture(proc_tex[i], f_size);
    SetDrawBuffer(GL_COLOR_ATTACHMENT0);
    proc_index = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!GetErrorGL(__LINE__)) throw "error";
}

OpenGL::~OpenGL() {
    std::cout << "[BACKEND] Exiting OpenGL backend." << std::endl;
}

void OpenGL::PrintMeta() {
    const char* render_mode = (use_display) ? "WINDOWED" : "HEADLESS";

    printf(".   [Render Meta]\n");
    printf("├── Window Mode:   %s\n", render_mode);
    printf("├── Frame Size:    %dx%d\n", f_size.w, f_size.h);
    printf("├── Device Size:   %dx%d\n", d_size.w, d_size.h);
    printf("└──.   [EGL Meta]\n");
    printf("│  ├── APIs:       %s\n", EGLQuery(EGL_CLIENT_APIS));
    printf("│  ├── Version:    %s\n", EGLQuery(EGL_VERSION));
    printf("│  └── Vendor:     %s\n", EGLQuery(EGL_VENDOR));
    printf("└──.   [GL Meta]\n");
    printf("   ├── Renderer:   %s\n", GLQuery(GL_RENDERER));
    printf("   ├── Version:    %s\n", GLQuery(GL_VERSION));
    printf("   ├── Vendor:     %s\n", GLQuery(GL_VENDOR));
    printf("   └── GLSL Ver.:  %s\n", GLQuery(GL_SHADING_LANGUAGE_VERSION));
}

std::vector<PixelFormat> OpenGL::GetInputFormats() {
    return InputFormats;
}

std::vector<PixelFormat> OpenGL::GetOutputFormats() {
    return OutputFormats;
}

bool OpenGL::LoadInput(PixelFormat) {
    if (!get_planes_size(frame, &in_size[0], &in_planes))
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glGenTextures(in_planes, &in_tex[0]);
    for (unsigned int i = 0; i < in_planes; i++)
        CreateTexture(in_tex[i], in_size[i]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    switch (in_format) {
        case AV_PIX_FMT_YUV420P:
            in_shader = LoadShader(1, (char*)in_yuv420_vs, (char*)in_yuv420_fs);
            break;
        case AV_PIX_FMT_BGRA:
            in_shader = LoadShader(1, (char*)in_bgra_vs, (char*)in_bgra_fs);
            break;
        default:
            printf("[RENDER] Unsupported GPU based color convertion.\n");
            break;
    }

    if (!in_shader) return false;

    input_ready = true;

    return !GetErrorGL("Push");
}

bool OpenGL::LoadDisplay() {
    disp_shader = LoadShader(1, (char*)display_vs, (char*)display_fs);
    if (!disp_shader) return false;

    display_ready = true;
    return true;
}

bool OpenGL::LoadFilter() {
    proc_shader = LoadShader(1, (char*)filter_vs, (char*)filter_fs);
    if (!proc_shader) return false;

    process_ready = true;
    return true;
}

bool OpenGL::LoadOutput(PixelFormat) {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glGenTextures(out_planes, &out_tex[0]);
    for (unsigned int i = 0; i < out_planes; i++)
        CreateTexture(out_tex[i], out_size[i]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    out_shader = LoadShader(1, (char*)out_yuv420_vs,  (char*)out_yuv420_fs);
    if (!out_shader) return false;

    output_ready = true;

    return !GetErrorGL("LoadOutput");
}

bool OpenGL::CommitPipeline() {}

bool OpenGL::Push(AVFrame*) {
    if (!resampler_push_frame(render->resampler, render->state, frame))
        return false;

    if (!render->input_ready)
        if (!load_input(render, render->resampler->frame)) return false;

    glBindFramebuffer(GL_FRAMEBUFFER, render->frame_buffer);
    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, get_framebuffer(render));
    set_draw_buffer(GL_COLOR_ATTACHMENT0);

    {
        glViewport(0, 0, render->f_size.w, render->f_size.h);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render->in_shader);

        for (unsigned int i = 0; i < render->in_planes; i++) {
            char plane_name[16];
            sprintf(plane_name, "PlaneTex%d", i);
            set_uniform1i(render->in_shader, plane_name, i);

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, render->in_tex[i]);
            fill_texture(render->in_size[i], render->resampler->frame->data[i]);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    bind_framebuffer_tex(GL_COLOR_ATTACHMENT0, 0);
    punch_framebuffer(render);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    render->pts = render->resampler->frame->pts;

    return !get_gl_error(__LINE__);
}

bool OpenGL::Draw() {}
bool OpenGL::Filter() {}
AVFrame* OpenGL::Pull(AVFrame* frame) {}

} // namespace Kimera
