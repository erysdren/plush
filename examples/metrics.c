/* CyanBun96 - a few basic performance metric displays.
 * Designed to be easily portable - just copy-paste the ones you need and adjust
 * the variables at the top!
 * The camera must be a global for the ones that print text.
 * Framebuffer must be global for the frame graph.
 *
 * drawFPS (line 34) - simple FPS counter, updates once per second.
 * drawFrameGraph (line 55) - frame time graph
 * drawFrameMinMax5pLow (line 88) - frame time stats, including shortest time to
 *                                 render, longest time to render, and 5% lows
 * drawTriStatsAvg (line 119) - the average number of triangles at different
 *                                 points in the rendering process
 * drawJitter (line 156) - frame jitter, less means smoother rendering with less
 *                         stutters
 * Insert calls right before the frame is copied to the screen (here line 248)
 */
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

void drawFPS() { /* CyanBun96 */
	int text_xpos = camera->ClipLeft + 5;
	int text_ypos = camera->ClipTop + 5;
	int text_color = 50;
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

	plTextPrintf(camera, text_xpos, text_ypos, 0, text_color, string);
}

void drawFrameGraph() { /* CyanBun96 */
	int xpos = camera->ClipLeft + 5;
	int ypos = camera->ClipTop + 25;
	int color = 50; /* top color, fades down */
	float fade_speed = 1.5; /* color fade speed, set to 0 for solid */
	float scale = 2.5; /* applied before clipping */
	int clip = 200; /* to prevent the graph from going too far right */
	static int frametimes[100]; /* less = shorter graph */
	static int framecount = 0; /* replace with a global if you have one */

	static int time_n = sizeof(frametimes) / sizeof(int);
	static int old_tick = 0;
	int frametime = exClock() - old_tick;
	old_tick = exClock();
	frametimes[framecount % time_n] = (int)(frametime * scale) % 200;
	framecount++; /* remove if incremented globally */

	for (int i = 0; i < time_n; i++) { /* right-to-left, bright-to-dark */
		int len;
		if (fade_speed > 0 && (color * fade_speed) < frametimes[i])
			len = color * fade_speed;
		else 
			len = frametimes[i];
		float cur_color = color;
		for (int j = 0; j < len; j++) {
			int xcoord = xpos + len - j;
			int ycoord = W * (ypos + i);
			framebuffer[xcoord + ycoord] = (int)cur_color;
			cur_color -= fade_speed;
		}
	}
}

static int int_comp(const void *a, const void *b) { /* required for qsort */
	return (*(int *)a - *(int *)b);
}

void drawFrameMinMax5pLow() { /* CyanBun96 */
	int text_xpos = camera->ClipLeft + 5;
	int text_ypos = camera->ClipTop + 135;
	int text_color = 50;
	static int framecount = 0; /* replace with a global if you have one */
	static int frametimes[100]; /* less = more frequent updates */

	static int time_n = sizeof(frametimes) / sizeof(int);
	static int old_tick = 0;
	static uint8_t string[64] = "Min: ...\nMax: ...\n5pLow: ...";

	frametimes[framecount % time_n] = exClock() - old_tick;
	old_tick = exClock();
	if (framecount % time_n == 0) {
		qsort(frametimes, time_n, sizeof(int), int_comp);
		int fivePcLow = 0;
		for (int i = 0; i < time_n * 0.05; i++)
			fivePcLow += frametimes[time_n - 1 - i];
		fivePcLow /= time_n * 0.05;
		snprintf(string, sizeof(string), "Min: %d\nMax: %d\n5pLow: %d",
			frametimes[0], frametimes[time_n - 1], fivePcLow);
	}
	framecount++; /* remove if incremented globally */

	plTextPrintf(camera, text_xpos, text_ypos, 0, text_color, string);
}

void drawTriStatsAvg() { /* CyanBun96 */
	int text_xpos = camera->ClipLeft + 5;
	int text_ypos = camera->ClipTop + 195;
	int text_color = 50;
	static int framecount = 0; /* replace with a global if you have one */
	static int tristats[100 * 4]; /* less = more frequent updates */
	/* tristats size must be a multiple of 4 */

	static int stat_n = sizeof(tristats) / sizeof(int) / 4;
	static int old_tick = 0;
	static uint8_t string[128]="Tris: ...\nCull: ...\nClip: ...\nTssl: ...";

	tristats[framecount % stat_n] = plRender_TriStats[0];
	tristats[framecount % stat_n + stat_n] = plRender_TriStats[1];
	tristats[framecount % stat_n + stat_n * 2] = plRender_TriStats[2];
	tristats[framecount % stat_n + stat_n * 3] = plRender_TriStats[3];
	if (framecount % stat_n == 0) {
		int tris, cull, clip, tssl = 0;
		for (int i = 0; i < stat_n; i++) {
			tris += tristats[i];
			cull += tristats[i + stat_n];
			clip += tristats[i + stat_n * 2];
			tssl += tristats[i + stat_n * 3];
		}
		tris /= stat_n;
		cull /= stat_n;
		clip /= stat_n;
		tssl /= stat_n;
		snprintf(string, sizeof(string),
			"Tris: %d\nCull: %d\nClip: %d\nTssl: %d",
			tris, cull, clip, tssl);
	}
	framecount++; /* remove if incremented globally */

	plTextPrintf(camera, text_xpos, text_ypos, 0, text_color, string);
}

void drawJitter() { /* CyanBun96 */
	int text_xpos = camera->ClipLeft + 5;
	int text_ypos = camera->ClipTop + 275;
	int text_color = 50;
	static int framecount = 0; /* replace with a global if you have one */
	static int frametimes[100]; /* less = more frequent updates */

	static int time_n = sizeof(frametimes) / sizeof(int);
	static int old_tick = 0;
	static uint8_t string[64] = "Jitter: ...";

	frametimes[framecount % time_n] = exClock() - old_tick;
	old_tick = exClock();

	if (framecount % time_n == 0) {
		int sum = 0;
		for (int i = 0; i < time_n; i++)
			sum += frametimes[i];
		float avg = (float)sum / time_n;
		float var = 0.0f;
		for (int i = 0; i < time_n; i++) {
			float d = frametimes[i] - avg;
			var += d * d;
		}
		var /= time_n;
		float stdev = sqrtf(var);
		snprintf((char*)string, sizeof(string), "Jitter: %.2f", stdev);
	}
	framecount++; /* remove if incremented globally */

	plTextPrintf(camera, text_xpos, text_ypos, 0, text_color, string);
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
		drawFrameMinMax5pLow();
		drawTriStatsAvg();
		drawJitter();
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
