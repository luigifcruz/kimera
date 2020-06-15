
#define KIMERA_WINDOW_GLFW

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "kimera/state.h"

#include "kimera/transport.h"
#include "kimera/codec.h"
#include "kimera/loopback.h"
#include "kimera/display.h"
#include "kimera/client.h"
#include "render/render.h"
#include "render/meta.h"

void transmitter(State* state, volatile sig_atomic_t* stop) {
    kimera_print_state(state);

    RenderState* render = alloc_render();

    render->mode = WINDOWED;
    render->api  = EGL_OPENGL_ES_API;

    if (!start_render(render, state)) goto cleanup;
    render_print_meta(render);

    float vertices[] = {
         1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f 
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


    bool configured = false;
    unsigned int buffer_size = 0;
    unsigned int textures[8];

    // Start Socket Server. 
    SocketState socket;
    if (!open_socket_server(&socket, state))
        goto cleanup;

    // Start Loopback Input.
    LoopbackState loopback;
    if (!open_loopback_source(&loopback, state))
        goto cleanup;

    // Start Encoder.
    EncoderState encoder;
    if (!start_encoder(&encoder, state))
        goto cleanup;

    // Add resampler.
    ResamplerState resampler;
    open_resampler(&resampler, state->out_format);

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback, state) && !(*stop)) {
        if (!resampler_push_frame(&resampler, state, loopback.frame))
            continue;

        AVFrame* frame = resampler.frame;

        if (!configured) {
            for (int i = 0; i < 8; i++) {
                if (frame->linesize[i] == 0) break;
                buffer_size++;
            }
            
            glGenTextures(buffer_size, textures);
            for (unsigned int i = 0; i < buffer_size; i++) {
                glBindTexture(GL_TEXTURE_2D, textures[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            configured = true;
            printf("Buffer Size: %d\n", buffer_size);
        }

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(frameShader);
        
        for (unsigned int i = 0; i < buffer_size; i++) {
            int ratio = frame->width / frame->linesize[i];
            //printf("%d: %d %d %d\n", i, frame->linesize[i], ratio, frame->width/ratio);
            
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RED, frame->width/ratio, frame->height/ratio, 
                0, GL_RED, GL_UNSIGNED_BYTE, frame->data[i]);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        set_uniform1f(frameShader, "time", render_get_time(render));
        set_uniform2f(frameShader, "resolution", render->width, render->height);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        if (!render_commit_frame(render)) break;

        if (encoder_push(&encoder, resampler.frame))
            socket_send_packet(&socket, encoder.packet);
    }

    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(frameShader);

cleanup:
    close_socket(&socket);
    close_resampler(&resampler);
    close_loopback(&loopback, state);
    close_encoder(&encoder);
    free_render(render);
}

void receiver(State* state, volatile sig_atomic_t* stop) {}

int main(int argc, char *argv[]) {
    return kimera_client(argc, argv, transmitter, receiver);
}
