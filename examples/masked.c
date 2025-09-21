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
#include "sky.h"

pl_Light *light;
pl_Obj *cube1;
pl_Obj *cube2;
pl_Obj *sky;
pl_Mat *materials[2];
pl_Cam *camera;
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];

static void main_loop(void)
{
	/* rotate cubes */
	cube1->Xa += 1.0;
	cube1->Ya += 1.0;
	cube1->Za += 1.0;

	cube2->Xa += 1.0;
	cube2->Ya -= 1.0;
	cube2->Za -= 1.0;

	/* clear framebuffer and zbuffer */
	memset(framebuffer, 0, sizeof(framebuffer));
	memset(zbuffer, 0, sizeof(zbuffer));

	/* render frame */
	plRenderBegin(camera);
	plRenderLight(light);
	plRenderObj(sky);
	plRenderObj(cube2);
	plRenderObj(cube1);
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
	materials[0]->ShadeType = PL_SHADE_GOURAUD;
	materials[0]->Texture = plTexCreate(64, 64, texture2_pixels, 256, texture2_palette);
	materials[0]->Diffuse[0] = materials[0]->Diffuse[1] = materials[0]->Diffuse[2] = 0;
	materials[0]->Texture->ClearColor = 255;
	materials[0]->PerspectiveCorrect = 2;
	plMatInit(materials[0]);

	/* create sky material */
	materials[1] = plMatCreate();
	materials[1]->Ambient[0] = materials[1]->Ambient[1] = materials[1]->Ambient[2] = 0;
	materials[1]->ShadeType = PL_SHADE_NONE;
	materials[1]->Shininess = 1;
	materials[1]->Texture = plTexCreate(96, 96, sky_pixels, 256, sky_palette);
	materials[1]->TexScaling = 20.0;
	materials[1]->PerspectiveCorrect = 2;
	plMatInit(materials[1]);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, materials, 2);
	palette[0] = palette[1] = palette[2] = 0;
	plMatMapToPal(materials[0], palette, 0, 255);
	plMatMapToPal(materials[1], palette, 0, 255);

	exSetPalette(palette);

	/* create cubes */
	cube1 = plObjCreate(NULL);
	cube1->Model = plMakeBox(128, 128, 128, materials[0]);

	cube2 = plObjCreate(NULL);
	cube2->Model = plMakeBox(64, 64, 64, materials[0]);
	cube2->Zp += 128;

	/* create sky */
	sky = plObjCreate(NULL);
	sky->Model = plMakeSphere(65000, 10, 10, materials[1]);
	plMdlFlipNormals(sky->Model);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Z = -384;

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
	plMdlDelete(cube1->Model);
	plMdlDelete(cube2->Model);
	plMdlDelete(sky->Model);
	plObjDelete(cube1);
	plObjDelete(cube2);
	plObjDelete(sky);
	plTexDelete(materials[0]->Texture);
	plTexDelete(materials[1]->Texture);
	plMatDelete(materials[0]);
	plMatDelete(materials[1]);

	/* shut down video */
	exSetText();

	return 0;
}
