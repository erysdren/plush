// Ex3.c: simple Plush example
// Rotates a flat shaded cube AND a gouraud torus
// Uses z-buffering for smooth intersections
// Also rotates the lightsource around
// Added from ex2: frees up memory at the end (good to do :)

#include "ex.h" 

// Our variables
static pl_Light *TheLight; // Our light
static float TheLight_Xa, TheLight_Ya, TheLight_Za; // The rotation angles of our light
static pl_Obj *TheCube; // Our cube object
static pl_Obj *TheTorus; // Our torus object
static pl_Mat *CubeMat; // The material for the cube
static pl_Mat *TorusMat; // The material for the torus
static pl_Mat *AllMaterials[3]; // Used for creating palette
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

	TorusMat = plMatCreate(); // Create the material for the torus
	TorusMat->NumGradients = 100; // Have it use 100 colors
	TorusMat->ShadeType = PL_SHADE_GOURAUD; // Make the torus gouraud shaded
	TorusMat->Shininess = 10; // Make the torus a bit more shiny

	TorusMat->Ambient[0] = 0; // Set red ambient component
	TorusMat->Ambient[1] = 12; // Set green ambient component
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

	exSetPalette(ThePalette); // Set the palette

	TheCube = plObjCreate(NULL);
	TheTorus = plObjCreate(NULL);
	TheCube->Model = plMakeBox(100.0,100.0,100.0,CubeMat); // Create the cube
	TheTorus->Model = plMakeTorus(40.0,100.0,10,8,TorusMat); // Create the torus

	TheTorus->Xp = -70.0; // Shift the torus to the left a bit

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

	TheLight = plLightCreate(); // Create the light. Will be set up every frame

	return PL_EXIT_CONTINUE;
}

int exIterate(void *appstate)
{
	TheCube->Xa += 1.0; // Rotate cube by 1 degree on each axis
	TheCube->Ya += 1.0;
	TheCube->Za += 1.0;

	TheTorus->Xa += 1.9; // Rotate the torus
	TheTorus->Ya -= 1.0;
	TheTorus->Za += 0.3;

	TheLight_Za += 1.0; // Rotate the light
	TheLight_Xa = 50.0;

	plLightSet(TheLight,
				PL_LIGHT_VECTOR, // Set the newly rotated light
				TheLight_Xa, TheLight_Ya, TheLight_Za, // angles
				1.0, // intensity
				1.0); // falloff, not used for vector lights

	// clear zbuffer for next frame
	plMemSet(TheZBuffer,0,W*H*sizeof(float));
	plMemSet(TheFrameBuffer,0,W*H*sizeof(uint8_t)); // clear framebuffer for next frame
	plRenderBegin(TheCamera); // Start rendering with the camera
	plRenderLight(TheLight); // Render our light
	plRenderObj(TheCube); // Render our cube
	plRenderObj(TheTorus); // Render our torus
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
	plFree(TheFrameBuffer); // Free up memory
	plFree(TheZBuffer);
	plCamDelete(TheCamera);
	plLightDelete(TheLight);
	plObjDelete(TheCube);
	plObjDelete(TheTorus);
	plMatDelete(CubeMat);
	plMatDelete(TorusMat);
}

int main(int argc, char **argv)
{
	return exBegin(argc, argv, "ex3: Simple Plush example");
}
