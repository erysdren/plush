/*
** DUCKDEMO: a Plush demo program.
** Copyright (c) 1997, Justin Frankel
** For use with djgpp v2.x and Allegro
** For more information on Plush, see http://nullsoft.home.ml.org/plush/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <plush/plush.h>

#include "ex.h"

pl_Mat *Material1, *Material2;   // Materials for the duck
pl_Cam *Camera;                // Camera
pl_Obj *Object;              // The duck
pl_Light *Light;              // Light source

uint8_t framebuffer[W*H]; // Our framebuffer to render to

void SetUpColors();

int main(int argc, char **argv) {
  float *zbuffer;             // Our zbuffer

  printf("%s\n%s\n\n",plVersionString,plCopyrightString); // I like to do this

  exSetGraphics(); // Set graphics

  Material1 = plMatCreate();
  Material2 = plMatCreate();

  SetUpColors(); // Init palette

  Object = plRead3DSObj("duckdemo.3ds",Material1);
  if (!Object) {
    perror("Can't load duckdemo.3ds");
    return 1;
  }

     // First child is an eye, child of child is eye
  plMdlSetMat(Object->Children->Model,Material2);
  plMdlSetMat(Object->Children->Children->Model,Material2);
     // Child of eye is other eye, make it child of duck
  plObjAddChild(Object, plObjRemoveParent(Object->Children->Children));

  plMdlScale(Object->Model,0.1); // Scale object down...

  Object->BackfaceCull = 0;    // We want to be able to see through the duck
  Object->BackfaceIllumination = 1;

  Light = plLightCreate();        // Create a lightsource
  plLightSet(Light,PL_LIGHT_VECTOR,0,0,0,1.0,1.0); // Vector light, 1.0 intensity

  if ((argc > 1 && !strcasecmp(argv[1],"-nozb")) ||
      (argc > 2 && !strcasecmp(argv[2],"-nozb"))) zbuffer = 0;
  else 
    zbuffer = (float *) plMalloc(sizeof(float)*W*H);

  Camera = plCamCreate(W,H, // Create camera
                       W*3.0/(H*4.0), // Aspect ratio (usually 1.0)
                       80.0, framebuffer, zbuffer);
  Camera->Z = -500;   // move the camera back a bit
  if (zbuffer) Camera->Sort = 0; // Sorting not necessary w/ zbuffer
  else Camera->Sort = 1;

  while (!exGetKey()) {
    Object->Xa += 2.0;  // Rotate object
    Object->Ya += 2.0;
    Object->Za += 2.0;

    memset(framebuffer, 0, sizeof(framebuffer)); // clear framebuffer & zbuffer
    if (Camera->zBuffer) memset(Camera->zBuffer,0,sizeof(float)*
                                Camera->ScreenWidth*Camera->ScreenHeight);

    plRenderBegin(Camera); // Render to camera
    plRenderLight(Light);  // Render light
    plRenderObj(Object); // Render duck
    plRenderEnd(); // Finish rendering
    exWaitVSync();                   // Sync with retrace
    memcpy(exGraphMem,framebuffer,sizeof(framebuffer)); // dump to screen
  }
  plObjDelete(Object); // Free duck
  plLightDelete(Light);   // Free light
  plCamDelete(Camera); // Free camera

  plFree(zbuffer);
  exSetText(); // Restore text mode
  //printf("Try \"duckdemo 640x480\" or \"duckdemo 320x200 -nozb\" etc\n");
  return 0;
}

void SetUpColors() {
  uint8_t pal[768]; // Our rgb triplet palette
  pl_Mat *AllMaterials[2];
  memset(pal,0,768);

  //Material1->Priority = 0;  // setup material 1
  Material1->NumGradients = 200;
  Material1->Diffuse[0] = 203;
  Material1->Diffuse[1] = 212;
  Material1->Diffuse[2] = 0;
  Material1->Specular[0] = 128;
  Material1->Specular[1] = 56;
  Material1->Specular[2] = 0;
  Material1->Shininess = 15;
  Material1->Ambient[0] = Material1->Ambient[1] = Material1->Ambient[2] = 0;
  Material1->Transparent = 0;
  Material1->Environment = NULL;
  Material1->Texture = NULL; // Could do plReadTexturePCX() here for texture...
  Material1->ShadeType = PL_SHADE_GOURAUD;
  plMatInit(Material1);

  //Material2->Priority = 1;  // setup material 2
  Material2->NumGradients = 100;
  Material2->Diffuse[0] = 0;
  Material2->Diffuse[1] = 0;
  Material2->Diffuse[2] = 0;
  Material2->Specular[0] = 160;
  Material2->Specular[1] = 130;
  Material2->Specular[2] = 0;
  Material2->Shininess = 5;
  Material2->Ambient[0] = Material2->Ambient[1] = Material2->Ambient[2] = 0;
  Material2->Transparent = 0;
  Material2->Environment = NULL;
  Material2->Texture = NULL;
  Material2->ShadeType = PL_SHADE_GOURAUD;
  plMatInit(Material2);

  AllMaterials[0] = Material1;
  AllMaterials[1] = Material2;
  plMatMakeOptPal(pal,1,255,AllMaterials,2); // Create a nice palette
  pal[0] = pal[1] = pal[2] = 0; // Color 0 is black
  plMatMapToPal(Material1,pal,0,255); // Map the material to our palette
  plMatMapToPal(Material2,pal,0,255); // Map the material to our palette

  exSetPalette(pal); // Set the palette
}
