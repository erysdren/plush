// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#include "SDL.h"

#ifdef __PSP__
#define W (480)
#define H (272)
#else
#define W (640)
#define H (480)
#endif

static uint8_t *exGraphMem = NULL;
static SDL_Window *exWindow = NULL;
static SDL_Renderer *exRenderer = NULL;
static SDL_Texture *exTexture = NULL;
static SDL_Surface *exWindowSurface = NULL;
static SDL_Surface *exSurface = NULL;
static int exMouseButtons = 0;
static int exMouseX = 0;
static int exMouseY = 0;
static int exMouseDeltaX = 0;
static int exMouseDeltaY = 0;

static int exClockPerSecond(void)
{
	return 1000;
}

static int exClock(void)
{
	return (int)SDL_GetTicks();
}

static int exGetKey(void)
{
	SDL_Event event;
	int lastkey = 0;

	exMouseDeltaX = exMouseDeltaY = 0;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				lastkey = 27;
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
					exMouseButtons |= 1;
				if (event.button.button == SDL_BUTTON_RIGHT)
					exMouseButtons |= 2;
				break;

			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
					exMouseButtons &= ~1;
				if (event.button.button == SDL_BUTTON_RIGHT)
					exMouseButtons &= ~2;
				break;

			case SDL_MOUSEMOTION:
				exMouseX = event.motion.x;
				exMouseY = event.motion.y;
				exMouseDeltaX = event.motion.xrel;
				exMouseDeltaY = event.motion.yrel;
				break;

			case SDL_KEYDOWN:
				lastkey = event.key.keysym.sym;
				break;
		}
	}

	return lastkey;
}

static void exWaitVSync(void)
{
	if (SDL_LockTexture(exTexture, NULL, &exWindowSurface->pixels, &exWindowSurface->pitch) == 0)
	{
		SDL_BlitSurface(exSurface, NULL, exWindowSurface, NULL);
		SDL_UnlockTexture(exTexture);
	}

	SDL_RenderClear(exRenderer);
	SDL_RenderCopy(exRenderer, exTexture, NULL, NULL);
	SDL_RenderPresent(exRenderer);
}

static void exSetPalette(uint8_t palette[768])
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

	SDL_SetPaletteColors(exSurface->format->palette, colors, 0, 256);
}

static void exSetText(void);

static void exSetGraphics(void)
{
	Uint32 format;

	SDL_Init(SDL_INIT_VIDEO);

	exWindow = SDL_CreateWindow("Plush Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);

	exRenderer = SDL_CreateRenderer(exWindow, -1, SDL_RENDERER_PRESENTVSYNC);

	SDL_RenderSetLogicalSize(exRenderer, W, H);

	format = SDL_GetWindowPixelFormat(exWindow);

	exTexture = SDL_CreateTexture(exRenderer, format, SDL_TEXTUREACCESS_STREAMING, W, H);

	exWindowSurface = SDL_CreateRGBSurfaceWithFormatFrom(NULL, W, H, 0, 0, format);

	exSurface = SDL_CreateRGBSurfaceWithFormat(0, W, H, 0, SDL_PIXELFORMAT_INDEX8);

	exGraphMem = (uint8_t *)exSurface->pixels;

	SDL_ShowWindow(exWindow);

	atexit(exSetText);
}

static void exSetText(void)
{
	SDL_FreeSurface(exWindowSurface);
	SDL_FreeSurface(exSurface);
	SDL_DestroyTexture(exTexture);
	SDL_DestroyRenderer(exRenderer);
	SDL_DestroyWindow(exWindow);
	SDL_Quit();
}
