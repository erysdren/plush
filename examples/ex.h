// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <float.h>
#include <time.h>
#include <math.h>

#include <plush/plush.h>

/* return codes from ex* functions */
enum {
	PL_EXIT_CONTINUE = -1,
	PL_EXIT_SUCCESS = 0,
	PL_EXIT_FAILURE = 1
};

/* load your files and allocate your buffers here */
/* NOTE: will be called before a graphics context is ready */
/* return a value in *appstate and it will get passed in subsequent calls */
/* return the appropriate PL_EXIT_* code to exit or continue */
int exInit(void **appstate, int argc, char **argv);

/* called for each frame the program is running */
/* return the appropriate PL_EXIT_* code to exit or continue */
int exIterate(void *appstate);

/* called each time the user presses a key */
/* return the appropriate PL_EXIT_* code to exit or continue */
int exKeyEvent(void *appstate, int key);

/* called when the program is shutting down */
/* code will be whatever PL_EXIT_* value caused the program to exit */
void exQuit(void *appstate, int code);

#if defined(PLUSH_EXAMPLE_SDL2)
#include "ex_sdl2.h"
#elif defined(PLUSH_EXAMPLE_SDL3)
#include "ex_sdl3.h"
#elif defined(PLUSH_EXAMPLE_DOS)
#include "ex_dos.h"
#else
#error please define a plush example backend!
#endif
