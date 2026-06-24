// sprite.c: a more complex scene with a rotating camera and a sprite plane
// owo

#include "ex.h"

#define NUM_MATERIALS 2

static pl_Light *light1, *light2;
static pl_Obj *world;
static pl_Obj *sprite;
static pl_Mat *materials[NUM_MATERIALS];
static pl_Cam *camera;
static uint8_t framebuffer[W * H];
static float zbuffer[W * H];
static uint8_t palette[768];
static float camera_angle = 0;
static float light1_angle = 0;

int exInit(void **appstate, int argc, char **argv)
{
	int i;

	/* create materials */
	materials[0] = plMatCreate();
	materials[0]->ShadeType = PL_SHADE_GOURAUD;

	materials[1] = plMatCreate();
	materials[1]->ShadeType = PL_SHADE_NONE;
	materials[1]->Diffuse[0] = 255;
	materials[1]->Diffuse[1] = 0;
	materials[1]->Diffuse[2] = 0;
	materials[1]->Transparent = 2;

	/* create world */
	world = plObjCreate(NULL);
	world->Model = plMakeBox(128, 128, 8, materials[0]);
	for (i = 0; i < 6; i++)
	{
		float ang;
		pl_Obj *cube = plObjCreate(world);
		cube->Model = plMakeBox(16, 16, 64, materials[0]);

		ang = ((2 * PL_PI) / 6) * i;
		cube->Xp = 48 * cos(ang);
		cube->Zp = 48 * sin(ang);
		cube->Ya = 90 + atan2(cube->Zp, cube->Xp) * 180 / PL_PI;
	}

	/* create sprite */
	sprite = plObjCreate(NULL);
	sprite->Model = plMakePlane(32, 32, 8, materials[1]);
	sprite->Yp += 16;

	/* initialize materials */
	for (i = 0; i < NUM_MATERIALS; i++)
		plMatInit(materials[i]);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, materials, NUM_MATERIALS);
	palette[0] = palette[1] = palette[2] = 0;
	for (i = 0; i < NUM_MATERIALS; i++)
		plMatMapToPal(materials[i], palette, 0, 255);

	exSetPalette(palette);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Y = 32;
	camera->Z = -160;

	/* create lights */
	light1 = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, -90.0, 30.0, -45.0, 1.0, 1.0);
	light2 = plLightSet(plLightCreate(), PL_LIGHT_POINT, 0.0, 24.0, 0.0, 1.0, 256.0);

	return PL_EXIT_CONTINUE;
}

int exKeyEvent(void *appstate, int key)
{
	// any keypress will trigger an exit
	return PL_EXIT_SUCCESS;
}

int exIterate(void *appstate)
{
	/* clear back buffer */
	plMemSet(zbuffer, 0, sizeof(zbuffer));
	plMemSet(framebuffer, 0, sizeof(framebuffer));

	/* rotate camera */
	camera_angle += 0.01;
	camera->X = 160 * cos(camera_angle);
	camera->Z = 160 * sin(camera_angle);
	camera->Pan = 90 + atan2(camera->Z, camera->X) * 180 / PL_PI;

	/* rotate sprite to face camera */
	sprite->Xa = 90;
	sprite->Ya = camera->Pan;

	/* rotate vector light */
	light1_angle -= 0.01;
	light1->Xp = cos(light1_angle);
	light1->Zp = sin(light1_angle);

	/* render frame */
	plRenderBegin(camera);
	plRenderLight(light1);
	plRenderLight(light2);
	plRenderObj(world);
	plRenderObj(sprite);
	plRenderEnd();

	/* copy to screen */
	plMemCpy(exGraphMem, framebuffer, sizeof(framebuffer));

	return PL_EXIT_CONTINUE;
}

void exQuit(void *appstate, int code)
{
	int i;

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light1);
	plLightDelete(light2);
	plObjDelete(world);
	for (i = 0; i < NUM_MATERIALS; i++)
		plMatDelete(materials[i]);
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "sprite");
}
