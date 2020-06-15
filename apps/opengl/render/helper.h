#ifndef RENDER_HELPER_H
#define RENDER_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "glad/glad.h"
#include "glad/glad_egl.h"

size_t get_file_size(FILE* fp) {
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return size;
}

char* open_shader(char* filepath) {
    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("[RENDER] Can't open shader file: %s\n", filepath);
        return NULL;
    }

    size_t size = get_file_size(fp);
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

bool get_gl_error(int line) {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("[RENDER] GL returned an error #%d at line #%d.\n", error, line);
        return true;
    }
    return false;
}

bool get_egl_error(int line) {
    int error = eglGetError();
    if (error != EGL_SUCCESS) {
        printf("[RENDER] EGL returned an error #%d at line #%d.\n", error, line);
        return true;
    }
    return false;
}

unsigned int compile_shader(unsigned int type, char* path) {
    char* code_str = open_shader(path);
    if (!code_str) return 0;

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&code_str, NULL);
    glCompileShader(shader);
    free(code_str);
    
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("[RENDER] Shader compilation error:\n%s", info_log);
        return 0;
    }

    return shader;
}

unsigned int load_shader(char* vs_path, char* fs_path) {
    unsigned int program = glCreateProgram();
    unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vs_path);
    unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fs_path);

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

void set_uniform4f(int program, char* name, float v0, float v1, float v2, float v3) {
    int location = glGetUniformLocation(program, name);
    glUniform4f(location, v0, v1, v2, v3);
}

void set_uniform3f(int program, char* name, float v0, float v1, float v2) {
    int location = glGetUniformLocation(program, name);
    glUniform3f(location, v0, v1, v2);
}

void set_uniform2f(int program, char* name, float v0, float v1) {
    int location = glGetUniformLocation(program, name);
    glUniform2f(location, v0, v1);
}

void set_uniform1f(int program, char* name, float v0) {
    int location = glGetUniformLocation(program, name);
    glUniform1f(location, v0);
}

void set_uniform1i(int program, char* name, int v0) {
    int location = glGetUniformLocation(program, name);
    glUniform1i(location, v0);
}

void set_draw_buffer(GLenum attachment) {
    GLenum targets[1] = { GL_COLOR_ATTACHMENT0 };
    targets[0] = attachment;
    glDrawBuffers(1, targets);
}

double mticks() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double) tv.tv_usec / 1000 + tv.tv_sec * 1000;
}

#endif