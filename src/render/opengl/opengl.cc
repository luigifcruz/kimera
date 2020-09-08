#include "kimera/render/opengl/backend.hpp"

namespace Kimera {

OpenGL::OpenGL(State& state) {
    std::cout << "[OPENGL] Initializing OpenGL backend." << std::endl;

    // Set initial frame size.
    f_size.w = state.width;
    f_size.h = state.height;
    f_size.pix = GL_RGBA;

    d_size.w = f_size.w / 2;
    d_size.h = f_size.h / 2;

    // Load EGL device
    device = std::make_unique<Device>(state, d_size.w, d_size.h, false);

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

    if (GetError("OpenGL::OpenGL()")) throw "error";
}

OpenGL::~OpenGL() {
    std::cout << "[OPENGL] Exiting OpenGL backend." << std::endl;
    device.reset();
}

void OpenGL::PrintMeta() {
    const char* render_mode = (use_display) ? "WINDOWED" : "HEADLESS";

    printf(".   [Render Meta]\n");
    printf("├── Window Mode:   %s\n", render_mode);
    printf("├── Frame Size:    %dx%d\n", f_size.w, f_size.h);
    printf("├── Device Size:   %dx%d\n", d_size.w, d_size.h);
    printf("└──.   [EGL Meta]\n");
    printf("│  ├── APIs:       %s\n", device->Query(EGL_CLIENT_APIS));
    printf("│  ├── Version:    %s\n", device->Query(EGL_VERSION));
    printf("│  └── Vendor:     %s\n", device->Query(EGL_VENDOR));
    printf("└──.   [GL Meta]\n");
    printf("   ├── Renderer:   %s\n", Query(GL_RENDERER));
    printf("   ├── Version:    %s\n", Query(GL_VERSION));
    printf("   ├── Vendor:     %s\n", Query(GL_VENDOR));
    printf("   └── GLSL Ver.:  %s\n", Query(GL_SHADING_LANGUAGE_VERSION));
}

std::vector<PixelFormat> OpenGL::GetInputFormats() {
    return InputFormats;
}

std::vector<PixelFormat> OpenGL::GetOutputFormats() {
    return OutputFormats;
}

bool OpenGL::LoadInput(AVFrame* frame) {
    this->in_format = (PixelFormat)frame->format;

    if (!ParsePlaneSizes(frame, &in_size[0], &in_planes))
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

    return !GetError("OpenGL::Push()");
}

bool OpenGL::LoadOutput(AVFrame* frame) {
    this->out_format = (PixelFormat)frame->format;

    if (!ParsePlaneSizes(frame, &out_size[0], &out_planes))
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glGenTextures(out_planes, &out_tex[0]);
    for (unsigned int i = 0; i < out_planes; i++)
        CreateTexture(out_tex[i], out_size[i]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    out_shader = LoadShader(1, (char*)out_yuv420_vs,  (char*)out_yuv420_fs);
    if (!out_shader) return false;

    return !GetError("OpenGL::LoadOutput()");
}

bool OpenGL::LoadDisplay() {
    disp_shader = LoadShader(1, (char*)display_vs, (char*)display_fs);
    if (!disp_shader) return false;
    return true;
}

bool OpenGL::LoadFilter() {
    proc_shader = LoadShader(1, (char*)filter_vs, (char*)filter_fs);
    if (!proc_shader) return false;
    return true;
}

bool OpenGL::CommitPipeline() {
    return true;
}

bool OpenGL::Push(AVFrame* frame) {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    BindFramebufferTexture(GL_COLOR_ATTACHMENT0, GetFramebuffer());
    SetDrawBuffer(GL_COLOR_ATTACHMENT0);

    {
        glViewport(0, 0, f_size.w, f_size.h);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(in_shader);

        for (unsigned int i = 0; i < in_planes; i++) {
            char plane_name[16];
            sprintf(plane_name, "PlaneTex%d", i);
            SetUniform1i(in_shader, plane_name, i);

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, in_tex[i]);
            FillTexture(in_size[i], frame->data[i]);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    BindFramebufferTexture(GL_COLOR_ATTACHMENT0, 0);
    PunchFramebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    frame->pts += 1;

    return !GetError("OpenGL::Push()");
}

bool OpenGL::Pull(AVFrame* frame) {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    SetDrawBuffer(GL_COLOR_ATTACHMENT0);
    glUseProgram(out_shader);

    SetUniform1i(out_shader, (char*)"renderedTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetLastFramebuffer());

    for (unsigned int i = 0; i < out_planes; i++) {
        SetUniform1i(out_shader, (char*)"PlaneId", i);
        BindFramebufferTexture(GL_COLOR_ATTACHMENT0, out_tex[i]);

        glViewport(0, 0, out_size[i].w, out_size[i].h);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        ReadTexture(out_size[i], frame->data[i]);
        BindFramebufferTexture(GL_COLOR_ATTACHMENT0, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    frame->pts += 1;

    return !GetError("OpenGL::Pull()");
}

bool OpenGL::Draw() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    {
        glViewport(0, 0, d_size.w, d_size.h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(disp_shader);

        float imgAspectRatio = (float)f_size.w / (float)f_size.h;
        float viewAspectRatio = (float)d_size.w / (float)d_size.h;

        float xScale = 1.0f, yScale = 1.0f;
        if (imgAspectRatio > viewAspectRatio) {
            yScale = viewAspectRatio / imgAspectRatio;
        } else {
            xScale = imgAspectRatio / viewAspectRatio;
        }
        SetUniform2f(disp_shader, (char*)"ScaleFact", xScale, yScale);

        SetUniform1f(disp_shader, (char*)"renderedTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GetLastFramebuffer());

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (GetError("OpenGL::Draw()")) return false;

    return device->Step(&d_size.w, &d_size.h);
}

bool OpenGL::Filter() {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    BindFramebufferTexture(GL_COLOR_ATTACHMENT0, GetFramebuffer());
    SetDrawBuffer(GL_COLOR_ATTACHMENT0);

    {
        glUseProgram(proc_shader);

        SetUniform1i(proc_shader, (char*)"pts", pts);
        //SetUniform1f(proc_shader, "time", );
        SetUniform2f(proc_shader, (char*)"resolution", f_size.w, f_size.h);
        SetUniform2f(proc_shader, (char*)"display", d_size.w, d_size.h);

        //if (cb != NULL) (void)(*cb)(render, obj);

        SetUniform1i(proc_shader, (char*)"renderedTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GetLastFramebuffer());

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    BindFramebufferTexture(GL_COLOR_ATTACHMENT0, 0);
    PunchFramebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return !GetError("OpenGL::Filter()");
}

} // namespace Kimera
