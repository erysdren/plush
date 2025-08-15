// ex.h: provides a standard interface for video and keyboard for the
// example programs to use.

#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <conio.h>
#include <time.h>

#ifdef __DJGPP__
#include <sys/nearptr.h>
#define EXREGS_EAX exRegs.x.ax
#define EXREGS_EBX exRegs.x.bx
#define EXREGS_ECX exRegs.x.cx
#define EXREGS_EDX exRegs.x.dx
#else
#define EXREGS_EAX exRegs.x.eax
#define EXREGS_EBX exRegs.x.ebx
#define EXREGS_ECX exRegs.x.ecx
#define EXREGS_EDX exRegs.x.edx
#endif

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
		EXREGS_EAX = 0;
		int386(0x33, &exRegs, &exRegs);
		exHasMouse = 1;
	}

	EXREGS_EAX = 3;
	int386(0x33, &exRegs, &exRegs);

	exMouseDeltaX = EXREGS_ECX - exMouseX;
	exMouseDeltaY = EXREGS_EDX - exMouseY;

	exMouseX = EXREGS_ECX;
	exMouseY = EXREGS_EDX;

	exMouseButtons = EXREGS_EBX;

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
	EXREGS_EAX = 0x13;
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
	EXREGS_EAX = 0x03;
	int386(0x10, &exRegs, &exRegs);
}
