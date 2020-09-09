#ifndef OPENGL_BACKEND_H
#define OPENGL_BACKEND_H

#define MAX_PROC 2
#define MAX_PLANES 3

extern "C" {
#include "glad/glad.h"
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}

#include "kimera/render/opengl/device.hpp"
#include "kimera/render/shaders.hpp"
#include "kimera/render/backend.hpp"
#include "kimera/state.hpp"

#include <iostream>
#include <vector>

namespace Kimera {

static const float vertices[] = {
     1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f
};

static const unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

typedef struct {
    int w;
    int h;
    GLenum pix;
} Format;

class OpenGL : public Backend {
public:
    OpenGL(State&);
    ~OpenGL();

    void PrintMeta();

    std::vector<PixelFormat> GetInputFormats();
    std::vector<PixelFormat> GetOutputFormats();

    bool LoadInput(PixelFormat);
    bool LoadDisplay();
    bool LoadFilter();
    bool LoadOutput(PixelFormat);
    bool CommitPipeline();

    bool Push(AVFrame*);
    bool Draw();
    bool Filter();
    bool Pull(AVFrame*);

private:
    std::vector<PixelFormat> InputFormats = {
        AV_PIX_FMT_YUV420P,
        AV_PIX_FMT_BGRA
    };

    std::vector<PixelFormat> OutputFormats = {
        AV_PIX_FMT_YUV420P,
    };

    bool use_opengles;
    bool use_display;

    double time;

    std::unique_ptr<Device> device;

    PixelFormat in_format;
    PixelFormat out_format;

    Format f_size;
    Format d_size;

    Format in_size[MAX_PLANES];
    Format out_size[MAX_PLANES];

    unsigned int vertex_buffer;
    unsigned int index_buffer;
    unsigned int frame_buffer;

    unsigned int in_shader;
    unsigned int out_shader;
    unsigned int proc_shader;
    unsigned int disp_shader;

    unsigned int in_planes;
    unsigned int out_planes;

    unsigned int proc_index;

    unsigned int in_tex[MAX_PLANES];
    unsigned int out_tex[MAX_PLANES];
    unsigned int proc_tex[MAX_PROC];

    const char* Query(GLenum);
    bool GetError(std::string);

    bool ParsePlaneSizes(PixelFormat, int, int, Format*, unsigned int*);

    char* OpenShader(char*);
    unsigned int CompileShader(unsigned int, char*);
    unsigned int LoadShader(int type, char*, char*);

    void CreateTexture(unsigned int, Format);
    void SetDrawBuffer(unsigned int);
    void FillTexture(Format, uint8_t*);
    void ReadTexture(Format, uint8_t*);

    unsigned int PunchFramebuffer();
    unsigned int GetFramebuffer();
    unsigned int GetLastFramebuffer();

    void BindFramebufferTexture(unsigned int, unsigned int);

    void SetUniform4f(int, char*, float, float, float, float);
    void SetUniform3f(int, char*, float, float, float);
    void SetUniform2f(int, char*, float, float);
    void SetUniform1f(int, char*, float);
    void SetUniform1i(int, char*, int);
};

} // namespace Kimera

#endif
