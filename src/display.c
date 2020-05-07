#include "display.h"

bool start_display(DisplayState* display, State* state) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("[SDL] Init error: %s.\n", SDL_GetError());
		goto cleanup;
	}

	if (TTF_Init() != 0) {
		printf("[SDL] TTF init error.\n");
		display->font = NULL;
	}
	display->font = TTF_OpenFont("CourierPrime-Regular.ttf", 50);

	char* name[32];
	switch (state->mode) {
		case TRANSMITTER:
			sprintf((char*)name, "Kimera - Transmitter Display");
			break;
		case RECEIVER:
			sprintf((char*)name, "Kimera - Receiver Display");
			break;
	}	

	if (!(display->win = SDL_CreateWindow(
					  (char*)name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                      state->width/2, state->height/2, SDL_WINDOW_SHOWN))) {
		printf("[SDL] Window error: %s\n", SDL_GetError());
		goto cleanup;
	}
	
	if (!(display->ren = SDL_CreateRenderer(display->win, -1, 
					  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
		printf("[SDL] Renderer error: %s\n", SDL_GetError());
		goto cleanup;
	}

	if (!(display->tex = SDL_CreateTexture(display->ren, SDL_PIXELFORMAT_YV12,
					  SDL_TEXTUREACCESS_STREAMING, state->width, state->height))) {
		printf("[SDL] Error: %s\n", SDL_GetError());
		goto cleanup;
	}

	SDL_RenderClear(display->ren);
	SDL_RenderCopy(display->ren, display->tex, NULL, NULL);
	SDL_RenderPresent(display->ren);

	return true;

cleanup:
	close_display(display);
	return false;
}

void close_display(DisplayState* display) {
	if (display->tex)
		SDL_DestroyTexture(display->tex);
	if (display->ren)
		SDL_DestroyRenderer(display->ren);
	if (display->win)
		SDL_DestroyWindow(display->win);
	SDL_Quit();
	TTF_Quit();
}

bool display_draw(DisplayState* display, AVFrame* frame) {
	SDL_Event e;
    while(SDL_PollEvent(&e)){
        switch(e.type){
            case SDL_QUIT:
                return false;
        }
    }

	SDL_RenderClear(display->ren);
	
	SDL_UpdateYUVTexture(display->tex, NULL,
						 frame->data[0], frame->linesize[0],
						 frame->data[1], frame->linesize[1],
						 frame->data[2], frame->linesize[2]);
	SDL_RenderCopy(display->ren, display->tex, NULL, NULL);

	if (display->font) {
		char buffer[32];
		sprintf((char*)&buffer, "%ld", (int64_t)frame->pts);
		SDL_Color textColor = { 255, 255, 255, 255 };

		SDL_Surface *textSurface = TTF_RenderText_Solid(display->font, buffer, textColor);
		SDL_Texture *textTexture = SDL_CreateTextureFromSurface(display->ren, textSurface);
		
		SDL_Rect textRect = { 0, 0, textSurface->w, textSurface->h };
		SDL_RenderCopy(display->ren, textTexture, NULL, &textRect);
		SDL_DestroyTexture(textTexture);
	}
	
	SDL_RenderPresent(display->ren);
	return true;
}
