// multicam.c: multiple cameras using the same framebuffer example
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
pl_Cam *cameras[4];
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];

int main(int argc, char **argv)
{
	int i;

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

	/* create cameras */
	for (i = 0; i < 4; i++)
	{
		cameras[i] = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
		cameras[i]->Z = -300;

		switch (i)
		{
			case 0:
				cameras[i]->CenterX = W / 4;
				cameras[i]->CenterY = H / 4;
				cameras[i]->ClipTop = 0;
				cameras[i]->ClipLeft = 0;
				cameras[i]->ClipBottom = H / 2;
				cameras[i]->ClipRight = W / 2;
				break;

			case 1:
				cameras[i]->CenterX = (W / 4) * 3;
				cameras[i]->CenterY = H / 4;
				cameras[i]->ClipTop = 0;
				cameras[i]->ClipLeft = W / 2;
				cameras[i]->ClipBottom = H / 2;
				cameras[i]->ClipRight = W;
				break;

			case 2:
				cameras[i]->CenterX = W / 4;
				cameras[i]->CenterY = (H / 4) * 3;
				cameras[i]->ClipTop = H / 2;
				cameras[i]->ClipLeft = 0;
				cameras[i]->ClipBottom = H;
				cameras[i]->ClipRight = W / 2;
				break;

			case 3:
				cameras[i]->CenterX = (W / 4) * 3;
				cameras[i]->CenterY = (H / 4) * 3;
				cameras[i]->ClipTop = H / 2;
				cameras[i]->ClipLeft = W / 2;
				cameras[i]->ClipBottom = H;
				cameras[i]->ClipRight = W;
				break;
		}
	}

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	/* main loop */
	while (!exGetKey())
	{
		/* rotate model */
		teapot->Xa += 1.0;
		teapot->Ya += 1.0;
		teapot->Za += 1.0;

		/* clear back buffer */
		memset(zbuffer, 0, sizeof(zbuffer));
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		for (i = 0; i < 4; i++)
		{
			plRenderBegin(cameras[i]);
			plRenderLight(light);
			plRenderObj(teapot);
			plRenderEnd();
		}

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	for (i = 0; i < 4; i++)
		plCamDelete(cameras[i]);
	plLightDelete(light);
	plObjDelete(teapot);
	plMatDelete(material);

	/* shut down video */
	exSetText();

	return 0;
}
