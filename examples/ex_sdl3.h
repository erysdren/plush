// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#include <SDL3/SDL.h>

#ifdef __PSP__
#define W (480)
#define H (272)
#else
#define W (640)
#define H (480)
#endif

uint8_t *exGraphMem = NULL;
SDL_Window *exWindow = NULL;
SDL_Renderer *exRenderer = NULL;
SDL_Texture *exTexture = NULL;
SDL_Surface *exWindowSurface = NULL;
SDL_Surface *exSurface = NULL;
int exMouseButtons = 0;
int exMouseX = 0;
int exMouseY = 0;
int exMouseDeltaX = 0;
int exMouseDeltaY = 0;

int exClockPerSecond(void)
{
	return 1000;
}

int exClock(void)
{
	return (int)SDL_GetTicks();
}

int exGetKey(void)
{
	SDL_Event event;
	int lastkey = 0;

	exMouseDeltaX = exMouseDeltaY = 0;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT:
				lastkey = 27;
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
					exMouseButtons |= 1;
				if (event.button.button == SDL_BUTTON_RIGHT)
					exMouseButtons |= 2;
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (event.button.button == SDL_BUTTON_LEFT)
					exMouseButtons &= ~1;
				if (event.button.button == SDL_BUTTON_RIGHT)
					exMouseButtons &= ~2;
				break;

			case SDL_EVENT_MOUSE_MOTION:
				exMouseX = event.motion.x;
				exMouseY = event.motion.y;
				exMouseDeltaX = event.motion.xrel;
				exMouseDeltaY = event.motion.yrel;
				break;

			case SDL_EVENT_KEY_DOWN:
				lastkey = event.key.key;
				break;
		}
	}

	return lastkey;
}

void exSetPalette(uint8_t palette[768])
{
	int i;
	SDL_Color colors[256];

	for (i = 0; i < 256; i++)
	{
		colors[i].r = palette[i * 3];
		colors[i].g = palette[i * 3 + 1];
		colors[i].b = palette[i * 3 + 2];
		colors[i].a = 255;
	}

	SDL_SetPaletteColors(SDL_CreateSurfacePalette(exSurface), colors, 0, 256);
}

int exBegin(int argc, char **argv, const char *title)
{
	int r = PL_EXIT_CONTINUE;
	void *appstate = NULL;

	if ((r = exInit(&appstate, argc, argv)) != PL_EXIT_CONTINUE)
		return r;

	if (!SDL_Init(SDL_INIT_VIDEO))
		return 1;

	if (!SDL_CreateWindowAndRenderer(title, W, H, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &exWindow, &exRenderer))
		return 1;

	SDL_SetRenderVSync(exRenderer, 1);

	SDL_SetRenderLogicalPresentation(exRenderer, W, H, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	exTexture = SDL_CreateTexture(exRenderer, SDL_GetWindowPixelFormat(exWindow), SDL_TEXTUREACCESS_STREAMING, W, H);
	if (!exTexture)
		return 1;

	SDL_SetTextureScaleMode(exTexture, SDL_SCALEMODE_NEAREST);

	exWindowSurface = SDL_CreateSurfaceFrom(W, H, SDL_GetWindowPixelFormat(exWindow), NULL, 0);
	if (!exWindowSurface)
		return 1;

	exSurface = SDL_CreateSurface(W, H, SDL_PIXELFORMAT_INDEX8);
	if (!exSurface)
		return 1;

	exGraphMem = (uint8_t *)exSurface->pixels;

	SDL_ShowWindow(exWindow);

	while (true)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					goto done;

				case SDL_EVENT_KEY_DOWN:
					if ((r = exKeyEvent(appstate, event.key.key)) != PL_EXIT_CONTINUE)
						goto done;
					break;
			}
		}

		if ((r = exIterate(appstate)) != PL_EXIT_CONTINUE)
			goto done;

		if (SDL_LockTexture(exTexture, NULL, &exWindowSurface->pixels, &exWindowSurface->pitch))
		{
			SDL_BlitSurface(exSurface, NULL, exWindowSurface, NULL);
			SDL_UnlockTexture(exTexture);
		}

		SDL_RenderClear(exRenderer);
		SDL_RenderTexture(exRenderer, exTexture, NULL, NULL);
		SDL_RenderPresent(exRenderer);
	}

done:
	exQuit(appstate, r);
	SDL_DestroySurface(exWindowSurface);
	SDL_DestroySurface(exSurface);
	SDL_DestroyTexture(exTexture);
	SDL_DestroyRenderer(exRenderer);
	SDL_DestroyWindow(exWindow);
	SDL_Quit();
	return r;
}
