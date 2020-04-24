#include "display.h"

bool start_display(DisplayState* state) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("[SDL] Init error: %s\n", SDL_GetError());
		goto cleanup;
	}

	if ((state->win = SDL_CreateWindow(
					  "Camera Loopback", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                      960, 540, SDL_WINDOW_SHOWN)) == NULL) {
		printf("[SDL] Window error: %s\n", SDL_GetError());
		goto cleanup;
	}
	
	if ((state->ren = SDL_CreateRenderer(state->win, -1, 
					  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
		printf("[SDL] Renderer error: %s\n", SDL_GetError());
		goto cleanup;
	}

	if ((state->tex = SDL_CreateTexture(state->ren, SDL_PIXELFORMAT_YV12,
					  SDL_TEXTUREACCESS_STREAMING, 1920, 1080)) == NULL) {
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
	if (state->tex != NULL)
		SDL_DestroyTexture(state->tex);
	if (state->ren != NULL)
		SDL_DestroyRenderer(state->ren);
	if (state->win != NULL)
		SDL_DestroyWindow(state->win);
	SDL_Quit();
}

bool display_draw(DisplayState* state, AVFrame* frame) {
	SDL_RenderClear(state->ren);		
	SDL_UpdateYUVTexture(state->tex, NULL,
            frame->data[0], frame->linesize[0],
            frame->data[1], frame->linesize[1],
            frame->data[2], frame->linesize[2]);
	SDL_RenderCopy(state->ren, state->tex, NULL, NULL);
	SDL_RenderPresent(state->ren);
}