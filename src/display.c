#include "kimera/display.h"

int display_event_handler(void* display_ptr, SDL_Event* event) {
	DisplayState* display = display_ptr;
	if (event->type == SDL_QUIT)
		raise(SIGINT);
	if (event->type == SDL_KEYUP) {
		if (event->key.keysym.sym == SDLK_ESCAPE ||
			event->key.keysym.sym == SDLK_q) 
			raise(SIGINT);

		if (event->key.keysym.sym == SDLK_i) 
			display->info = !display->info;

		if (event->key.keysym.sym == SDLK_f) {
			display->fullscreen = !display->fullscreen;
			if (display->fullscreen)
				SDL_SetWindowFullscreen(display->win, SDL_WINDOW_FULLSCREEN_DESKTOP);
			else
				SDL_SetWindowFullscreen(display->win, 0);
		}
	}
	return 1;
}

DisplayState* alloc_display() {
    DisplayState* state = malloc(sizeof(DisplayState));
    state->win   = NULL;
    state->ren   = NULL;
    state->tex 	 = NULL;
    state->font  = NULL;
    state->event = NULL;
    return state;
}

void free_display(DisplayState* display) {
	if (display->tex)
		SDL_DestroyTexture(display->tex);
	if (display->ren)
		SDL_DestroyRenderer(display->ren);
	if (display->win)
		SDL_DestroyWindow(display->win);
	if (display->font)
		TTF_CloseFont(display->font);
	SDL_Quit();
	TTF_Quit();
	free(display);
}

bool open_display(DisplayState* display, State* state) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("[SDL] Init error: %s.\n", SDL_GetError());
		goto cleanup;
	}

	if (TTF_Init() != 0) {
		printf("[SDL] TTF init error.\n");
	}

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
                      state->width/1.5, state->height/1.5, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE))) {
		printf("[SDL] Window error: %s\n", SDL_GetError());
		goto cleanup;
	}
	
	if (!(display->ren = SDL_CreateRenderer(display->win, -1, 
					  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
		printf("[SDL] Renderer error: %s\n", SDL_GetError());
		goto cleanup;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(display->ren, state->width, state->height);

	if (!(display->tex = SDL_CreateTexture(display->ren, SDL_PIXELFORMAT_YV12,
					  SDL_TEXTUREACCESS_STREAMING, state->width, state->height))) {
		printf("[SDL] Error: %s\n", SDL_GetError());
		goto cleanup;
	}

	SDL_AddEventWatch(display_event_handler, display);

	SDL_RenderClear(display->ren);
	SDL_RenderCopy(display->ren, display->tex, NULL, NULL);
	SDL_RenderPresent(display->ren);

	display->font 		= TTF_OpenFont("CourierPrime-Regular.ttf", 25);
	display->info 		= false;
	display->fullscreen = false;
	display->ishidden 	= true;

	return true;

cleanup:
	free_display(display);
	return false;
}

void display_draw(DisplayState* display, State* state, AVFrame* frame) {
	if (display->ishidden)
		SDL_ShowWindow(display->win);

	SDL_PollEvent(display->event);
	SDL_RenderClear(display->ren);
	
	SDL_UpdateYUVTexture(display->tex, NULL, frame->data[0], frame->linesize[0],
						 					 frame->data[1], frame->linesize[1],
						 					 frame->data[2], frame->linesize[2]);
	SDL_RenderCopy(display->ren, display->tex, NULL, NULL);

	if (display->font && display->info) {
		int width, height;
		char buffer[512];
		int length = 0;

		SDL_GetWindowSize(display->win, &width, &height);

		length += sprintf((char*)&buffer+length, "PTS: %lld, ", (long long)frame->pts);
		length += sprintf((char*)&buffer+length, "Frame: %dx%d@%d, ", frame->width, frame->height, state->framerate);
		length += sprintf((char*)&buffer+length, "Viewport: %dx%d, ", width, height);
		length += sprintf((char*)&buffer+length, "Codec: %s, ", state->codec);
		length += sprintf((char*)&buffer+length, "Pixel: %s -> %s",  av_get_pix_fmt_name(state->in_format), av_get_pix_fmt_name(state->out_format));

		SDL_Color textColor = { 255, 255, 255, 255 };
		SDL_Color shadeColor = { 0, 0, 0, 128 };
		SDL_Surface *textSurface = TTF_RenderText_Shaded(display->font, buffer, textColor, shadeColor);
		SDL_Texture *textTexture = SDL_CreateTextureFromSurface(display->ren, textSurface);
		
		SDL_Rect textRect = { 0, 0, textSurface->w, textSurface->h };
		SDL_RenderCopy(display->ren, textTexture, NULL, &textRect);
		SDL_DestroyTexture(textTexture);
	}
	
	SDL_RenderPresent(display->ren);
}
