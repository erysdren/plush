// texture.c: custom texture example
// owo

#include "ex.h" 

#include "texture1.h"

static pl_Light *light;
static pl_Obj *model;
static pl_Mat *material;
static pl_Cam *camera;
static uint8_t framebuffer[W * H];
static uint8_t palette[768];

int exInit(void **appstate, int argc, char **argv)
{
	/* create material */
	material = plMatCreate();
	material->NumGradients = 256;
	material->ShadeType = PL_SHADE_FLAT;
	material->Texture = plTexCreate(64, 64, texture1_pixels, 256, texture1_palette);
	material->Diffuse[0] = material->Diffuse[1] = material->Diffuse[2] = 0;
	plMatInit(material);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = palette[1] = palette[2] = 0;
	plMatMapToPal(material, palette, 0, 255);

	exSetPalette(palette);

	/* create cube */
	model = plObjCreate(NULL);
	model->Model = plMakeBox(100, 100, 100, material);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, NULL);
	camera->Z = -300;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	return PL_EXIT_CONTINUE;
}

void exQuit(void *appstate, int code)
{
	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plObjDelete(model);
	plMatDelete(material);
}

int exKeyEvent(void *appstate, int key)
{
	// any keypress will trigger an exit
	return PL_EXIT_SUCCESS;
}

int exIterate(void *appstate)
{
	/* rotate model */
	model->Xa += 1.0;
	model->Ya += 1.0;
	model->Za += 1.0;

	/* clear back buffer */
	plMemSet(framebuffer, 0, W * H);

	/* render frame */
	plRenderBegin(camera);
	plRenderLight(light);
	plRenderObj(model);
	plRenderEnd();

	/* copy to screen */
	plMemCpy(exGraphMem, framebuffer, W * H);

	return PL_EXIT_CONTINUE;
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "texture: custom texture example");
}
