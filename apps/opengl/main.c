
#define KIMERA_UNIX
#define KIMERA_WINDOW_GLFW

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "render/render.h"
#include "render/meta.h"

#include "kimera/socket.h"
#include "kimera/encoder.h"
#include "kimera/decoder.h"
#include "kimera/resampler.h"
#include "kimera/loopback.h"
#include "kimera/display.h"
#include "kimera/client.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_writer.h"

int main(int argc, char* argv[]) {
    State state;
    state.width = 640;
    state.height = 480;

    RenderState* render = alloc_render();

    render->mode = WINDOWED;
    render->api  = EGL_OPENGL_ES_API;

    if (!start_render(render, &state)) goto cleanup;
    render_print_meta(render);

    float vertices[] = {
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f 
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glEnable(GL_BLEND); 
    glEnable(GL_MULTISAMPLE);

    unsigned int VBO, EBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    unsigned int frameShader = load_shader("./shaders/triangle.vs", "./shaders/triangle.fs");
    if (!frameShader) goto cleanup;

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load("img.png", &width, &height, &nrChannels, 0);
    
    while (1) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(frameShader);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);

        set_uniform1f(frameShader, "time", render_get_time(render));
        set_uniform2f(frameShader, "resolution", render->width, render->height);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        if (!render_commit_frame(render)) break;
    }

    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(frameShader);

cleanup:
    free_render(render);
    return 0;
}
