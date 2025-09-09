// triangle.c: obligatory rainbow colored triangle
// owo

// note: not actually rainbow colored until i implement vertex colour support
// -- erysdren

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h>

#include "ex.h"

pl_Obj *triangle;
pl_Mat *material;
pl_Cam *camera;
uint8_t framebuffer[W * H];
uint8_t palette[768];

int main(int argc, char **argv)
{
	int i;

	/* setup graphics mode */
	exSetGraphics();

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

	/* main loop */
	while (!exGetKey())
	{
		/* clear framebuffer */
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);
		plRenderObj(triangle);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	plCamDelete(camera);
	plObjDelete(triangle);
	plMatDelete(material);

	/* shut down video */
	exSetText();

	return 0;
}
