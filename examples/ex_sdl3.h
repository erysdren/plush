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

static void exWaitVSync(void)
{
	if (SDL_LockTexture(exTexture, NULL, &exWindowSurface->pixels, &exWindowSurface->pitch))
	{
		SDL_BlitSurface(exSurface, NULL, exWindowSurface, NULL);
		SDL_UnlockTexture(exTexture);
	}

	SDL_RenderClear(exRenderer);
	SDL_RenderTexture(exRenderer, exTexture, NULL, NULL);
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

	SDL_SetPaletteColors(SDL_CreateSurfacePalette(exSurface), colors, 0, 256);
}

static void exSetGraphics(void)
{
	uint32_t format;

	SDL_Init(SDL_INIT_VIDEO);

	exWindow = SDL_CreateWindow("Plush Example", W, H, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);

	exRenderer = SDL_CreateRenderer(exWindow, NULL);
	SDL_SetRenderVSync(exRenderer, 1);

	SDL_SetRenderLogicalPresentation(exRenderer, W, H, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	format = SDL_GetWindowPixelFormat(exWindow);

	exTexture = SDL_CreateTexture(exRenderer, format, SDL_TEXTUREACCESS_STREAMING, W, H);

	exWindowSurface = SDL_CreateSurfaceFrom(W, H, format, NULL, 0);

	exSurface = SDL_CreateSurface(W, H, SDL_PIXELFORMAT_INDEX8);

	exGraphMem = (uint8_t *)exSurface->pixels;

	SDL_ShowWindow(exWindow);
}

static void exSetText(void)
{
	SDL_DestroySurface(exWindowSurface);
	SDL_DestroySurface(exSurface);
	SDL_DestroyTexture(exTexture);
	SDL_DestroyRenderer(exRenderer);
	SDL_DestroyWindow(exWindow);
	SDL_Quit();
}
