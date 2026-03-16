// logo.c: plush logo
// owo

#include "ex.h"

static pl_Light *light;
static pl_Obj *logo;
static pl_Mat *material;
static pl_Cam *camera;
static uint8_t framebuffer[W * H];
static float zbuffer[W * H];
static uint8_t palette[768];

int exInit(void **appstate, int argc, char **argv)
{
	/* create material */
	material = plMatCreate();
	material->NumGradients = 2500;
	material->Texture = plReadPCXTex("logo.pcx", true, true);
	material->ShadeType = PL_SHADE_GOURAUD;
	material->Ambient[0] = material->Ambient[1] = material->Ambient[2] = -128;
	material->Diffuse[0] = material->Diffuse[1] = material->Diffuse[2] = 0;
	plMatInit(material);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = palette[1] = palette[2] = 0;
	plMatMapToPal(material, palette, 0, 255);

	exSetPalette(palette);

	/* load logo model */
	logo = plObjCreate(NULL);
	logo->Model = plReadWavefrontMdl("logo.obj", material);
	logo->Ya = 30;

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Z = -160;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	return PL_EXIT_CONTINUE;
}

int exIterate(void *appstate)
{
	/* clear back buffer */
	plMemSet(zbuffer, 0, sizeof(zbuffer));
	plMemSet(framebuffer, 0, sizeof(framebuffer));

	/* render frame */
	plRenderBegin(camera);
	plRenderLight(light);
	plRenderObj(logo);
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
	plObjDelete(logo);
	plMatDelete(material);
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "Plush Logo");
}
