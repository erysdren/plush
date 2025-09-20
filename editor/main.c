
#include <SDL3/SDL.h>
#include <plush/plush.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#define log_error(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define log_warning(...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#define WINDOW_TITLE "Plush Editor"
#define WINDOW_WIDTH (800)
#define WINDOW_HEIGHT (600)

#define FRAMEBUFFER_SIZE (WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint8_t))
#define ZBUFFER_SIZE (WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(float))

#define DEFAULT_FOV (90)
#define DEFAULT_SENSITIVITY (8)
#define DEFAULT_MOVESPEED (32)
#define DEFAULT_STRAFESPEED (32)

#define MAX_MATERIALS (256)
#define MAX_VELOCITY (128)

/*
 *
 * this holds all the editor state
 *
 */

typedef struct app {
	/* sdl video state */
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Surface *surface;
	SDL_Surface *surface8;
	SDL_Palette *palette;

	/* plush video state */
	uint8_t *framebuffer;
	float *zbuffer;
	pl_Cam *camera;
	pl_Light *headlight;

	/* camera state */
	float velocity[3];
	float movespeed;
	float strafespeed;

	/* assets */
	pl_Mat *fallback_material;

	/* timer state */
	uint64_t ticks;
	uint64_t deltaticks;
	float time;
	float deltatime;
} app_t;

/*
 *
 * utility functions
 *
 */

static SDL_AppResult die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, fmt, ap);
	va_end(ap);

	SDL_Event event;
	event.quit.type = SDL_EVENT_QUIT;
	event.quit.timestamp = SDL_GetTicksNS();
	SDL_PushEvent(&event);

	return SDL_APP_FAILURE;
}

/*
 *
 * sdl callbacks
 *
 */

SDLMAIN_DECLSPEC SDL_AppResult SDLCALL SDL_AppInit(SDL_UNUSED void **appstate, SDL_UNUSED int argc, SDL_UNUSED char *argv[])
{
	uint8_t pal[768];
	app_t *app;

	if (!SDL_Init(SDL_INIT_VIDEO))
		return die("%s", SDL_GetError());

	/* allocate app state */
	app = SDL_malloc(sizeof(app_t));
	if (!app)
		return die("%s", SDL_GetError());

	/* create window and renderer */
	SDL_CreateWindowAndRenderer(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &app->window, &app->renderer);
	SDL_SetWindowMinimumSize(app->window, WINDOW_WIDTH, WINDOW_HEIGHT);
	SDL_SetWindowRelativeMouseMode(app->window, true);
	SDL_SetRenderLogicalPresentation(app->renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	/* create render texture */
	app->texture = SDL_CreateTexture(app->renderer, SDL_GetWindowPixelFormat(app->window), SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
	SDL_SetTextureScaleMode(app->texture, SDL_SCALEMODE_NEAREST);

	/* allocate plush buffers */
	app->framebuffer = plMalloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint8_t));
	app->zbuffer = plMalloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(float));

	/* create render surfaces */
	app->surface = SDL_CreateSurfaceFrom(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_GetWindowPixelFormat(app->window), NULL, 0);
	app->surface8 = SDL_CreateSurfaceFrom(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_PIXELFORMAT_INDEX8, app->framebuffer, WINDOW_WIDTH);

	/* create camera */
	app->camera = plCamCreate(WINDOW_WIDTH, WINDOW_HEIGHT, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, DEFAULT_FOV, app->framebuffer, app->zbuffer);
	app->movespeed = DEFAULT_MOVESPEED;
	app->strafespeed = DEFAULT_STRAFESPEED;

	/* create light */
	app->headlight = plLightSet(plLightCreate(), PL_LIGHT_POINT, 0, 0, 0, 0.5, 256);

	app->fallback_material = plMatCreate();
	app->fallback_material->ShadeType = PL_SHADE_FLAT;
	app->fallback_material->Ambient[0] = 0.1745 * 255;
	app->fallback_material->Ambient[1] = 0.03175 * 255;
	app->fallback_material->Ambient[2] = 0.03175 * 255;
	app->fallback_material->Diffuse[0] = 0.61424 * 255;
	app->fallback_material->Diffuse[1] = 0.10136 * 255;
	app->fallback_material->Diffuse[2] = 0.10136 * 255;
	app->fallback_material->Specular[0] = 0.727811 * 255;
	app->fallback_material->Specular[1] = 0.626959 * 255;
	app->fallback_material->Specular[2] = 0.626959 * 255;
	plMatInit(app->fallback_material);

	/* generate palette from fallback material */
	/* first color in the palette is always black and the second color is always white */
	plMatMakeOptPal(pal, 2, 255, &app->fallback_material, 1);
	pal[0] = pal[1] = pal[2] = 0;
	pal[3] = pal[4] = pal[5] = 255;

	/* map fallback material to generated palette */
	plMatMapToPal(app->fallback_material, pal, 0, 255);

	/* convert generated palette to sdl palette */
	app->palette = SDL_CreateSurfacePalette(app->surface8);
	for (int i = 0; i < app->palette->ncolors; i++)
	{
		app->palette->colors[i].r = pal[i * 3 + 0];
		app->palette->colors[i].g = pal[i * 3 + 1];
		app->palette->colors[i].b = pal[i * 3 + 2];
		app->palette->colors[i].a = SDL_ALPHA_OPAQUE;
	}

	/* start timer */
	app->ticks = SDL_GetTicks();
	app->deltaticks = 0;
	app->time = (float)app->ticks / 1000.0f;
	app->deltatime = (float)app->deltaticks / 1000.0f;

	/* return app state */
	*appstate = app;

	return SDL_APP_CONTINUE;
}

SDLMAIN_DECLSPEC SDL_AppResult SDLCALL SDL_AppIterate(SDL_UNUSED void *appstate)
{
	const bool *keys = SDL_GetKeyboardState(NULL);
	float look[3];
	float strafe[2];
	app_t *app = (app_t *)appstate;

	/* tick world state */
	uint64_t now = SDL_GetTicks();
	app->deltaticks = now - app->ticks;
	app->ticks = now;
	app->time = (float)app->ticks / 1000.0f;
	app->deltatime = (float)app->deltaticks / 1000.0f;

	/* clear screen */
	SDL_memset(app->zbuffer, 0, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(float));
	SDL_FillSurfaceRect(app->surface8, NULL, 0);

	/* handle movement */
	look[0] = plCos(plDegToRad(app->camera->Pan + 90)) * plCos(plDegToRad(app->camera->Pitch));
	look[1] = plSin(plDegToRad(app->camera->Pitch));
	look[2] = plSin(plDegToRad(app->camera->Pan + 90)) * plCos(plDegToRad(app->camera->Pitch));

	strafe[0] = plCos(plDegToRad(app->camera->Pan));
	strafe[1] = plSin(plDegToRad(app->camera->Pan));

	if (keys[SDL_SCANCODE_LSHIFT])
	{
		app->movespeed = DEFAULT_MOVESPEED * 2;
		app->strafespeed = DEFAULT_STRAFESPEED * 2;
	}
	else
	{
		app->movespeed = DEFAULT_MOVESPEED;
		app->strafespeed = DEFAULT_STRAFESPEED;
	}

	if (keys[SDL_SCANCODE_W])
	{
		app->velocity[0] += look[0] * app->movespeed * app->deltatime;
		app->velocity[1] += look[1] * app->movespeed * app->deltatime;
		app->velocity[2] += look[2] * app->movespeed * app->deltatime;
	}

	if (keys[SDL_SCANCODE_S])
	{
		app->velocity[0] -= look[0] * app->movespeed * app->deltatime;
		app->velocity[1] -= look[1] * app->movespeed * app->deltatime;
		app->velocity[2] -= look[2] * app->movespeed * app->deltatime;
	}

	if (keys[SDL_SCANCODE_A])
	{
		app->velocity[0] -= strafe[0] * app->strafespeed * app->deltatime;
		app->velocity[2] -= strafe[1] * app->strafespeed * app->deltatime;
	}

	if (keys[SDL_SCANCODE_D])
	{
		app->velocity[0] += strafe[0] * app->strafespeed * app->deltatime;
		app->velocity[2] += strafe[1] * app->strafespeed * app->deltatime;
	}

	/* cap velocity */
	if (app->velocity[0] < -MAX_VELOCITY) app->velocity[0] = -MAX_VELOCITY;
	if (app->velocity[0] > MAX_VELOCITY) app->velocity[0] = MAX_VELOCITY;
	if (app->velocity[1] < -MAX_VELOCITY) app->velocity[1] = -MAX_VELOCITY;
	if (app->velocity[1] > MAX_VELOCITY) app->velocity[1] = MAX_VELOCITY;
	if (app->velocity[2] < -MAX_VELOCITY) app->velocity[2] = -MAX_VELOCITY;
	if (app->velocity[2] > MAX_VELOCITY) app->velocity[2] = MAX_VELOCITY;

	/* apply velocity */
	app->camera->X += app->velocity[0] * app->deltatime;
	app->camera->Y += app->velocity[1] * app->deltatime;
	app->camera->Z += app->velocity[2] * app->deltatime;

	/* keep headlight on camera */
	app->headlight->Xp = app->camera->X;
	app->headlight->Yp = app->camera->Y;
	app->headlight->Zp = app->camera->Z;

	/* render something */
	plRenderBegin(app->camera);
	plRenderLight(app->headlight);
	plRenderEnd();
	plTextPutStr(app->camera, 0, 0, 0, 1, "press escape to exit\npress space to release mouse");

	/* sync to screen */
	SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(app->renderer);
	if (SDL_LockTexture(app->texture, NULL, &app->surface->pixels, &app->surface->pitch))
	{
		SDL_BlitSurface(app->surface8, NULL, app->surface, NULL);
		SDL_UnlockTexture(app->texture);
	}
	SDL_RenderTexture(app->renderer, app->texture, NULL, NULL);
	SDL_RenderPresent(app->renderer);

	return SDL_APP_CONTINUE;
}

SDLMAIN_DECLSPEC SDL_AppResult SDLCALL SDL_AppEvent(SDL_UNUSED void *appstate, SDL_Event *event)
{
	app_t *app = (app_t *)appstate;

	if (event->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;

	switch (event->type)
	{
		case SDL_EVENT_KEY_DOWN:
		{
			if (event->key.scancode == SDL_SCANCODE_ESCAPE)
			{
				return SDL_APP_SUCCESS;
			}
			else if (event->key.scancode == SDL_SCANCODE_SPACE)
			{
				if (SDL_GetWindowRelativeMouseMode(app->window))
				{
					int w, h;
					SDL_GetWindowSize(app->window, &w, &h);
					SDL_WarpMouseInWindow(app->window, w/2.0f, h/2.0f);
					SDL_SetWindowRelativeMouseMode(app->window, false);
				}
				else
				{
					SDL_SetWindowRelativeMouseMode(app->window, true);
				}
			}
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
		{
			if (SDL_GetWindowRelativeMouseMode(app->window))
			{
				app->camera->Pitch -= event->motion.yrel * app->deltatime * DEFAULT_SENSITIVITY;
				app->camera->Pan -= event->motion.xrel * app->deltatime * DEFAULT_SENSITIVITY;
			}
			break;
		}
	}

	return SDL_APP_CONTINUE;
}

SDLMAIN_DECLSPEC void SDLCALL SDL_AppQuit(SDL_UNUSED void *appstate, SDL_UNUSED SDL_AppResult result)
{
	app_t *app = (app_t *)appstate;

	if (app)
	{
		if (app->headlight) plLightDelete(app->headlight);
		if (app->fallback_material) plMatDelete(app->fallback_material);
		if (app->surface) SDL_DestroySurface(app->surface);
		if (app->surface8) SDL_DestroySurface(app->surface8);
		if (app->texture) SDL_DestroyTexture(app->texture);
		if (app->renderer) SDL_DestroyRenderer(app->renderer);
		if (app->window) SDL_DestroyWindow(app->window);
		if (app->camera) plCamDelete(app->camera);
		if (app->framebuffer) plFree(app->framebuffer);
		if (app->zbuffer) plFree(app->zbuffer);
		SDL_free(app);
	}

	SDL_Quit();
}
