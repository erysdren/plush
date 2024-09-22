// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#include "SDL.h"

static char *exGraphMem;
static SDL_Window *exWindow;
static SDL_Surface *exWindowSurface;
static SDL_Surface *exSurface;

static int exGetKey(void)
{
	return SDL_QuitRequested();
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

	exWindow = SDL_CreateWindow("Plush Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 200, SDL_WINDOW_SHOWN);

	exWindowSurface = SDL_GetWindowSurface(exWindow);

	exSurface = SDL_CreateRGBSurfaceWithFormat(0, 320, 200, 8, SDL_PIXELFORMAT_INDEX8);

	exGraphMem = exSurface->pixels;
}

static void exSetText(void)
{
	SDL_FreeSurface(exSurface);
	SDL_DestroyWindow(exWindow);
	SDL_Quit();
}
