// model.c: 3DS model loading example
// owo

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h> 

#include "ex.h" 

pl_Light *light;
pl_Obj *model;
pl_Mat *material;
pl_Cam *camera;
uint8_t framebuffer[W * H];
uint8_t palette[768];

int main(int argc, char **argv)
{
	/* setup graphics mode */
	exSetGraphics();

	/* create material */
	material = plMatCreate();
	material->NumGradients = 100;
	material->ShadeType = PL_SHADE_FLAT;
	plMatInit(material);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = palette[1] = palette[2] = 0;
	plMatMapToPal(material, palette, 0, 255);

	exSetPalette(palette);

	/* load fork model */
	model = plRead3DSObj("fork.3ds", material);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, NULL);
	camera->Z = -300;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	/* main loop */
	while (!exGetKey())
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
		plRenderObj(model);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, W * H);
	}

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plObjDelete(model);
	plMatDelete(material);

	/* shut down video */
	exSetText();

	return 0;
}
