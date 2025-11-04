/*
** Flurry: a Plush demo program.
** Copyright (c) 1997, Justin Frankel
** For use with djgpp v2.x
** For more information on Plush, see http://nullsoft.home.ml.org/plush/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <plush/plush.h>

#include "ex.h"

#define NUM_ITERS 4
#define BOX_DIST 2.75
#define BOX_ROTFACTOR 2.6

static pl_Mat *mat[NUM_ITERS]; // Materials
static pl_Obj *obj; // Head object
static pl_Light *light;
static pl_Cam *cam;
static uint8_t *framebuffer;
static uint8_t pal[768];

static void makeBoxes(pl_Obj *obj, float s, pl_Mat **m, int i); // Makes hierarchy of cubes
static void rotateBoxes(pl_Obj *obj, float r); // Rotates hierarchy

int exInit(void **appstate, int argc, char **argv)
{
	int i;
	float ar;

	printf("Flurry v1.0 -- Mode13 specific version\n"
			"Copyright (c) 1996, Justin Frankel\n");
	printf("Using:\n"
			"  %s\n",plVersionString);

	framebuffer = (uint8_t *)plMalloc(W*H);
	ar = (W/(float) H) * (3.0/4.0); // Calc aspect ratio

	cam = plCamCreate(W,H,ar,90.0,framebuffer,NULL);
	cam->Sort = 1;
	cam->Z = -350;

	// Initialize materials
	plMemSet(&mat,0,sizeof(mat));
	for (i = 0; i < NUM_ITERS; i ++)
	{
		mat[i] = plMatCreate();
		mat[i]->NumGradients = 200;
		mat[i]->Transparent = 2;
		mat[i]->Ambient[0] = mat[i]->Ambient[1] = mat[i]->Ambient[2] = 20;
		mat[i]->Shininess = 3;
		mat[i]->ShadeType = PL_SHADE_GOURAUD;
	}
	mat[0]->Diffuse[0] = 190; mat[0]->Diffuse[1] = 190; mat[0]->Diffuse[2] = 0;
	mat[0]->Specular[0] = 240; mat[0]->Specular[1] = 240; mat[0]->Specular[2] = 0;
	mat[1]->Diffuse[0] = 0; mat[1]->Diffuse[1] = 0; mat[1]->Diffuse[2] = 100;
	mat[1]->Specular[0] = 0; mat[1]->Specular[1] = 0; mat[1]->Specular[2] = 100;
	mat[2]->Diffuse[0] = 0; mat[2]->Diffuse[1] = 130; mat[2]->Diffuse[2] = 0;
	mat[2]->Specular[0] = 0; mat[2]->Specular[1] = 130; mat[2]->Specular[2] = 0;
	mat[3]->Diffuse[0] = 100; mat[3]->Diffuse[1] = 0; mat[3]->Diffuse[2] = 0;
	mat[3]->Specular[0] = 100; mat[3]->Specular[1] = 0; mat[3]->Specular[2] = 0;

	for (i = 0; i < NUM_ITERS; i ++)
		plMatInit(mat[i]);

	plMemSet(pal,0,768);
	plMatMakeOptPal(pal,1,255,mat,NUM_ITERS); // Create a nice palette
	pal[0] = pal[1] = pal[2] = 0; // Color 0 is black
	for (i = 0; i < NUM_ITERS; i ++)
		plMatMapToPal(mat[i],pal,0,255); // Map the material to our palette
	exSetPalette(pal); // Set the new palette via mode 13 function

	// Make objects
	obj = plObjCreate(NULL);
	obj->Model = plMakeBox(100,100,100,mat[0]);
	makeBoxes(obj,100.0,mat+1,NUM_ITERS-1);

	// Setup light
	light = plLightCreate();
	plLightSet(light,PL_LIGHT_VECTOR,0,0,0,1.0,1.0);

	return PL_EXIT_CONTINUE;
}

int exIterate(void *appstate)
{
	rotateBoxes(obj,1.0);
	plMemSet(framebuffer,0,W*H);
	plRenderBegin(cam);
	plRenderLight(light);
	plRenderObj(obj);
	plRenderEnd();
	plMemCpy(exGraphMem,framebuffer,W*H); // dump to screen
	return PL_EXIT_CONTINUE;
}

int exKeyEvent(void *appstate, int key)
{
	switch (key)
	{
		case 27: return PL_EXIT_SUCCESS;
		case '-': cam->Fov += 1.0; if (cam->Fov > 170) cam->Fov = 170; break;
		case '=': cam->Fov -= 1.0; if (cam->Fov < 10) cam->Fov = 10; break;
	}

	return PL_EXIT_CONTINUE;
}
void exQuit(void *appstate, int code)
{
	int i;
	for (i = 0; i < NUM_ITERS; i++)
		plMatDelete(mat[i]);
	plFree(framebuffer);
	plObjDelete(obj);
	plLightDelete(light);
	plCamDelete(cam);
}

static void makeBoxes(pl_Obj *obj, float s, pl_Mat **m, int i)
{
	pl_Obj *child;
	if (!i) return;
	child = plObjCreate(obj);
	child->Model = plMakeBox(s/2,s/2,s/2,*m);
	child->Xp = s*BOX_DIST;;
	child = plObjCreate(obj);
	child->Model = plMakeBox(s/2,s/2,s/2,*m);
	child->Xp = -s*BOX_DIST;
	child = plObjCreate(obj);
	child->Model = plMakeBox(s/2,s/2,s/2,*m);
	child->Yp = s*BOX_DIST;
	child = plObjCreate(obj);
	child->Model = plMakeBox(s/2,s/2,s/2,*m);
	child->Yp = -s*BOX_DIST;
	child = plObjCreate(obj);
	child->Model = plMakeBox(s/2,s/2,s/2,*m);
	child->Zp = s*BOX_DIST;
	child = plObjCreate(obj);
	child->Model = plMakeBox(s/2,s/2,s/2,*m);
	child->Zp = -s*BOX_DIST;

	child = obj->Children;
	while (child)
	{
		makeBoxes(child,s/2,m+1,i-1);
		child = child->NextSibling;
	}
}

static void rotateBoxes(pl_Obj *obj, float r)
{
	pl_Obj *child;
	if (!obj) return;
	obj->Ya += r;
	obj->Xa += r;

	child = obj->Children;
	while (child)
	{
		rotateBoxes(child,r*BOX_ROTFACTOR);
		child = child->NextSibling;
	}
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "Flurry: a Plush demo program.");
}
