// Ex3.c: simple Plush example
// Rotates a flat shaded cube AND a gouraud torus
// Uses z-buffering for smooth intersections
// Also rotates the lightsource around
// Added from ex2: frees up memory at the end (good to do :)

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

// Include the plush header file
#include <plush.h> 

// Include our example graphics interface module
#include "ex.h" 

// Our variables
pl_Light *TheLight;   // Our light
pl_Float TheLight_Xa, TheLight_Ya, TheLight_Za; 
                     // The rotation angles of our light
pl_Obj *TheCube;      // Our cube object
pl_Obj *TheTorus;     // Our torus object
pl_Mat *CubeMat;      // The material for the cube
pl_Mat *TorusMat;     // The material for the torus
pl_Mat *AllMaterials[3]; // Used for creating palette
pl_Cam *TheCamera; // Our camera
char *TheFrameBuffer; // Our framebuffer to render to
pl_ZBuffer *TheZBuffer;   // Our zbuffer
char ThePalette[768];

int main() { // Main
  int i;
#if defined(DJGPP) || defined(__WATCOMC__)
   // Put the fpu in a low precision, no exception state
  _control87(MCW_EM|PC_24,MCW_EM|MCW_PC); 
#endif
  exSetGraphics(); // Set graphics
 
  TheFrameBuffer = (char *) malloc(320*200); // Alloc framebuffer
  if (!TheFrameBuffer) { 
    exSetText(); 
    printf("Out of memory!\n");
    exit(1);
  }
  // Alloc z-buffer
  TheZBuffer = (pl_ZBuffer *) malloc(320*200*sizeof(pl_ZBuffer));

  CubeMat = plMatCreate();    // Create the material for the cube
  CubeMat->NumGradients = 100; // Have it use 100 colors
  CubeMat->ShadeType = PL_SHADE_FLAT; // Make the cube flat shaded

  CubeMat->Ambient[0] = 32; // Set red ambient component
  CubeMat->Ambient[1] = 0;  // Set green ambient component
  CubeMat->Ambient[2] = 16; // Set blue ambient component

  CubeMat->Diffuse[0] = 200; // Set red diffuse component
  CubeMat->Diffuse[1] = 100; // Set green diffuse component
  CubeMat->Diffuse[2] = 150; // Set blue diffuse component

  plMatInit(CubeMat);          // Initialize the material

  TorusMat = plMatCreate();    // Create the material for the torus
  TorusMat->NumGradients = 100; // Have it use 100 colors
  TorusMat->ShadeType = PL_SHADE_GOURAUD; // Make the torus gouraud shaded
  TorusMat->Shininess = 10; // Make the torus a bit more shiny

  TorusMat->Ambient[0] = 0; // Set red ambient component
  TorusMat->Ambient[1] = 12;  // Set green ambient component
  TorusMat->Ambient[2] = 4; // Set blue ambient component

  TorusMat->Diffuse[0] = 20; // Set red diffuse component
  TorusMat->Diffuse[1] = 60; // Set green diffuse component
  TorusMat->Diffuse[2] = 70; // Set blue diffuse component

  TorusMat->Specular[0] = 100; // Set red specular component
  TorusMat->Specular[1] = 200; // Set green specular component
  TorusMat->Specular[2] = 150; // Set blue specular component

  AllMaterials[0] = CubeMat; // Make list of materials
  AllMaterials[1] = TorusMat; // Make list of materials
  AllMaterials[2] = 0; // Null terminate list of materials
  plMatMakeOptPal(ThePalette,1,255,AllMaterials,2); // Create a nice palette

  ThePalette[0] = ThePalette[1] = ThePalette[2] = 0; // Color 0 is black

  plMatMapToPal(CubeMat,ThePalette,0,255); // Map the material to our palette
  plMatMapToPal(TorusMat,ThePalette,0,255); // Map the material to our palette

  // Convert std 8 bit/chan palette to vga's 6 bit/chan palette
  for (i = 0; i < 768; i ++) ThePalette[i] >>= 2;
  exSetPalette(ThePalette); // Set the palette
 
  TheCube = plMakeBox(100.0,100.0,100.0,CubeMat); // Create the cube
  TheTorus = plMakeTorus(40.0,100.0,10,8,TorusMat); // Create the torus

  TheTorus->Xp = -70.0; // Shift the torus to the left a bit

  TheCamera = plCamCreate(320, // Screen width
                          200, // Screen height
                          320*3.0/(200*4.0), // Aspect ratio
                          90.0, // Field of view
                          TheFrameBuffer, // Framebuffer
                          TheZBuffer // ZBuffer
                          ); // Create the camera
  TheCamera->Z = -300; // Back the camera up from the origin
  TheCamera->Sort = 0; // We don't need to sort since zbuffering takes care
                       // of it for us!

  TheLight = plLightCreate(); // Create the light. Will be set up every frame
             
  while (!exGetKey()) { // While the keyboard hasn't been touched
    TheCube->Xa += 1.0; // Rotate cube by 1 degree on each axis
    TheCube->Ya += 1.0;
    TheCube->Za += 1.0;

    TheTorus->Xa += 1.9;  // Rotate the torus
    TheTorus->Ya -= 1.0;
    TheTorus->Za += 0.3;

    TheLight_Za += 1.0; // Rotate the light
    TheLight_Xa = 50.0;

    plLightSet(TheLight, PL_LIGHT_VECTOR, // Set the newly rotated light
               TheLight_Xa, TheLight_Ya, TheLight_Za, // angles
               1.0, // intensity
               1.0); // falloff, not used for vector lights

                                      // clear zbuffer for next frame
    memset(TheZBuffer,0,320*200*sizeof(pl_ZBuffer));
    memset(TheFrameBuffer,0,320*200); // clear framebuffer for next frame
    plRenderBegin(TheCamera);        // Start rendering with the camera
    plRenderLight(TheLight);         // Render our light
    plRenderObj(TheCube);            // Render our cube
    plRenderObj(TheTorus);           // Render our torus
    plRenderEnd();                   // Finish rendering
    exWaitVSync();                   // Sync with retrace
    memcpy(exGraphMem,TheFrameBuffer,320*200); // dump to screen
  }
  free(TheFrameBuffer); // Free up memory
  free(TheZBuffer);
  plCamDelete(TheCamera);
  plLightDelete(TheLight);
  plObjDelete(TheCube);
  plObjDelete(TheTorus);
  plMatDelete(CubeMat);
  plMatDelete(TorusMat);
  exSetText(); // Restore text mode
  return 0;          // Quit
}
