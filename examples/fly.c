#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h>

#include "ex.h"

/* Physical size of land */
#define LAND_SIZE 65000
/* Number of divisions of the land. Higher number == more polygons */
#define LAND_DIV 32

                  /* Mouse interface functions */
 // mouse sensitivity
float mouse_sens = 2048.0/32768.0;

                    /* Misc functions */
 // Sets up the materials
void setup_materials(pl_Mat **mat, uint8_t *pal);
 // Sets up the landscape and skies
pl_Obj *setup_landscape(pl_Mat *m, pl_Mat *sm, pl_Mat *sm2);

                       /* Main!!! */
int main(int argc, char **argv) {
  char lastmessage[80] = "Fly 3.0"; // last message used for status line
  int draw_sky = 1;                 // do we draw the sky?
  int wait_vsync = 1;               // do we wait for vsync?
  int frames, t;                    // for framerate counter
  int i;
  int c;

  uint8_t *framebuffer;            // our doublebuffer
  pl_Mat *mat[3+1];                 // our materials, we have 1 extra for null
                                    // termination for plMatMakeOptPal2()
  pl_Cam *cam;                      // our camera
  pl_Obj *land;                     // the land object
  pl_Obj *sky, *sky2;               // the two skies
  int done = 0;

  uint8_t pal[768];                    // our palette

  srand(0);                         // initialize rng

  printf("Plush 3D Fly v3.0.\n"
         "  %s\n"
         "  %s\n",plVersionString,plCopyrightString);
     // print out startup info

  printf("\n\nControls:\n"
         "  Mouse: rotate\n" 
         "  Mouse buttons: left=move forward, right=move forward fast\n"
         "  s: toggle sky (default on)\n"
         "  -,+: adjust fov (default 90)\n"
         "  [,]: adjust mouse sensitivity\n"
         "  v: toggle vsync (default off)\n\n");

  exSetGraphics();  // intialize graphics
  framebuffer = (uint8_t *) plMalloc(W*H); // allocate framebuffer
      // create camera
  cam = plCamCreate(W, // width
                    H, // height
                    W/H*3.0/4.0, // aspect ratio
                    90.0, // fov
                    framebuffer, // framebuffer (our doublebuffer)
                    0);  // zbuffer (not used)
  cam->Y = 800; // move the camera up from the ground

  setup_materials(mat,pal); // intialize materials and palette

  land = setup_landscape(mat[0],mat[1],mat[2]); // create landscape
  sky = plObjRemoveParent(land->Children->NextSibling); // unhierarchicalize the sky from the land
  sky2 = plObjRemoveParent(land->Children);

  frames = 0;     // set up for framerate counter
  t = exClock();
  while (!done) { // main loop
    // save time when the frame began, to be used later.
    float prevtime = exClock() / (float) exClockPerSecond();
    frames++; // update for framerate counter
  
    memset(framebuffer,1,W*H); // clear our doublebuffer

    // lots of rendering special casing
    if (draw_sky) { // if we're drawing the sky
      if (cam->Y > 2000) { // if above the sky, only render the skies, with 
                           // no far clip plane
        cam->ClipBack = 0.0;
        plRenderBegin(cam);
        plRenderObj(sky);
        plRenderObj(sky2);
      } else {           // otherwise, render the sky (but not the second sky),
                         // and the land, with a far clip plane
        cam->ClipBack = 10000.0;
        plRenderBegin(cam);
        plRenderObj(sky);
        plRenderObj(land);
      }
    } else { // not drawing sky, just render the land
      cam->ClipBack = 10000.0;
      plRenderBegin(cam);
      plRenderObj(land);
    }
    plRenderEnd(); // finish rendering

    // display framerate counter
    plTextPrintf(cam,cam->ClipLeft+5,cam->ClipTop,0.0,156,"%.2f fps",
        (frames/ (float) (exClock() - t)) * (float) exClockPerSecond());
    // display last message
    plTextPrintf(cam,cam->ClipLeft+5,cam->ClipBottom-16,0.0,156,lastmessage);
       

    if (wait_vsync) exWaitVSync(); // wait for vsync
      /* blit to screen. This is pretty darn fast on ip5's but on a 486 you
         would probably be faster doing a plain memcpy(), i.e:
         memcpy((void *) __djgpp_conventional_base+0xA0000,framebuffer,64000);
      */
    memcpy(exGraphMem,framebuffer,W*H);

    // We calculate the amount of time in thousanths of seconds this frame took
    prevtime = ((exClock() / (float) exClockPerSecond()) - prevtime)*1000.0;
    if (exMouseButtons & 2) { // if right button hit, we go forward quickly
      cam->X -=
        prevtime*4*sin(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Z += 
        prevtime*4*cos(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Y += 
        prevtime*4*sin(cam->Pitch*PL_PI/180.0);
    } else if (exMouseButtons & 1) { // if left button hit, we go forward slowly
      cam->X -= 
        prevtime*2*sin(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Z += 
        prevtime*2*cos(cam->Pan*PL_PI/180.0)*cos(cam->Pitch*PL_PI/180.0);
      cam->Y += 
        prevtime*2*sin(cam->Pitch*PL_PI/180.0);
    }
    cam->Pitch -= (exMouseDeltaY*mouse_sens); // update pitch and pan of ship
    cam->Pan -= (exMouseDeltaX*mouse_sens);
    
    if (cam->X > LAND_SIZE/2) cam->X = LAND_SIZE/2; // make sure we don't go 
    if (cam->X < -LAND_SIZE/2) cam->X = -LAND_SIZE/2; // too far away
    if (cam->Z > LAND_SIZE/2) cam->Z = LAND_SIZE/2;
    if (cam->Z < -LAND_SIZE/2) cam->Z = -LAND_SIZE/2;
    if (cam->Y < 0) cam->Y = 0;
    if (cam->Y > 8999) cam->Y = 8999;

    while ((c = exGetKey())) switch(c) { // handle keystrokes
      case 27: done++; break;    // ESC == quit
        // + is for zooming in.
      case '=': case '+': cam->Fov -= 1.0; if (cam->Fov < 1.0) cam->Fov = 1.0;
        sprintf(lastmessage,"FOV: %2.f",cam->Fov);
      break;
        // - is for zooming out
      case '-': cam->Fov += 1.0; if (cam->Fov > 179.0) cam->Fov = 179.0;
        sprintf(lastmessage,"FOV: %2.f",cam->Fov);
      break;
        // [ decreases mouse sensitivity
      case '[': mouse_sens /= 1.1; 
        sprintf(lastmessage,"MouseSens: %.3f",mouse_sens);
      break;
        // ] increases mouse sensitivity
      case ']': mouse_sens *= 1.1; 
        sprintf(lastmessage,"MouseSens: %.3f",mouse_sens);
      break;
        // v toggles vsync
      case 'v': wait_vsync ^= 1;
        sprintf(lastmessage,"VSync %s",wait_vsync ? "on" : "off");
      break;
        // s toggles sky
      case 's': draw_sky ^= 1;
        sprintf(lastmessage,"Sky %s",draw_sky ? "on" : "off");
      break;
    } 
  }
  // set text mode
  exSetText();
  // clean up
  plFree(framebuffer);
  plObjDelete(land);
  plObjDelete(sky);
  plObjDelete(sky2);
  plTexDelete(mat[0]->Texture);
  plTexDelete(mat[1]->Texture);
  plTexDelete(mat[2]->Texture);
  plMatDelete(mat[0]);
  plMatDelete(mat[1]);
  plMatDelete(mat[2]);
  plCamDelete(cam);

  printf("This has been a Plush demo app.\n"
         "Visit the Plush 3D homepage at: \n" 
         "  http://nullsoft.home.ml.org/plush/\n\n");

  return 0;
}

void setup_materials(pl_Mat **mat, uint8_t *pal) {
  int i;
  // create our 3 materials, make the fourth null so that plMatMakeOptPal2() 
  // knows where to stop
  mat[0] = plMatCreate();
  mat[1] = plMatCreate();
  mat[2] = plMatCreate();
  mat[3] = 0;

  pal[0] = pal[1] = pal[2] = 0; // make color 0 black.

  // set up material 0 (the ground)
  mat[0]->ShadeType = PL_SHADE_GOURAUD_DISTANCE;
  mat[0]->Shininess = 1;
  mat[0]->NumGradients = 2500;
  mat[0]->Ambient[0] = pal[0]*2 - 255; // these calculations are to get the
  mat[0]->Ambient[1] = pal[1]*2 - 255; // distance shading to work right
  mat[0]->Ambient[2] = pal[2]*2 - 255; 
  mat[0]->Diffuse[0] = 127-pal[0];
  mat[0]->Diffuse[1] = 127-pal[1];
  mat[0]->Diffuse[2] = 127-pal[2];
  mat[0]->Specular[0] = 127-pal[0];
  mat[0]->Specular[1] = 127-pal[1];
  mat[0]->Specular[2] = 127-pal[2];
  mat[0]->FadeDist = 10000.0;
  mat[0]->Texture = plReadPCXTex("ground.pcx",1,1);
  mat[0]->TexScaling = 40.0*LAND_SIZE/50000;
  mat[0]->PerspectiveCorrect = 16;

  // set up material 1 (the sky)
  mat[1]->ShadeType = PL_SHADE_GOURAUD_DISTANCE;
  mat[1]->Shininess = 1;
  mat[1]->NumGradients = 1500;
  mat[1]->Ambient[0] = pal[0]*2 - 255;
  mat[1]->Ambient[1] = pal[1]*2 - 255;
  mat[1]->Ambient[2] = pal[2]*2 - 255;
  mat[1]->Diffuse[0] = 127-pal[0];
  mat[1]->Diffuse[1] = 127-pal[1];
  mat[1]->Diffuse[2] = 127-pal[2];
  mat[1]->Specular[0] = 127-pal[0];
  mat[1]->Specular[1] = 127-pal[1];
  mat[1]->Specular[2] = 127-pal[2];
  mat[1]->FadeDist = 10000.0;
  mat[1]->Texture = plReadPCXTex("sky.pcx",1,1);
  mat[1]->TexScaling = 45.0*LAND_SIZE/50000;
  mat[1]->PerspectiveCorrect = 32;

  // set up material 2 (the second sky)
  mat[2]->ShadeType = PL_SHADE_NONE;
  mat[2]->Shininess = 1;
  mat[2]->NumGradients = 1500;
  mat[2]->Texture = plReadPCXTex("sky2.pcx",1,1);
  mat[2]->TexScaling = 10.0; //200.0*LAND_SIZE/50000;
  mat[2]->PerspectiveCorrect = 2;
    
  // intialize the materials
  plMatInit(mat[0]);
  plMatInit(mat[1]);
  plMatInit(mat[2]);

  // make a nice palette
  plMatMakeOptPal(pal,1,255,mat,3);
 
  // map the materials to this new palette
  plMatMapToPal(mat[0],pal,0,255);
  plMatMapToPal(mat[1],pal,0,255);
  plMatMapToPal(mat[2],pal,0,255);

  // set the new palette
  exSetPalette(pal);
}

pl_Obj *setup_landscape(pl_Mat *m, pl_Mat *sm, pl_Mat *sm2) {
  int i;
  // make our root object the land
  pl_Obj *sky;
  pl_Obj *o = plObjCreate(NULL);
  o->Model = plMakePlane(LAND_SIZE,LAND_SIZE,LAND_DIV-1,m);
  // give it a nice random bumpy effect
  for (i = 0; i < o->Model->NumVertices; i ++)
    o->Model->Vertices[i].y += (float) (rand()%1400)-700;
  // gotta recalculate normals for backface culling to work right
  plMdlCalcNormals(o->Model);

  // Make our first child the first sky
  sky = plObjCreate(o);
  sky->Model = plMakePlane(LAND_SIZE,LAND_SIZE,1,sm);
  sky->Yp = 2000;
  sky->BackfaceCull = 0;

  // and the second the second sky
  sky = plObjCreate(o);
  sky->Model = plMakeSphere(LAND_SIZE,10,10,sm2);
  sky->Yp = 2000;
  plMdlFlipNormals(sky->Model);

  return (o);
}
