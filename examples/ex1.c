// Ex1.c: simple Plush example
// Rotates a flat shaded cube

#include "ex.h" 

// Our variables
static pl_Light *TheLight; // Our light
static pl_Obj *TheCube; // Our cube object
static pl_Mat *CubeMat; // The material for the cube
static pl_Cam *TheCamera; // Our camera
static uint8_t TheFrameBuffer[W*H]; // Our framebuffer to render to
static uint8_t ThePalette[768];

int exInit(void **appstate, int argc, char **argv)
{
	CubeMat = plMatCreate(); // Create the material for the cube
	CubeMat->NumGradients = 100; // Have it use 100 colors
	CubeMat->ShadeType = PL_SHADE_FLAT; // Make the cube flat shaded
	plMatInit(CubeMat); // Initialize the material

	plMatMakeOptPal(ThePalette,1,255,&CubeMat,1); // Create a nice palette

	ThePalette[0] = ThePalette[1] = ThePalette[2] = 0; // Color 0 is black

	plMatMapToPal(CubeMat,ThePalette,0,255); // Map the material to our palette

	exSetPalette(ThePalette); // Set the palette

	TheCube = plObjCreate(NULL);
	TheCube->Model = plMakeBox(100.0,100.0,100.0,CubeMat); // Create the cube

	// Create the camera
	TheCamera = plCamCreate(W, // Screen width
							H, // Screen height
							W*3.0/(H*4.0), // Aspect ratio
							90.0, // Field of view
							TheFrameBuffer, // Framebuffer
							NULL); // ZBuffer (none)
	TheCamera->Z = -300; // Back the camera up from the origin

	TheLight = plLightSet(plLightCreate(), // Create a light to be set up
							PL_LIGHT_VECTOR, // vector light
							0.0,0.0,0.0, // rotation angles
							1.0, // intensity
							1.0); // falloff, not used for vector lights
	return PL_EXIT_CONTINUE;
}

int exIterate(void *appstate)
{
	TheCube->Xa += 1.0; // Rotate by 1 degree on each axis
	TheCube->Ya += 1.0;
	TheCube->Za += 1.0;
	memset(TheFrameBuffer,0,W*H); // clear framebuffer for next frame
	plRenderBegin(TheCamera); // Start rendering with the camera
	plRenderLight(TheLight); // Render our light
	plRenderObj(TheCube); // Render our object
	plRenderEnd(); // Finish rendering
	memcpy(exGraphMem,TheFrameBuffer,W*H); // dump to screen
	return PL_EXIT_CONTINUE;
}

int exKeyEvent(void *appstate, int key)
{
	// any keypress will trigger an exit
	return PL_EXIT_SUCCESS;
}

void exQuit(void *appstate, int code)
{
	plMatDelete(CubeMat);
	plMdlDelete(TheCube->Model);
	plObjDelete(TheCube);
	plCamDelete(TheCamera);
	plLightDelete(TheLight);
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "ex1: Simple Plush example");
}
