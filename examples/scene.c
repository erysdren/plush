// scene.c: prerendered framebuffer+zbuffer example
// owo

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h>

#include "ex.h"

pl_Light *light;
pl_Obj *teapot;
pl_Mat *material;
pl_Cam *camera;
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t stored_framebuffer[W * H];
float stored_zbuffer[W * H];
uint8_t palette[768];

#if (W == 640) && (H == 480)
#define STORED_FRAMEBUFFER "scene640.col"
#define STORED_ZBUFFER "scene640.dph"
#define STORED_PALETTE "scene640.pal"
#elif (W == 320) && (H == 200)
#define STORED_FRAMEBUFFER "scene320.col"
#define STORED_ZBUFFER "scene320.dph"
#define STORED_PALETTE "scene320.pal"
#else
#error no stored buffers for provided resolution
#endif

int main(int argc, char **argv)
{
	FILE *file;
	int i;

	/* load prerendered assets */
	file = fopen(STORED_FRAMEBUFFER, "rb");
	fread(stored_framebuffer, sizeof(stored_framebuffer), 1, file);
	fclose(file);

	file = fopen(STORED_ZBUFFER, "rb");
	fread(stored_zbuffer, sizeof(stored_zbuffer), 1, file);
	fclose(file);

	file = fopen(STORED_PALETTE, "rb");
	fread(palette, sizeof(palette), 1, file);
	fclose(file);

	/* invert and scale zbuffer */
	for (i = 0; i < sizeof(stored_zbuffer) / 4; i++)
	{
		stored_zbuffer[i] = -8.0f * stored_zbuffer[i];
		stored_zbuffer[i] += 0.1f;
	}

	/* setup graphics mode */
	exSetGraphics();

	/* create material */
	/* these ambient/diffuse/specular values are taken from Haiku's GLTeapot */
	material = plMatCreate();
	material->NumGradients = 50;
	material->ShadeType = PL_SHADE_GOURAUD;
	material->Ambient[0] = 0.1745 * 255;
	material->Ambient[1] = 0.03175 * 255;
	material->Ambient[2] = 0.03175 * 255;
	material->Diffuse[0] = 0.61424 * 255;
	material->Diffuse[1] = 0.10136 * 255;
	material->Diffuse[2] = 0.10136 * 255;
	material->Specular[0] = 0.727811 * 255;
	material->Specular[1] = 0.626959 * 255;
	material->Specular[2] = 0.626959 * 255;
	plMatInit(material);

	plMatMapToPal(material, palette, 0, 255);

	exSetPalette(palette);

	/* load fork model */
	teapot = plObjCreate(NULL);
	teapot->Model = plReadWavefrontMdl("teapot.obj", material);
	teapot->Xp = 1;
	teapot->Yp = 4;
	teapot->Zp = 1;

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->X = -8;
	camera->Y = 4;
	camera->Z = -8;
	camera->Pan = -45;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	/* main loop */
	while (!exGetKey())
	{
		/* rotate model */
		teapot->Xa += 1.0;
		teapot->Ya += 1.0;
		teapot->Za += 1.0;

		/* reset zbuffer and framebuffer */
		memcpy(zbuffer, stored_zbuffer, sizeof(zbuffer));
		memcpy(framebuffer, stored_framebuffer, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);
		plRenderLight(light);
		plRenderObj(teapot);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plObjDelete(teapot);
	plMatDelete(material);

	/* shut down video */
	exSetText();

	return 0;
}
