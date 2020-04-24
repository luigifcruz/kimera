#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>

typedef struct {
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* tex;
} DisplayState;

bool start_display(DisplayState*);
void close_display(DisplayState*);
bool display_draw(DisplayState*, AVFrame*);

#endif