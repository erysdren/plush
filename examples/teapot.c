// teapot.c: OBJ model loading example
// owo

#include "ex.h"

static pl_Light *light;
static pl_Obj *teapot;
static pl_Mat *material;
static pl_Cam *camera;
static uint8_t framebuffer[W * H];
static float zbuffer[W * H];
static uint8_t palette[768];

int exInit(void **appstate, int argc, char **argv)
{
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

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = palette[1] = palette[2] = 0;
	plMatMapToPal(material, palette, 0, 255);

	exSetPalette(palette);

	/* load fork model */
	teapot = plObjCreate(NULL);
	teapot->Model = plReadWavefrontMdl("teapot.obj", material);
	plMdlScale(teapot->Model, 32);
	plMdlTranslate(teapot->Model, 0, -32, 0);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Z = -300;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	return PL_EXIT_CONTINUE;
}

int exIterate(void *appstate)
{
	/* rotate model */
	teapot->Xa += 1.0;
	teapot->Ya += 1.0;
	teapot->Za += 1.0;

	/* clear back buffer */
	plMemSet(zbuffer, 0, sizeof(zbuffer));
	plMemSet(framebuffer, 0, sizeof(framebuffer));

	/* render frame */
	plRenderBegin(camera);
	plRenderLight(light);
	plRenderObj(teapot);
	plRenderEnd();

	/* copy to screen */
	plMemCpy(exGraphMem, framebuffer, sizeof(framebuffer));

	return PL_EXIT_CONTINUE;
}

int exKeyEvent(void *appstate, int key)
{
	// any keypress will trigger an exit
	return PL_EXIT_SUCCESS;
}

void exQuit(void *appstate, int code)
{
	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plObjDelete(teapot);
	plMatDelete(material);
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "teapot: OBJ model loading example");
}
