
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
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VBO, EBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    unsigned int program = load_shader("./shaders/triangle.vs", "./shaders/triangle.fs");
    if (!program) goto cleanup;

    while (1) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        set_uniform1f(program, "time", render_get_time(render));
        set_uniform2f(program, "resolution", render->width, render->height);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        if (!render_commit_frame(render)) break;
    }

    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

cleanup:
    free_render(render);
    return 0;
}
