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

uint8_t *framebuffer;

#define NUM_ITERS 4
#define BOX_DIST 2.75
#define BOX_ROTFACTOR 2.6

void makeBoxes(pl_Obj *obj, float s,
               pl_Mat **m, int i);       // Makes hierarchy of cubes
void rotateBoxes(pl_Obj *obj, float r); // Rotates hierarchy

int main(int argc, char **argv) {
  int i, done = 0;

  pl_Mat *mat[NUM_ITERS]; // Materials
  pl_Obj *obj;             // Head object
  pl_Light *light;
  pl_Cam *cam;

  float ar;
  int c;
  uint8_t pal[768];

  printf("Flurry v1.0 -- Mode13 specific version\n"
         "Copyright (c) 1996, Justin Frankel\n");
  printf("Using:\n"
         "  %s\n",plVersionString);

  exSetGraphics(); // Set graphics

  framebuffer = (uint8_t *) plMalloc(W*H);
  ar = (W/(float) H) * (3.0/4.0); // Calc aspect ratio

  cam = plCamCreate(W,H,ar,90.0,framebuffer,NULL);
  cam->Sort = 1;
  cam->Z = -350;

  // Initialize materials
  memset(&mat,0,sizeof(mat));
  for (i = 0; i < NUM_ITERS; i ++) {
    mat[i] = plMatCreate();
    mat[i]->NumGradients = 200;
    mat[i]->Transparent = 2;
    mat[i]->Ambient[0] = mat[i]->Ambient[1] = mat[i]->Ambient[2] = 20;
    mat[i]->Shininess = 3;
    mat[i]->ShadeType = PL_SHADE_GOURAUD;
    //mat[i]->Priority = i;
  }
  mat[0]->Diffuse[0] = 190; mat[0]->Diffuse[1] = 190;  mat[0]->Diffuse[2] = 0;
  mat[0]->Specular[0] = 240; mat[0]->Specular[1] = 240; mat[0]->Specular[2] = 0;
  mat[1]->Diffuse[0] = 0; mat[1]->Diffuse[1] = 0;  mat[1]->Diffuse[2] = 100;
  mat[1]->Specular[0] = 0; mat[1]->Specular[1] = 0; mat[1]->Specular[2] = 100;
  mat[2]->Diffuse[0] = 0; mat[2]->Diffuse[1] = 130;  mat[2]->Diffuse[2] = 0;
  mat[2]->Specular[0] = 0; mat[2]->Specular[1] = 130; mat[2]->Specular[2] = 0;
  mat[3]->Diffuse[0] = 100; mat[3]->Diffuse[1] = 0;  mat[3]->Diffuse[2] = 0;
  mat[3]->Specular[0] = 100; mat[3]->Specular[1] = 0; mat[3]->Specular[2] = 0;

  for (i = 0; i < NUM_ITERS; i ++)
    plMatInit(mat[i]);

  memset(pal,0,768);
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

  while (!done) {
    rotateBoxes(obj,1.0);
    memset(framebuffer,0,W*H);
    plRenderBegin(cam);
    plRenderLight(light);
    plRenderObj(obj);
    plRenderEnd();
    exWaitVSync();                   // Sync with retrace
    memcpy(exGraphMem,framebuffer,W*H); // dump to screen
    while ((c = exGetKey())) switch(c) {
      case 27: done = 1; break;
      case '-': cam->Fov += 1.0; if (cam->Fov > 170) cam->Fov = 170; break;
      case '=': cam->Fov -= 1.0; if (cam->Fov < 10) cam->Fov = 10; break;
    }
  }
  plFree(framebuffer);
  plObjDelete(obj);
  plLightDelete(light);
  plCamDelete(cam);
  exSetText(); // Restore text mode
  return 0;
}

void makeBoxes(pl_Obj *obj, float s, pl_Mat **m, int i) {
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

void rotateBoxes(pl_Obj *obj, float r) {
  pl_Obj *child;
  int i;
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
