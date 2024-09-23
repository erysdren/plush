// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#include "SDL.h"

#ifndef W
#define W (640)
#endif

#ifndef H
#define H (480)
#endif

static char *exGraphMem = NULL;
static SDL_Window *exWindow = NULL;
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
				exit(0);

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
	SDL_Delay(10);
	SDL_BlitSurface(exSurface, NULL, exWindowSurface, NULL);
	SDL_UpdateWindowSurface(exWindow);
}

static void exSetPalette(char *palette)
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

static void exSetGraphics(void)
{
	SDL_Init(SDL_INIT_VIDEO);

	exWindow = SDL_CreateWindow("Plush Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_SHOWN);

	exWindowSurface = SDL_GetWindowSurface(exWindow);

	exSurface = SDL_CreateRGBSurfaceWithFormat(0, W, H, 8, SDL_PIXELFORMAT_INDEX8);

	exGraphMem = exSurface->pixels;
}

static void exSetText(void)
{
	SDL_FreeSurface(exSurface);
	SDL_DestroyWindow(exWindow);
	SDL_Quit();
}
