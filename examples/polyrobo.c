// polyrobo.c: big fancy robot example
// owo

// TODO: make this load polyrobo.mdl directly!

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h>

#include "ex.h"

pl_Light *light;
pl_Obj *polyrobo;
pl_Mat *materials[7];
pl_Cam *camera;
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];
size_t num_materials = 0;

int main(int argc, char **argv)
{
	size_t i, n;

	/* setup graphics mode */
	exSetGraphics();

	/* fallback material */
	materials[0] = plMatCreate();

	/* load polyrobo model */
	polyrobo = plObjCreate(NULL);
	polyrobo->Model = plReadWavefrontMdlEx("polyrobo.obj", &materials[1], 6, &n, materials[0]);

	/* override material setups */
	materials[1]->Ambient[0] = materials[1]->Ambient[1] = materials[1]->Ambient[2] = -128;
	materials[1]->Specular[0] = materials[1]->Specular[1] = materials[1]->Specular[2] = 64;
	materials[1]->Diffuse[0] = materials[1]->Diffuse[1] = materials[1]->Diffuse[2] = 128;
	materials[1]->NumGradients = 2500;
	materials[1]->ShadeType = PL_SHADE_GOURAUD;
	materials[1]->Texture = plReadPCXTex("black.pcx", true, true);
	materials[1]->PerspectiveCorrect = 8;

	materials[2]->Ambient[0] = materials[2]->Ambient[1] = materials[2]->Ambient[2] = -128;
	materials[2]->Specular[0] = materials[2]->Specular[1] = materials[2]->Specular[2] = 64;
	materials[2]->Diffuse[0] = materials[2]->Diffuse[1] = materials[2]->Diffuse[2] = 128;
	materials[2]->NumGradients = 2500;
	materials[2]->ShadeType = PL_SHADE_GOURAUD;
	materials[2]->Texture = plReadPCXTex("black.pcx", true, true);
	materials[2]->Environment = plReadPCXTex("chrome_grayr.pcx", true, true);
	materials[2]->TexEnvMode = PL_TEXENV_MAX;

	materials[3]->Ambient[0] = materials[3]->Ambient[3] = materials[3]->Ambient[2] = -128;
	materials[3]->Specular[0] = materials[3]->Specular[3] = materials[3]->Specular[2] = 64;
	materials[3]->Diffuse[0] = materials[3]->Diffuse[3] = materials[3]->Diffuse[2] = 128;
	materials[3]->NumGradients = 2500;
	materials[3]->ShadeType = PL_SHADE_GOURAUD;
	materials[3]->Texture = plReadPCXTex("black.pcx", true, true);
	materials[3]->Environment = plReadPCXTex("chrome_steel1.pcx", true, true);
	materials[3]->TexEnvMode = PL_TEXENV_MAX;

	materials[4]->Ambient[0] = materials[4]->Ambient[1] = materials[4]->Ambient[2] = -128;
	materials[4]->Specular[0] = materials[4]->Specular[1] = materials[4]->Specular[2] = 64;
	materials[4]->Diffuse[0] = materials[4]->Diffuse[1] = materials[4]->Diffuse[2] = 128;
	materials[4]->NumGradients = 2500;
	materials[4]->ShadeType = PL_SHADE_GOURAUD;
	materials[4]->Texture = plReadPCXTex("droid_legmap.pcx", true, true);
	materials[4]->PerspectiveCorrect = 8;

	materials[5]->Ambient[0] = materials[5]->Ambient[1] = materials[5]->Ambient[2] = -128;
	materials[5]->Specular[0] = materials[5]->Specular[1] = materials[5]->Specular[2] = 64;
	materials[5]->Diffuse[0] = materials[5]->Diffuse[1] = materials[5]->Diffuse[2] = 128;
	materials[5]->NumGradients = 2500;
	materials[5]->ShadeType = PL_SHADE_GOURAUD;
	materials[5]->Texture = plReadPCXTex("ears.pcx", true, true);
	materials[5]->PerspectiveCorrect = 8;

	materials[6]->Ambient[0] = materials[6]->Ambient[1] = materials[6]->Ambient[2] = 0;
	materials[6]->Specular[0] = materials[6]->Specular[1] = materials[6]->Specular[2] = 64;
	materials[6]->Diffuse[0] = materials[6]->Diffuse[1] = materials[6]->Diffuse[2] = 128;
	materials[6]->NumGradients = 2500;
	materials[6]->ShadeType = PL_SHADE_NONE;
	materials[6]->Texture = plReadPCXTex("eye.pcx", true, true);
	materials[6]->PerspectiveCorrect = 8;

	/* initialize materials */
	num_materials = n + 1;
	for (i = 0; i < num_materials; i++)
		plMatInit(materials[i]);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, materials, num_materials);
	palette[0] = palette[1] = palette[2] = 0;
	for (i = 0; i < num_materials; i++)
		plMatMapToPal(materials[i], palette, 0, 255);

	exSetPalette(palette);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Y += 96;
	camera->Z -= 512;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	/* main loop */
	while (!exGetKey())
	{
		/* rotate model */
		polyrobo->Ya += 1.0;

		/* clear back buffer */
		memset(zbuffer, 0, sizeof(zbuffer));
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);
		plRenderLight(light);
		plRenderObj(polyrobo);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plMdlDelete(polyrobo->Model);
	plObjDelete(polyrobo);
	for (i = 0; i < num_materials; i++)
	{
		if (materials[i]->Texture) plTexDelete(materials[i]->Texture);
		if (materials[i]->Environment) plTexDelete(materials[i]->Environment);
		plMatDelete(materials[i]);
	}

	/* shut down video */
	exSetText();

	return 0;
}
