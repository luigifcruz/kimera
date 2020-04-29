#include "display.h"

bool start_display(DisplayState* state) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("[SDL] Init error: %s.\n", SDL_GetError());
		goto cleanup;
	}

	if (TTF_Init() != 0) {
		printf("[SDL] TTF init error.\n");
		state->font = NULL;
	}
	state->font = TTF_OpenFont("./OxygenMono.ttf", 50);

	if (!(state->win = SDL_CreateWindow(
					  "Camera Loopback", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                      1920/2, 1080/2, SDL_WINDOW_SHOWN))) {
		printf("[SDL] Window error: %s\n", SDL_GetError());
		goto cleanup;
	}
	
	if (!(state->ren = SDL_CreateRenderer(state->win, -1, 
					  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
		printf("[SDL] Renderer error: %s\n", SDL_GetError());
		goto cleanup;
	}

	if (!(state->tex = SDL_CreateTexture(state->ren, SDL_PIXELFORMAT_YV12,
					  SDL_TEXTUREACCESS_STREAMING, 1920, 1080))) {
		printf("[SDL] Error: %s\n", SDL_GetError());
		goto cleanup;
	}

	SDL_RenderClear(state->ren);
	SDL_RenderCopy(state->ren, state->tex, NULL, NULL);
	SDL_RenderPresent(state->ren);

	return true;

cleanup:
	close_display(state);
	return false;
}

void close_display(DisplayState* state) {
	if (state->tex)
		SDL_DestroyTexture(state->tex);
	if (state->ren)
		SDL_DestroyRenderer(state->ren);
	if (state->win)
		SDL_DestroyWindow(state->win);
	SDL_Quit();
	TTF_Quit();
}

bool display_draw(DisplayState* state, AVFrame* frame) {
	SDL_Event e;
    while(SDL_PollEvent(&e)){
        switch(e.type){
            case SDL_QUIT:
                return false;
        }
    }

	SDL_RenderClear(state->ren);
	
	SDL_UpdateYUVTexture(state->tex, NULL,
						 frame->data[0], frame->linesize[0],
						 frame->data[1], frame->linesize[1],
						 frame->data[2], frame->linesize[2]);
	SDL_RenderCopy(state->ren, state->tex, NULL, NULL);

	if (state->font) {
		char buffer[32];
		sprintf((char*)&buffer, "%ld", (int64_t)frame->pts);
		SDL_Color textColor = { 255, 255, 255, 255 };

		SDL_Surface *textSurface = TTF_RenderText_Solid(state->font, buffer, textColor);
		SDL_Texture *textTexture = SDL_CreateTextureFromSurface(state->ren, textSurface);
		
		SDL_Rect textRect = { 0, 0, textSurface->w, textSurface->h };
		SDL_RenderCopy(state->ren, textTexture, NULL, &textRect);
		SDL_DestroyTexture(textTexture);
	}
	
	SDL_RenderPresent(state->ren);
	return true;
}
