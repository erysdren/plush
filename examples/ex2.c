// Ex2.c: simple Plush example
// Rotates a flat shaded cube
// The cube is now a different colored cube then ex1.c
// ZBuffering has been added, as well as dynamic framebuffer allocation

#include "ex.h" 

// Our variables
static pl_Light *TheLight; // Our light
static pl_Obj *TheCube; // Our cube object
static pl_Mat *CubeMat; // The material for the cube
static pl_Mat *AllMaterials[2]; // Used for creating palette
static pl_Cam *TheCamera; // Our camera
static uint8_t *TheFrameBuffer; // Our framebuffer to render to
static float *TheZBuffer; // Our zbuffer
static uint8_t ThePalette[768];

int exInit(void **appstate, int argc, char **argv)
{
	TheFrameBuffer = (uint8_t *)plMalloc(W*H*sizeof(uint8_t)); // Alloc framebuffer
	if (!TheFrameBuffer)
		return PL_EXIT_FAILURE;

	// Alloc z-buffer
	TheZBuffer = (float *)plMalloc(W*H*sizeof(float));
	if (!TheZBuffer)
		return PL_EXIT_FAILURE;

	CubeMat = plMatCreate(); // Create the material for the cube
	CubeMat->NumGradients = 100; // Have it use 100 colors
	CubeMat->ShadeType = PL_SHADE_FLAT; // Make the cube flat shaded

	CubeMat->Ambient[0] = 32; // Set red ambient component
	CubeMat->Ambient[1] = 0; // Set green ambient component
	CubeMat->Ambient[2] = 16; // Set blue ambient component

	CubeMat->Diffuse[0] = 200; // Set red diffuse component
	CubeMat->Diffuse[1] = 100; // Set green diffuse component
	CubeMat->Diffuse[2] = 150; // Set blue diffuse component

	plMatInit(CubeMat); // Initialize the material

	AllMaterials[0] = CubeMat; // Make list of materials
	AllMaterials[1] = 0; // Null terminate list of materials
	plMatMakeOptPal(ThePalette,1,255,AllMaterials,1); // Create a nice palette

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
							TheZBuffer); // ZBuffer
	TheCamera->Z = -300; // Back the camera up from the origin
	TheCamera->Sort = 0; // We don't need to sort since zbuffering takes care
						// of it for us!

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

	plMemSet(TheZBuffer,0,W*H*sizeof(float)); // clear zbuffer for next frame
	plMemSet(TheFrameBuffer,0,W*H*sizeof(uint8_t)); // clear framebuffer for next frame
	plRenderBegin(TheCamera); // Start rendering with the camera
	plRenderLight(TheLight); // Render our light
	plRenderObj(TheCube); // Render our object
	plRenderEnd(); // Finish rendering
	plMemCpy(exGraphMem,TheFrameBuffer,W*H); // dump to screen
	return PL_EXIT_CONTINUE;
}

int exKeyEvent(void *appstate, int key)
{
	// any keypress will trigger an exit
	return PL_EXIT_SUCCESS;
}

void exQuit(void *appstate, int code)
{
	plFree(TheFrameBuffer);
	plFree(TheZBuffer);
	plMatDelete(CubeMat);
	plObjDelete(TheCube);
	plCamDelete(TheCamera);
	plLightDelete(TheLight);
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "ex2: Simple Plush example");
}
