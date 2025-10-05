// triangle.c: obligatory rainbow colored triangle
// owo

// note: not actually rainbow colored until i implement vertex colour support
// -- erysdren

#include "ex.h"

static pl_Obj *triangle;
static pl_Mat *material;
static pl_Cam *camera;
static uint8_t framebuffer[W * H];
static uint8_t palette[768];

int exInit(void **appstate, int argc, char **argv)
{
	/* create triangle model */
	triangle = plObjCreate(NULL);
	triangle->Model = plMdlCreate(3, 1);

	/* setup triangle model */
	triangle->Model->Vertices[0].x = 128;
	triangle->Model->Vertices[0].y = 0;
	triangle->Model->Vertices[0].z = 0;

	triangle->Model->Vertices[1].x = -128;
	triangle->Model->Vertices[1].y = 0;
	triangle->Model->Vertices[1].z = 0;

	triangle->Model->Vertices[2].x = 0;
	triangle->Model->Vertices[2].y = 192;
	triangle->Model->Vertices[2].z = 0;

	triangle->Model->Faces[0].Vertices[0] = &triangle->Model->Vertices[0];
	triangle->Model->Faces[0].Vertices[1] = &triangle->Model->Vertices[1];
	triangle->Model->Faces[0].Vertices[2] = &triangle->Model->Vertices[2];

	triangle->Model->Faces[0].vsLighting[0] = 0.125;
	triangle->Model->Faces[0].vsLighting[1] = 2;
	triangle->Model->Faces[0].vsLighting[2] = 0.5;

	plMdlCalcNormals(triangle->Model);

	/* create triangle material */
	material = plMatCreate();
	material->NumGradients = 100;
	material->ShadeType = PL_SHADE_GOURAUD;
	plMatInit(material);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = palette[1] = palette[2] = 0;

	/* map the material to the palette */
	plMatMapToPal(material, palette, 0, 255);

	/* set material on triangle */
	plMdlSetMat(triangle->Model, material);

	/* set palette on screen */
	exSetPalette(palette);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, NULL);
	camera->Y = 96;
	camera->Z = -300;

	return PL_EXIT_CONTINUE;
}

int exIterate(void *appstate)
{
	/* clear framebuffer */
	plMemSet(framebuffer, 0, sizeof(framebuffer));

	/* render frame */
	plRenderBegin(camera);
	plRenderObj(triangle);
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
	plObjDelete(triangle);
	plMatDelete(material);
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "triangle: obligatory rainbow colored triangle");
}
