// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#if defined(PLUSH_EXAMPLE_SDL2)
#include "ex_sdl2.h"
#elif defined(PLUSH_EXAMPLE_SDL3)
#include "ex_sdl3.h"
#else
#error please define a plush example backend!
#endif
