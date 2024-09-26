// model.c: eggbug model example
// <3

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush.h> 

#include "ex.h" 

pl_Obj *model;
pl_Mat *material;
pl_Cam *camera;
uint8_t framebuffer[W * H];
pl_ZBuffer zbuffer[W * H];
uint8_t palette[768];

int main(int argc, char **argv)
{
	/* setup graphics mode */
	exSetGraphics();

	/* create material */
	material = plMatCreate();
	material->ShadeType = PL_SHADE_FLAT;
	material->NumGradients = 1500;
	material->Texture = plReadPCXTex("eggbug.pcx", 1, 1);
	material->Diffuse[0] = material->Diffuse[1] = material->Diffuse[2] = 0;
	material->TexScaling = 1;
	material->Shininess = 0;
	plMatInit(material);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = 0;
	palette[1] = 128;
	palette[2] = 128;
	plMatMapToPal(material, palette, 0, 255);

	exSetPalette(palette);

	/* load eggbug model */
	model = plRead3DSObj("eggbug.3ds", material);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Y = 8;
	camera->Z = -128;

	/* main loop */
	while (!exGetKey())
	{
		/* rotate model */
		model->Xa = 90;
		model->Ya += 1.5;

		/* clear back buffer */
		memset(zbuffer, 0, sizeof(zbuffer));
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);
		plRenderObj(model);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	plCamDelete(camera);
	plObjDelete(model);
	plMatDelete(material);

	/* shut down video */
	exSetText();

	return 0;
}
