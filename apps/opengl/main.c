
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

const float vertices[] = {
     1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f 
};

const unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

void transmitter(State* state, volatile sig_atomic_t* stop) {
    kimera_print_state(state);

    RenderState* render = alloc_render();

    render->mode = WINDOWED;
    render->api  = EGL_OPENGL_API;

    if (!start_render(render, state)) goto cleanup;
    render_print_meta(render);

    //glEnable(GL_BLEND); 
    //glEnable(GL_MULTISAMPLE);
    
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
    
    //
    // OpenGL Stuff
    //

    unsigned int VBO, EBO, FBO, RT_A, RT_B;
    
    bool configured = false;
    unsigned int planes = 0;
    unsigned int textures[8];

    // Generate Vertices Buffer Object
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate Element Buffer Object
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
   
    // Setup Processing FrameBuffer (this is different than the EGL PBuffer)
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenTextures(1, &RT_A);
        glBindTexture(GL_TEXTURE_2D, RT_A);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->width, state->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RT_A, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &RT_B);
        glBindTexture(GL_TEXTURE_2D, RT_B);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->width, state->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, RT_B, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Load All Shaders Programs
    unsigned int frameShader = load_shader("./shaders/frame.vs", "./shaders/frame.fs");
    unsigned int filterShader = load_shader("./shaders/filter.vs", "./shaders/filter.fs");
    unsigned int displayShader = load_shader("./shaders/display.vs", "./shaders/display.fs");
    if (!frameShader || !displayShader) goto cleanup;

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback, state) && !(*stop)) {
        if (!resampler_push_frame(&resampler, state, loopback.frame))
            continue;

        AVFrame* frame = resampler.frame;

        // Make Color Convertion (X to RGBA)
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glViewport(0, 0, state->width, state->height);
        
        set_draw_buffer(GL_COLOR_ATTACHMENT0);

        if (!configured) {
            for (int i = 0; i < 8; i++) {
                if (frame->linesize[i] == 0) break;
                planes++;
            }

            glGenTextures(planes, textures);
            for (unsigned int i = 0; i < planes; i++) {
                 glBindTexture(GL_TEXTURE_2D, textures[i]);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glBindTexture(GL_TEXTURE_2D, 0);
            }

            configured = true;
        }

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(frameShader);
        
        char plane_id[10];
        for (unsigned int i = 0; i < planes; i++) {
            int ratio = frame->width / frame->linesize[i];
            //printf("%d: %d %d %d\n", i, frame->linesize[i], ratio, frame->width/ratio);
            
            sprintf(plane_id, "plane%d", i);
            set_uniform1i(frameShader, plane_id, i);
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame->width/ratio, frame->height/ratio, 
                         0, GL_RED, GL_UNSIGNED_BYTE, frame->data[i]);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Add Custom Filter
        glUseProgram(filterShader);
        set_draw_buffer(GL_COLOR_ATTACHMENT1);

        set_uniform1f(filterShader, "time", glfwGetTime());
        set_uniform2f(filterShader, "resolution", state->width, state->height);

        set_uniform1i(filterShader, "renderedTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RT_A);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw Frame To Display
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, render->width, render->height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(displayShader);

        float imgAspectRatio = (float)state->width / (float)state->height;
        float viewAspectRatio = (float)render->width / (float)render->height;

        float xScale = 1.0f, yScale = 1.0f;
        if (imgAspectRatio > viewAspectRatio) {
            yScale = viewAspectRatio / imgAspectRatio;
        } else {
            xScale = imgAspectRatio / viewAspectRatio;
        }
        set_uniform2f(displayShader, "ScaleFact", xScale, yScale);
        
        set_uniform1i(displayShader, "renderedTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RT_B);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        if (!render_commit_frame(render)) break;

        if (encoder_push(&encoder, resampler.frame))
            socket_send_packet(&socket, encoder.packet);
    }

    //glDeleteBuffers(1, &EBO);
    //glDeleteBuffers(1, &VBO);
    //glDeleteProgram(frameShader);

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
