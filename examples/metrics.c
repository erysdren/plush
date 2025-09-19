// teapot.c: OBJ model loading example
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
pl_Cam *camera;
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];

void drawFPS() {
	static int framecount = 0; /* replace with a global if you have one */
	static int old_framecount = 0;
	static int old_tick = 0;
	static uint8_t string[16] = "... FPS";

	if (exClock() - old_tick > exClockPerSecond()) {
		old_tick = exClock();
		int frames_drawn = framecount - old_framecount;
		old_framecount = framecount;
		snprintf(string, sizeof(string), "%d FPS", frames_drawn);
	}
	framecount++; /* remove if incremented globally */

	int xpos = camera->ClipLeft + 5;
	int ypos = camera->ClipTop + 5;
	int color = 50;
	plTextPrintf(camera, xpos, ypos, 0, color, string);
}

void drawFrameGraph() {
	static int framecount = 0; /* replace with a global if you have one */
	static int frametimes[100];
	static int time_n = sizeof(frametimes) / sizeof(int);
	static int old_tick = 0;

	float scale = 2.5; /* applied before clipping */
	int clip = 200; /* to prevent the graph from going too far right */

	int frametime = exClock() - old_tick;
	old_tick = exClock();
	frametimes[framecount % time_n] = (int)(frametime * scale) % 200;
	framecount++; /* remove if incremented globally */

	int xpos = camera->ClipLeft + 5;
	int ypos = camera->ClipTop + 25;
	int color = 50;
	float fade_speed = 2;
	for (int i = 0; i < time_n; i++) { /* right-to-left, bright-to-dark */
		int len = color * fade_speed < frametimes[i] ?
			color * fade_speed : frametimes[i];
		float cur_color = color;
		for (int j = 0; j < len; j++) {
			int xcoord = xpos + len - j;
			int ycoord = W * (ypos + i);
			framebuffer[xcoord + ycoord] = cur_color;
			cur_color -= fade_speed;
		}
	}
}

int main(int argc, char **argv)
{
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

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Z = -300;

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
		plRenderBegin(camera);
		plRenderLight(light);
		plRenderObj(teapot);
		plRenderEnd();

		drawFPS();
		drawFrameGraph();
		/*float dummy = 0; // time waster loop 
		for (int i = 1; i < 10000000; i++)
			dummy += sqrt(i);*/
		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plObjDelete(teapot);
	plMatDelete(material);

	/* shut down video */
	exSetText();

	return 0;
}
