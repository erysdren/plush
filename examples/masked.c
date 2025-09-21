// masked.c: example of using ClearColor on a texture
// owo

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "ex.h" 

#include "texture2.h"

pl_Light *light;
pl_Obj *model;
pl_Obj *sky;
pl_Mat *materials[2];
pl_Cam *camera;
uint8_t framebuffer[W * H];
uint8_t palette[768];

static void main_loop(void)
{
	/* rotate model */
	model->Xa += 1.0;
	model->Ya += 1.0;
	model->Za += 1.0;

	/* clear back buffer */
	memset(framebuffer, 0, W * H);

	/* render frame */
	plRenderBegin(camera);
	plRenderLight(light);
	plRenderObj(sky);
	plRenderObj(model);
	plRenderEnd();

	/* wait for vsync, then copy to screen */
	exWaitVSync();
	memcpy(exGraphMem, framebuffer, W * H);
}

int main(int argc, char **argv)
{
	/* setup graphics mode */
	exSetGraphics();

	/* create masked material */
	materials[0] = plMatCreate();
	materials[0]->NumGradients = 256;
	materials[0]->ShadeType = PL_SHADE_FLAT;
	materials[0]->Texture = plTexCreate(64, 64, texture2_pixels, 256, texture2_palette);
	materials[0]->Diffuse[0] = materials[0]->Diffuse[1] = materials[0]->Diffuse[2] = 0;
	materials[0]->Texture->ClearColor = 255;
	plMatInit(materials[0]);

	/* create sky material */
	materials[1] = plMatCreate();
	materials[1]->Ambient[0] = materials[1]->Ambient[1] = materials[1]->Ambient[2] = 0;
	materials[1]->ShadeType = PL_SHADE_NONE;
	materials[1]->Shininess = 1;
	materials[1]->Texture = plReadPCXTex("sky.pcx", true, true);
	materials[1]->TexScaling = 20.0;
	materials[1]->PerspectiveCorrect = 2;
	plMatInit(materials[1]);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, materials, 2);
	palette[0] = palette[1] = palette[2] = 0;
	plMatMapToPal(materials[0], palette, 0, 255);
	plMatMapToPal(materials[1], palette, 0, 255);

	exSetPalette(palette);

	/* create cube */
	model = plObjCreate(NULL);
	model->Model = plMakeBox(100, 100, 100, materials[0]);

	/* create sky */
	sky = plObjCreate(NULL);
	sky->Model = plMakeSphere(65000, 10, 10, materials[1]);
	plMdlFlipNormals(sky->Model);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, NULL);
	camera->Z = -300;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	/* main loop */
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, true);
#else
	while (!exGetKey())
		main_loop();
#endif

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plMdlDelete(model->Model);
	plMdlDelete(sky->Model);
	plObjDelete(model);
	plObjDelete(sky);
	plTexDelete(materials[0]->Texture);
	plTexDelete(materials[1]->Texture);
	plMatDelete(materials[0]);
	plMatDelete(materials[1]);

	/* shut down video */
	exSetText();

	return 0;
}
