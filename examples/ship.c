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
pl_Obj *city;
pl_Obj *sky;
pl_Mat *materials[64];
pl_Cam *camera;
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];
size_t num_materials;
size_t num_materials1 = 0;
size_t num_materials2 = 0;

int main(int argc, char **argv)
{
	/* setup graphics mode */
	exSetGraphics();

	/* fallback material */
	materials[0] = plMatCreate();
	materials[0]->Ambient[0] = 0.1745 * 255;
	materials[0]->Ambient[1] = 0.03175 * 255;
	materials[0]->Ambient[2] = 0.03175 * 255;
	materials[0]->Diffuse[0] = 0.61424 * 255;
	materials[0]->Diffuse[1] = 0.10136 * 255;
	materials[0]->Diffuse[2] = 0.10136 * 255;
	materials[0]->Specular[0] = 0.727811 * 255;
	materials[0]->Specular[1] = 0.626959 * 255;
	materials[0]->Specular[2] = 0.626959 * 255;

	/* sky material */
	materials[1] = plMatCreate();
	materials[1]->ShadeType = PL_SHADE_NONE;
	materials[1]->Shininess = 1;
	materials[1]->NumGradients = 1500;
	materials[1]->Texture = plReadPCXTex("sky2.pcx", true, true);
	materials[1]->TexScaling = 20.0;
	materials[1]->PerspectiveCorrect = 2;

	/* load city model */
	city = plObjCreate(NULL);
	city->Model = plReadWavefrontMdlEx("citybbk.obj", &materials[2], 64 - 2, &num_materials1, materials[0]);

	/* create sky model */
	sky = plObjCreate(city);
	sky->Model = plMakeSphere(65000, 10, 10, materials[1]);
	plMdlFlipNormals(sky->Model);

	/* load ship model */
	ship = plObjCreate(NULL);
	ship->Model = plReadWavefrontMdlEx("ship.obj", &materials[num_materials1 + 2], 64 - num_materials1 - 2, &num_materials2, materials[0]);
	ship->Yp = 300;
	ship->Zp = 200;

	/* initialize materials */
	num_materials = 2 + num_materials1 + num_materials2;
	plMatInit(materials[0]);
	plMatInit(materials[1]);
	for (int i = 2; i < num_materials; i++)
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
	camera->X = 10;
	camera->Y = 330;
	camera->Pitch = -10;
	camera->Pan = 10;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 40.0, 30.0, 10.0, 1.0, 1.0);

	/* main loop */
	while (!exGetKey())
	{
		/* rotate model */
		ship->Xa += 1.0;
		ship->Ya += 1.0;
		ship->Za += 1.0;

		city->Ya += 0.1;

		/* clear back buffer */
		memset(zbuffer, 0, sizeof(zbuffer));
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);
		plRenderLight(light);
		plRenderObj(sky);
		plRenderObj(city);
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
	plObjDelete(city);
	for (int i = 0; i < num_materials; i++)
		plMatDelete(materials[i]);

	/* shut down video */
	exSetText();

	return 0;
}
