// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#include <io.h>
#include <i86.h>
#include <dos.h>
#include <graph.h>
#include <fcntl.h>
#include <conio.h>
#include <time.h>

#define W (320)
#define H (200)

union REGS exRegs;
uint8_t *exGraphMem;
int exMouseButtons = 0;
int exMouseX = 0;
int exMouseY = 0;
int exMouseDeltaX = 0;
int exMouseDeltaY = 0;
int exHasMouse = 0;

int exClockPerSecond(void)
{
#ifdef __DJGPP__
	return UCLOCKS_PER_SEC;
#else
	return CLOCKS_PER_SEC;
#endif
}

int exClock(void)
{
#ifdef __DJGPP__
	return (int)uclock();
#else
	return (int)clock();
#endif
}

int exGetKey(void)
{
	// like the sdl backend, we abuse this function to also read the mouse
	if (!exHasMouse)
	{
		exRegs.x.eax = 0;
		int386(0x33, &exRegs, &exRegs);
		exHasMouse = 1;
	}

	exRegs.x.eax = 3;
	int386(0x33, &exRegs, &exRegs);

	exMouseDeltaX = exRegs.x.ecx - exMouseX;
	exMouseDeltaY = exRegs.x.edx - exMouseY;

	exMouseX = exRegs.x.ecx;
	exMouseY = exRegs.x.edx;

	exMouseButtons = exRegs.x.ebx;

	return kbhit() ? getch() : 0;
}

void exWaitVSync(void)
{
	while (!(inp(0x3DA) & 8));
	while ((inp(0x3DA) & 8));
}

void exSetPalette(uint8_t palette[768])
{
	int i;
	outp(0x3c8,0);
	for(i = 0; i < 256; i++)
	{
		outp(0x3c9, palette[i * 3 + 0] >> 2);
		outp(0x3c9, palette[i * 3 + 1] >> 2);
		outp(0x3c9, palette[i * 3 + 2] >> 2);
	}
}

void exSetGraphics(void)
{
#ifdef __DJGPP__
	__djgpp_nearptr_enable();
#endif
	exRegs.x.eax = 0x13;
	int386(0x10, &exRegs, &exRegs);
#ifdef __DJGPP__
	exGraphMem = (uint8_t *)__djgpp_conventional_base + 0xA0000;
#else
	exGraphMem = (uint8_t *)0xA0000;
#endif
}

void exSetText(void)
{
#ifdef __DJGPP__
	__djgpp_nearptr_disable();
#endif
	exRegs.x.eax = 0x03;
	int386(0x10, &exRegs, &exRegs);
}
