#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>

#include "config.h"

typedef struct {
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* tex;
    TTF_Font* font;
    SDL_Event* event;
    bool info;
} DisplayState;

bool start_display(DisplayState*, State*);
void close_display(DisplayState*);
bool display_draw(DisplayState*, State*, AVFrame*);

#endif
