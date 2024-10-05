// model.c: eggbug model example
// <3

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h> 

#include "ex.h" 

pl_Obj *model;
pl_Mat *material;
pl_Cam *camera;
uint8_t framebuffer[W * H];
pl_ZBuffer zbuffer[W * H];
uint8_t palette[768];

#define NUM_EGGBUGS (32)
struct {
	pl_Float speed;
	pl_Obj *model;
} eggbugs[NUM_EGGBUGS];

static int rangerandom(int min, int max)
{
	return rand() % (max + 1 - min) + min;
}

int main(int argc, char **argv)
{
	int now, then;
	float dt;
	int i;

	/* setup graphics mode */
	exSetGraphics();

	/* create material */
	material = plMatCreate();
	material->ShadeType = PL_SHADE_FLAT;
	material->NumGradients = 1500;
	material->Texture = plReadPCXTex("eggbug.pcx", 1, 1);
	material->Diffuse[0] = material->Diffuse[1] = material->Diffuse[2] = 0;
	material->TexScaling = 1;
	material->Shininess = 0;
	plMatInit(material);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = 32;
	palette[1] = 32;
	palette[2] = 32;
	plMatMapToPal(material, palette, 0, 255);

	exSetPalette(palette);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Pitch = -45;
	camera->Pan = 45;
	camera->X = 256;
	camera->Y = 384;
	camera->Z = -256;

	/* seed random timer */
	srand(time(NULL));

	/* setup eggbugs */
	for (i = 0; i < NUM_EGGBUGS; i++)
	{
		eggbugs[i].model = plRead3DSObj("eggbug.3ds", material);
		eggbugs[i].model->Xa = 90;
		eggbugs[i].model->Ya = -90;
		eggbugs[i].model->Xp = rangerandom(-256, 256);
		eggbugs[i].model->Yp = rangerandom(-256, 256);
		eggbugs[i].model->Zp = rangerandom(-256, 256) + 512;
		eggbugs[i].speed = rangerandom(16, 64);
	}

	/* main loop */
	then = exClock();
	while (!exGetKey())
	{
		now = exClock();
		dt = (float)(now - then) / (float)exClockPerSecond();
		then = now;

		/* move eggbugs */
		for (i = 0; i < NUM_EGGBUGS; i++)
		{
			eggbugs[i].model->Zp -= eggbugs[i].speed * dt;

			/* reset if needed */
			if (eggbugs[i].model->Zp < -512)
			{
				eggbugs[i].model->Xp = rangerandom(-256, 256);
				eggbugs[i].model->Yp = rangerandom(-256, 256);
				eggbugs[i].model->Zp = rangerandom(-256, 256) + 512;
				eggbugs[i].speed = rangerandom(32, 128);
			}
		}

		/* clear back buffer */
		memset(zbuffer, 0, sizeof(zbuffer));
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);

		/* render eggbugs */
		for (i = 0; i < NUM_EGGBUGS; i++)
			plRenderObj(eggbugs[i].model);

		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	for (i = 0; i < NUM_EGGBUGS; i++)
		plObjDelete(eggbugs[i].model);
	plCamDelete(camera);
	plMatDelete(material);

	/* shut down video */
	exSetText();

	return 0;
}
