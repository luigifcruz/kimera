#include "kimera/render/opengl/backend.hpp"

namespace Kimera {

bool OpenGL::ParsePlaneSizes(PixelFormat fmt, int width, int height, Format* size, unsigned int* planes) {
    *planes = av_pix_fmt_count_planes(fmt);

    for (unsigned int i = 0; i < *planes; i++) {
        float ratio = (float)width / (float)av_image_get_linesize(fmt, width, i);

        switch (fmt) {
            case AV_PIX_FMT_YUV420P:
                (size+i)->w = (int)((float)width / ratio);
                (size+i)->h = (int)((float)height / ratio);
                (size+i)->pix = GL_RED;
                break;
            case AV_PIX_FMT_BGRA:
                (size+i)->w = width;
                (size+i)->h = height;
                (size+i)->pix = GL_RGBA;
                break;
            default:
                return false;
        }
    }

    if ((*planes) > MAX_PLANES) {
        printf("[OPENGL] Too many planes (%d), format unsupported.\n", (*planes));
        return false;
    }

    return true;
}

bool OpenGL::GetError(std::string line) {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "[OPENGL] GL returned an error #" << error
                  << " at " << line << "." << std::endl;
        return true;
    }
    return false;
}

// name: GL_VENDOR, GL_RENDERER, GL_VERSION, GL_SHADING_LANGUAGE_VERSION, GL_EXTENSIONS
const char* OpenGL::Query(GLenum name) {
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

void OpenGL::FillTexture(Format size, uint8_t* data) {
    glTexImage2D(GL_TEXTURE_2D, 0, size.pix, size.w, size.h, 0, size.pix, GL_UNSIGNED_BYTE, data);
}

void OpenGL::ReadTexture(Format size, uint8_t* data) {
    glReadPixels(0, 0, size.w, size.h, size.pix, GL_UNSIGNED_BYTE, data);
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

unsigned int OpenGL::PunchFramebuffer() {
    unsigned int index = proc_tex[proc_index];
    proc_index = (proc_index + 1) % MAX_PROC;
    return index;
}

unsigned int OpenGL::GetFramebuffer() {
    return proc_tex[proc_index];
}

unsigned int OpenGL::GetLastFramebuffer() {
    return proc_tex[!proc_index];
}

void OpenGL::BindFramebufferTexture(unsigned int atch_id, unsigned int tex_id) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, atch_id, GL_TEXTURE_2D, tex_id, 0);
}

void OpenGL::SetUniform4f(int program, char* name, float v0, float v1, float v2, float v3) {
    int location = glGetUniformLocation(program, name);
    glUniform4f(location, v0, v1, v2, v3);
}

void OpenGL::SetUniform3f(int program, char* name, float v0, float v1, float v2) {
    int location = glGetUniformLocation(program, name);
    glUniform3f(location, v0, v1, v2);
}

void OpenGL::SetUniform2f(int program, char* name, float v0, float v1) {
    int location = glGetUniformLocation(program, name);
    glUniform2f(location, v0, v1);
}

void OpenGL::SetUniform1f(int program, char* name, float v0) {
    int location = glGetUniformLocation(program, name);
    glUniform1f(location, v0);
}

void OpenGL::SetUniform1i(int program, char* name, int v0) {
    int location = glGetUniformLocation(program, name);
    glUniform1i(location, v0);
}

} // namespace Kimera
