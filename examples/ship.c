// ship.c: OBJ model loading example
// owo

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h>

#include "ex.h"

pl_Light *light;
pl_Obj *ship;
pl_Mat *materials[32];
pl_Cam *camera;
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];
size_t num_materials = 0;

int main(int argc, char **argv)
{
	/* setup graphics mode */
	exSetGraphics();

	/* load ship model */
	ship = plObjCreate(NULL);
	ship->Model = plReadWavefrontMdlEx("ship.obj", materials, 32, &num_materials, NULL);
	plMdlScale(ship->Model, 2);

	/* initialize materials */
	for (int i = 0; i < num_materials; i++)
	{
		materials[i]->ShadeType = PL_SHADE_GOURAUD;
		materials[i]->Ambient[0] = materials[i]->Ambient[1] = materials[i]->Ambient[2] = 0;
		plMatInit(materials[i]);
	}

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, materials, num_materials);
	palette[0] = palette[1] = palette[2] = 0;
	for (int i = 0; i < num_materials; i++)
		plMatMapToPal(materials[i], palette, 0, 255);

	exSetPalette(palette);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Z = -300;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	/* main loop */
	while (!exGetKey())
	{
		/* rotate model */
		ship->Xa += 1.0;
		ship->Ya += 1.0;
		ship->Za += 1.0;

		/* clear back buffer */
		memset(zbuffer, 0, sizeof(zbuffer));
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);
		plRenderLight(light);
		plRenderObj(ship);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plObjDelete(ship);
	for (int i = 0; i < num_materials; i++)
		plMatDelete(materials[i]);

	/* shut down video */
	exSetText();

	return 0;
}
