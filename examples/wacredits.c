/* The credits screen from WinAmp 5.666, with minor adjustments
 * Ported by CyanBun96 <3 */

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <plush/plush.h>
#include "ex.h"

#define LAND_SIZE 500
#define LAND_DIV 12
#define OBJ_NUM 8
#define FIRE_BITMAP_W 64
#define FIRE_BITMAP_H 64

pl_Mat *mat[7];
pl_Cam *cam;
pl_Obj *land, *lightobject, *billboard, *teamobject;
pl_Obj *object[OBJ_NUM]; /* the nested spinning tori */
pl_Spline spline;
float splineTime;
pl_Light *light;
unsigned int prevtime;
pl_Texture *grndtex, *watex;
float keys1[8 * 5];
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t pal[768];
int rpoo = 1;
bool fuckomode = 1; /* swaps wall and fire textures, 1 by default */
float speed = 0.7;

void makeBallTexture(char *tx)
{
	int x, y;
	unsigned char *p = (unsigned char *)tx;
	unsigned char *t = p + FIRE_BITMAP_W * FIRE_BITMAP_H;
	for (x = 0; x < FIRE_BITMAP_W; x++) {
		int a = *t - 10;
		if ((rand() & 0x7) == 7)
			a += 130;
		if (a < 0)
			a = 0;
		else if (a > 150)
			a = 150;
		*t++ = a;
	}
	for (y = 0; y < FIRE_BITMAP_H; y++) {
		*p++ = p[0]/4 + p[FIRE_BITMAP_W]/2 + p[FIRE_BITMAP_W + 1]/4;
		for (x = 1; x < FIRE_BITMAP_W - 1; x++)
			*p++ = p[0]/4 + p[FIRE_BITMAP_W]/4 +
				p[FIRE_BITMAP_W-1]/4 + p[FIRE_BITMAP_W + 1]/4;
		*p++ = p[0]/4 + p[FIRE_BITMAP_W]/2 + p[FIRE_BITMAP_W - 1]/4;
	}
}

void makeBallTextPal(char *pal)
{
	unsigned char *t = (unsigned char *)pal;
	int x = 255;
	t[0] = t[1] = t[2] = 0;
	t += 3;
	while (x) {
		if (x > 128) {
			int a = 256 - x;
			a *= 3;
			if (a > 255)
				a = 255;
			t[2] = 0;
			t[1] = a / 2;
			t[0] = a;
		} else {
			t[2] = 256 - x * 2;
			t[1] = 255 / 3 + ((256 - x) * 2) / 3;
			t[0] = 255;
		}
		t += 3;
		x--;
	}
}

pl_Texture *mkWATex()
{
	pl_Texture *p = (pl_Texture *) malloc(sizeof(pl_Texture));
	if (p) {
		p->Data = malloc(64 * 65 + 2);
		p->PaletteData = malloc(3 * 256);
		if (p->Data && p->PaletteData) {
			makeBallTextPal(p->PaletteData);
			p->Width = 6;
			p->Height = 6;
			p->iWidth = 64;
			p->iHeight = 64;
			p->uScale = 128;
			p->vScale = 64;
			p->NumColors = 150;
			p->ClearColor = -1;
		} else {
			printf("mkWATex failed\n");
			exit(1);
		}
	}
	return p;
}

uint8_t bk0[64 * 64] =
    {
"WWWWWWWWWWWWWWWWOOWWWWWWWwwwwoWwwWWWWWWWWWWWWooOWWWWWWWWWWWWWWOoWWWOwWWWWWWWWWWWWwooWWWWWWWwOooOoWWWWWWWWWWWoowWWWWWWWWWWWWWWWWwWWWwwwWWWWWWWWWWWWWwOWWWWoOWWWWWoWoOoWWWWWWWOWWWWWWWWWWWWWWWWWWOWWWOwWWWWWWWWWWWWWOOWWWWWwowWWWWWWwWowWWWWWWowWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWwooWWWOOwWWWWWWWWWoWOWWWOWWWWWWWWWWWWWWWWWWwWWWWOOWWWWWWWWWWWWWWWWWOWWWwWWWWWWWWWWowwoWWowWWWWWWWWWWWWWWWWWOWWWwooWWWWWWWWWWWWWWWWOooWWwoWWWWWWWWWWOWoWWoWWWWWWWWWWWWWWWWWWOWWWwOOWWWWWWWWWWWWWWWWWWooWWWwoOWWWWWWWWWoWooWWWWWWWWWWWWWWWWWWOWWWwOWWWWWWWWWWWWWWWWWWWWwOWoOowWWWWWWWWWOWOoWWWWWWWWWWWWWWWWWWOWWWwwOWWWWWWWWWWWWWWWWWWwWooooWWWWWWWWWWWWOOOWWWWWWWWWWWWWWWWWWOWWWwOOWWWWWWWWWWWWWWWWWoOOOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWWWwOWWWWWWWWWWWWWWWWWWOwoOOwooooOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWWWwOWWWWWWWWWWWWWWWWOWOOwOOwOWOWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWWWwOWWWWWWWWWWWWWWWWWWWOwwwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWoWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWWwwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWWOwoWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOOWWWOoWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwowWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWoWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWOOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWOWWWWWWWwowWWWWwooWWWWWWOWWOooooowoowOWWWWWWWWWWWWWWOWwooWWWoWWowWWWWWWWwOWWWWWWOwWWWWWOOWwOwWWoWWWWWwwWWWWWWWWWWWOwwwwOWWWWWowOOWWWWWWwowWWWWWWwwOWWWWOwWowOWWWwOOOwOWWWWWWWWWWWOowwwwwwOWWWOWWWWWWWWwwWOWWWWWWOoOWWWWWWOOWWWOowOwOOOWWWWWWWWWWWwWwwwwwoOWWWWwOWWWWOwoWWWWWWWWWWwwOWWWWoowWWWOowwwOwOWWWWWWWWWWWwWwwwwwoOWWWwooWWWWOOWWwOOWWWWWWWOOwWWWwOWWWWOowwwOwOWWWWWWWWWWWwWwwwwwoOWWWWOwWWwWwWWWWOwwWOWOWoWWWWWWwoWWWWOowOwOwOWWWWWWWWWWWwWwwwOwoOWWWWWwWWwoOWOOwwOWWWWWOWWOoOWWWwWWWWOowwwwwOWWWWWWWWWWWwWwwwOwOWWWWWWwWWWWWWWWWwowoooWWOOwwWWWWWWWWWWOwOwwwWwWWWWWWWWWWOowOOOwoWWWWWWwWWWWWWWWOWOWWWOOWWWWWWWWWWWWWWWWOOOOwOWWWWWWWWWWWWowOOOWoWWWWWWwWWWWWWWWWOWWWOOOOWWWWWWWWoWWWWWoWwOwWWWWWWWWWWWWWWOwWWWOWWWWWWOOWWWWWWWWwWWWWWOOwwWWWWWWwwOWWWWWOWOWWWWWOoowWWWWWWWOwwwWWWWWWWwwWWWWWWowWWWWWWWOWowOWWWWwWwWWWWWWwWoWWWwwWWOOWWWWWWWWWWWWWWWOWWWWWWWwowWWWWWWWWWWwOoWOoWoWWWWWWWWWWWWWWWwwwwWWWWWWWWWWWWWWWWowwwwWWWowwWWWWWWWWWWwOOOooowWwOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWowOWWWWWWWWWwOWOOwOowWwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWoWwWWWWWWOOWWWWWWWWWWoOWWWOoWwwWWWWWWWWWWWWWWWOwOWWWWOOoWWWWWWWWOOOWWWWWWWoWWWWWWWWWwoWWWWWwoWWWWWWWWWWWWWWWWOowwOoWOwOOWWWWWWWWWWWWWWWWWWWoWoWWWWWwowWWWWWWWWWWWWWWWWWWOOoOWWOOowWOOoOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWWWWWWWWWWWWWWWWWWWWWOwWwWWWWWWWWWWWWWWWWWWWWOoWWWWWOwWOOWWWWWWoooooOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWowOowwwWwwwWOWWWWWwWwOwwOWooWoWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOoWWWOWWWWWWOOWWWWWWWWWOWWWWWWwWoOwoOWWWWWWWWWWWWWWWWWWWWWWWWwWWwwWoWWWWWWWWWWWWWWWWWWWWWWWWWWWWowOOoWWwWWWWWWWWWWWWWWWWWWWOWowWWOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWoWWWwOWoooOWWWWWWWWWWWWWWOOoOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWooWOOWOOOOoWWWWWWWWWWWWWWWWOWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWooOWWWWWWWWWWWWWWWWWWWWWWWWWowWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwOwWWWWWWWWWWWWWWWWWWWWWWWoooWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOWowWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOoOWWWWWWWWWWWWWWWWWWWWWWWoowWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOOOwWWWWWWWWWWWWWWWWWWWWWWWWOOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWwwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWowWWWWWWWWWWWWWWWWWWWWWWWWWWOowWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWooOOoWWWWWWWWWWWWWWWWWWWWWWWWwwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwOWOWWWWWWWWWWWWWWWWWWWWWWWWOooWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwOWWWWWWWWWWWWWWWWWWWWWWWWWOOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWoWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWoOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwowWWWWWWWWWWWWWWWWWWWWWWWWWWwwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWW"
};

uint8_t bk1[64 * 64] =
    {
"WWWWwOWWWWWWWWWWWwOWWWWWWOwwwwwOWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWwOWWWWwOWWWWWWWWWWWOwWWWWWWOwWWWOwwwOWWWWWWWWWWwOWWWWWWWWWWWWWWWwOWWWWwWWWWWWWWWWWWWwwWWWWWWwWWWWWWwwwWWWWWWWWwwWWWWWWWWWWWWWWWWwwWWWOwWWWWWWWWWWWWWWwwWWWWWwWWWWWWWWwwOWWWWWOwWWWWWWWWWWWWWWWWWwwWWWwwWWWWWWWWWWWWWWWwOWWWWwOWWWWWWWWOwwWWWWwwWWWWWWWWWWWWWWWWWOwWWWwOWWWWWWWWWWWWWWWOoOWWWOwWWWWWWWWWWwwWWOwWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWOwWWWWwOWWWWWWWWWWwwOwOWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWWwwWWWwwWWWWWWWWWWWOowWWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWWWwwOwwowWWWWWWWWWWWwoWWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWWWWowOWWOWWWWWWWWWWWWOWWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWwowWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWwwwwwwwwwwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOOOOOOOOWWWWWWWWWwWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwwwwwwwwwwowwOWWWWWWWOwWWWWWWwOWWWWWOwwwwwwwwooooowWWWWWWWWWWWWwooooowWWWOwWWWWWWWWWWwOWWWWWWOwWWWWWWWwwWWWWwooooooOWWWWWWWWWWWwooooowWWWWwwWWWWWWWWwwWWWWWWWWwOWWWWWWwOWWWWwooooooOWWWWWWWWWWWoooooowWWWWOwWWWWWWOwwWWWWWWWWWOwWWWWWWwWWWWWwooooooOWWWWWWWWWWWoooooowWWWWWwWWWWWOowOWWWWWWWWWWwwWWWWOwWWWWWwooooooOWWWWWWWWWWWoooooowWWWWWwOWWWwwowwwwwOwwwOOWOwOWWWOOWWWWWwooooooOWWWWWWWWWWWoooooowWWWWWwOWWWOOOWWWWwwwowwwwwwwOWWwOWWWWWwooooooOWWWWWWWWWWWoooooowWWWWWwOWWWWWWWWWOwWWwWWWWWWOOWWwOWWWWWwooooooWWWWWWWWWWWWwoooooOWWWWWwOWWWWWWWWOoOWWwwWWWWWWWWWwOWWWWWwooooowWWWWWWWWWWWWOoooooWWWWWWwOWWWWWWWOoOWWWWwwWWWWWWWWwOWWWWWWoooooOWWWWWWWWWWWWWwoooOWWWWWWwOWWWWWWOoOWWWWWWwwWWWWWWWOwWWWWWWOoooOWWWWWWWWWWWWWWWWWWWWWWWWWOWWWWWWOwOWWWWWWWWwwOWWWWWWwWWWWWWWWOWWWWWwoowWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWOoWWWWOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOOWWOWWWWWwWWWWWWWWWWWOwWWOOOOWWOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwwOwOWWWWWOwWWWWWWWWWWwOWWOWOOOwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOOOwOWWWWWWWwwWWWWWWWWOwWWWWWOwOWWWWWWWWWWWWWWWWWWWWWWWWwWWWWWWWWWOOWWWWWWWWWoOWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWOwwwwOOOwwWWWWWWWWWWWWWWWWWWWWOoWWWWWWWwWWWWWWWWWWWWWWWWWWWWwOOwoOWWOwwwOWWWWWWWWWWWWWWWWWWWWWWwwWWWWWOwWWWWWWWWWWWWWWWWWWWWOwwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwwWWWWWOwWWWWOOwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWOOOwwwwwOWWWWWWwwwwwwwwwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwowWOwwwwOWWWWWWWWWWWWWWWWWWWwwwOWWWWWWWWWWWWWWWWWWWWWWWWWWWOwwwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwowwwWWWWWWWWWWWWWWWWWWWWWwwwwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWoooowOWWWWWWWWWWWWWWWWWWWOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWOwwwwwWWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwOWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwOwwOWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOOOOwwWWWWWWWWWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwwWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWwWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWOwWWWWWWWWWWWWWWWWWWWWWWWWWWWWwOWWWWWWWWWWWW"
};

uint8_t bkpal[16] = {
	0x10, 0x20, 0x30, 0x40,
	0x50, 0x60, 0x70, 0x7f,
	0x8f, 0x9f, 0xaf, 0xbf,
	0xcf, 0xdf, 0xef, 0xff
};

pl_Texture *mkArt(bool evil)
{
	int sz = 64;
	pl_Texture *p = (pl_Texture *) malloc(sizeof(pl_Texture));
	if (p) {
		p->Data = malloc(sz * sz);
		p->PaletteData = malloc(3 * 16);
		if (p->Data && p->PaletteData) {
			int x, y;
			p->Width = 6;
			p->Height = 6;
			p->iWidth = sz;
			p->iHeight = sz;
			p->uScale = sz;
			p->vScale = sz;
			p->NumColors = 16;
			p->ClearColor = -1;
			for (y = 0; y < sz; y++) {
				for (x = 0; x < sz; x++) {
					uint8_t c;
					if (bk0[y * sz + x] == 'o'
					    && bk1[y * sz + x] == 'o')
						c = evil ? 15 : 0;
					else if (bk0[y * sz + x] == 'w'
						 && bk1[y * sz + x] == 'o')
						c = evil ? 14 : 1;
					else if (bk0[y * sz + x] == 'O'
						 && bk1[y * sz + x] == 'o')
						c = evil ? 13 : 2;
					else if (bk0[y * sz + x] == 'W'
						 && bk1[y * sz + x] == 'o')
						c = evil ? 12 : 3;
					else if (bk0[y * sz + x] == 'o'
						 && bk1[y * sz + x] == 'w')
						c = evil ? 11 : 4;
					else if (bk0[y * sz + x] == 'w'
						 && bk1[y * sz + x] == 'w')
						c = evil ? 10 : 5;
					else if (bk0[y * sz + x] == 'O'
						 && bk1[y * sz + x] == 'w')
						c = evil ? 9 : 6;
					else if (bk0[y * sz + x] == 'W'
						 && bk1[y * sz + x] == 'w')
						c = evil ? 8 : 7;
					else if (bk0[y * sz + x] == 'o'
						 && bk1[y * sz + x] == 'O')
						c = evil ? 7 : 8;
					else if (bk0[y * sz + x] == 'w'
						 && bk1[y * sz + x] == 'O')
						c = evil ? 6 : 9;
					else if (bk0[y * sz + x] == 'O'
						 && bk1[y * sz + x] == 'O')
						c = evil ? 5 : 10;
					else if (bk0[y * sz + x] == 'W'
						 && bk1[y * sz + x] == 'O')
						c = evil ? 4 : 11;
					else if (bk0[y * sz + x] == 'o'
						 && bk1[y * sz + x] == 'W')
						c = evil ? 3 : 12;
					else if (bk0[y * sz + x] == 'w'
						 && bk1[y * sz + x] == 'W')
						c = evil ? 2 : 13;
					else if (bk0[y * sz + x] == 'O'
						 && bk1[y * sz + x] == 'W')
						c = evil ? 1 : 14;
					else
						c = evil ? 0 : 15;
					p->Data[y * sz + x] = c;
				}
			}

			for (x = 0; x < 16; x++) {
				p->PaletteData[x * 3 + 0] = bkpal[x];
				p->PaletteData[x * 3 + 1] = bkpal[x];
				p->PaletteData[x * 3 + 2] = bkpal[x];
			}
		} else {
			printf("mkArt failed\n");
			exit(1);
		}
	}
	return p;
}

pl_Texture *mkGroundTex()
{
	pl_Texture *p = (pl_Texture *) malloc(sizeof(pl_Texture));
	if (p) {
		p->Data = malloc(16 * 16);
		p->PaletteData = malloc(3 * 16);
		if (p->Data && p->PaletteData) {
			int x, y;
			p->Width = 4;
			p->Height = 4;
			p->iWidth = 16;
			p->iHeight = 16;
			p->uScale = 16 * 3;
			p->vScale = 16 * 3;
			p->NumColors = 16;
			p->ClearColor = -1;
			for (y = 0; y < 16; y++)
				for (x = 0; x < 16; x++)
					p->Data[y * 16 + x] = x ^ y;
			for (x = 0; x < 16; x++) {
				p->PaletteData[x*3+0] = 43 + ((93-43) * x) / 16;
				p->PaletteData[x*3+1] = 25 + ((52-25) * x) / 16;
				p->PaletteData[x*3+2] = 9 + ((23-9) * x) / 16;
			}
		} else {
			printf("mkGroundTex failed\n");
			exit(1);
		}
	}
	return p;
}

int MulDiv(int nNumber, int nNumerator, int nDenominator)
{
	if (nDenominator == 0)
		return 0;
	int64_t r = (int64_t) nNumber * (int64_t) nNumerator;
	if (r >= 0)
		r += nDenominator / 2;
	else
		r -= nDenominator / 2;
	return (int)(r / nDenominator);
}

void adjustmapping(pl_Obj *obj)
{
	int nf = obj->Model->NumFaces;
	int x;
	pl_Face *f = obj->Model->Faces;
	for (x = 0; x < nf; x++) {
		f->MappingV[0] = MulDiv(f->MappingV[0], (150<<16)/500, (1<<16));
		f->MappingV[1] = MulDiv(f->MappingV[1], (150<<16)/500, (1<<16));
		f->MappingV[2] = MulDiv(f->MappingV[2], (150<<16)/500, (1<<16));
		f++;
	}
}

pl_Obj *setup_landscape(float size, int div, pl_Mat *m)
{
	pl_Obj *o = plObjCreate(NULL);
	pl_Obj *o1 = plObjCreate(o);
	pl_Obj *o2 = plObjCreate(o);
	pl_Obj *o3 = plObjCreate(o);
	pl_Obj *o4 = plObjCreate(o);
	pl_Obj *o5 = plObjCreate(o);

	o->Model = plMakePlane(size, size, div, m);

	o1->Model = plMakePlane(size, size, div, m);
	o1->Yp = 150;
	o1->Xa = -180;
	div /= 3;

	o2->Model = plMakePlane(size, 150, div, m);
	o2->Yp = 75;
	o2->Zp = size / 2;
	o2->Xa = 90;
	adjustmapping(o2);

	o3->Model = plMakePlane(size, 150, div, m);
	o3->Yp = 75;
	o3->Zp = -size / 2;
	o3->Xa = -90;
	adjustmapping(o3);

	o4->Model = plMakePlane(size, 150, div, m);
	o4->Yp = 75;
	o4->Xp = size / 2;
	o4->Za = -90;
	o4->Ya = 90;
	adjustmapping(o4);

	o5->Model = plMakePlane(size, 150, div, m);
	o5->Yp = 75;
	o5->Xp = -size / 2;
	o5->Za = 90;
	o5->Ya = 90;
	adjustmapping(o5);

	o->Yp = 0;
	return (o);
}

void initspline(int i)
{
	spline.keys[i * 5 + 0] = (float)(rand() % 320) - 160;
	spline.keys[i * 5 + 1] = (float)(rand() % 110) + 15;
	spline.keys[i * 5 + 2] = (float)(rand() % 320) - 160;
	spline.keys[i * 5 + 3] = (float)(rand() % 32) / 32.0f;
	spline.keys[i * 5 + 4] = (float)(rand() % 32) / 32.0f;
}

void setup_materials(pl_Mat **mat, unsigned char *pal)
{
	mat[0] = plMatCreate();
	mat[1] = plMatCreate();
	mat[2] = plMatCreate();
	mat[3] = plMatCreate();
	mat[4] = plMatCreate();
	mat[5] = plMatCreate();
	mat[6] = 0;

	watex = mkWATex();
	grndtex = mkGroundTex();

	mat[0]->ShadeType = PL_SHADE_GOURAUD;
	mat[0]->Shininess = 16;
	mat[0]->NumGradients = 1500;
	mat[0]->Ambient[0] = -128;
	mat[0]->Ambient[1] = -128;
	mat[0]->Ambient[2] = -128;
	mat[0]->Diffuse[0] = 170;
	mat[0]->Diffuse[1] = 140;
	mat[0]->Diffuse[2] = 140;
	mat[0]->Specular[0] = 140;
	mat[0]->Specular[1] = 90;
	mat[0]->Specular[2] = 0;
	mat[0]->FadeDist = 5000.0;
	mat[0]->Texture = fuckomode ? watex : grndtex;
	mat[0]->TexScaling = 8.0;
	mat[0]->PerspectiveCorrect = 16;

	mat[1]->ShadeType = PL_SHADE_GOURAUD;
	mat[1]->Shininess = 8;
	mat[1]->NumGradients = 150;
	mat[1]->Ambient[0] = 0;
	mat[1]->Ambient[1] = 0;
	mat[1]->Ambient[2] = 0;
	mat[1]->Diffuse[0] = 0;
	mat[1]->Diffuse[1] = 0;
	mat[1]->Diffuse[2] = 0;
	mat[1]->Specular[0] = 450;
	mat[1]->Specular[1] = 264;
	mat[1]->Specular[2] = 150;

	mat[2]->ShadeType = PL_SHADE_GOURAUD;
	mat[2]->Shininess = 8;
	mat[2]->NumGradients = 150;
	mat[2]->Ambient[0] = 32;
	mat[2]->Ambient[1] = 32;
	mat[2]->Ambient[2] = 64;
	mat[2]->Diffuse[0] = 120;
	mat[2]->Diffuse[1] = 60;
	mat[2]->Diffuse[2] = 60;
	mat[2]->Specular[0] = 200;
	mat[2]->Specular[1] = 80;
	mat[2]->Specular[2] = 80;

	mat[3]->ShadeType = PL_SHADE_GOURAUD;
	mat[3]->Shininess = 1;
	mat[3]->NumGradients = 1;
	mat[3]->Ambient[0] = 0;
	mat[3]->Ambient[1] = 0;
	mat[3]->Ambient[2] = 0;
	mat[3]->Diffuse[0] = 0;
	mat[3]->Diffuse[1] = 0;
	mat[3]->Diffuse[2] = 0;
	mat[3]->Specular[0] = 0;
	mat[3]->Specular[1] = 0;
	mat[3]->Specular[2] = 0;
	mat[3]->Texture = fuckomode ? grndtex : watex;
	mat[3]->TexScaling = 1.4f;
	mat[3]->PerspectiveCorrect = 16;

	mat[4]->ShadeType = PL_SHADE_GOURAUD;
	mat[4]->Shininess = 1;
	mat[4]->NumGradients = 1;
	mat[4]->Ambient[0] = 0;
	mat[4]->Ambient[1] = 0;
	mat[4]->Ambient[2] = 0;
	mat[4]->Diffuse[0] = 0;
	mat[4]->Diffuse[1] = 0;
	mat[4]->Diffuse[2] = 0;
	mat[4]->Specular[0] = 0;
	mat[4]->Specular[1] = 0;
	mat[4]->Specular[2] = 0;
	mat[4]->Texture = mkArt(0);
	mat[4]->TexScaling = 1.0;
	mat[4]->PerspectiveCorrect = 16;

	mat[5]->ShadeType = PL_SHADE_GOURAUD;
	mat[5]->Shininess = 1;
	mat[5]->NumGradients = 1;
	mat[5]->Ambient[0] = 0;
	mat[5]->Ambient[1] = 0;
	mat[5]->Ambient[2] = 0;
	mat[5]->Diffuse[0] = 0;
	mat[5]->Diffuse[1] = 0;
	mat[5]->Diffuse[2] = 0;
	mat[5]->Specular[0] = 0;
	mat[5]->Specular[1] = 0;
	mat[5]->Specular[2] = 0;
	mat[5]->Texture = mkArt(1);
	mat[5]->TexScaling = 1.0;
	mat[5]->PerspectiveCorrect = 16;

	plMatInit(mat[0]);
	plMatInit(mat[1]);
	plMatInit(mat[2]);
	plMatInit(mat[3]);
	plMatInit(mat[4]);
	plMatInit(mat[5]);

	memset(pal, 0, 768);
	plMatMakeOptPal(pal, 1, 255, mat, 6);

	pal[0] = pal[1] = pal[2] = 0;
	pal[3] = pal[4] = pal[5] = 255;

	plMatMapToPal(mat[0], pal, 0, 255);
	plMatMapToPal(mat[1], pal, 0, 255);
	plMatMapToPal(mat[2], pal, 0, 255);
	plMatMapToPal(mat[3], pal, 0, 255);
	plMatMapToPal(mat[4], pal, 0, 255);
	plMatMapToPal(mat[5], pal, 0, 255);

	exSetPalette(pal);
}

void render_init(int w, int h, char *pal)
{
	int i;
	float wd;
	int ishigh = 0;

	rpoo = 1;
	splineTime = 0.0;
	spline.tens = (float)-0.6;
	spline.keyWidth = 5;
	spline.numKeys = 8;
	spline.keys = keys1;
	for (i = 0; i < spline.numKeys; i++)
		initspline(i);

	cam = plCamCreate(W, H, W*3.0 / (H*4.0), 120.0, framebuffer, zbuffer);
	cam->ScreenWidth = W;
	cam->ScreenHeight = H;
	cam->ClipTop = 1;
	cam->ClipLeft = 1;
	cam->ClipBottom = h - 1;
	cam->ClipRight = w - 1;
	cam->AspectRatio = 1.0;
	cam->CenterX = w / 2;
	cam->CenterY = h / 2;
	cam->Sort = 1;
	cam->Z = -300;
	cam->ClipBack = 1500.0;

	light = plLightCreate();

	setup_materials(mat, (uint8_t *)pal);
	land = setup_landscape(LAND_SIZE, ishigh?LAND_DIV*2 : LAND_DIV, mat[0]);

	teamobject = plObjCreate(NULL);
	teamobject->Model = plMakeBox(32, 32, 32, mat[5]);
	teamobject->Yp = 75;
	teamobject->Xp = -LAND_SIZE / 3;
	teamobject->Za = -90;
	teamobject->Ya = 90;
	teamobject->BackfaceCull = 0;

	billboard = plObjCreate(NULL);
	billboard->Model = plMakePlane(LAND_SIZE / 3, 90, 1, mat[4]);
	billboard->Yp = 75;
	billboard->Xp = LAND_SIZE / 3;
	billboard->Za = -90;
	billboard->Ya = 90;
	billboard->BackfaceCull = 0;

	wd = 9.8f;
	for (i = 0; i < OBJ_NUM; i++) {
		object[i] = plObjCreate(i == 0 ? NULL : object[0]);
		object[i]->Xp = 0;
		object[i]->Yp = 90;
		object[i]->Zp = 0;
		object[i]->Model = plMakeTorus(wd, wd + 4.0f,
				ishigh ? (64 + i * 4) : (12 + i * 2),
				ishigh ? 12 : 6, mat[1 + (i & 1)]);
		wd += 5.8f;
	}

	lightobject = plObjCreate(NULL);
	lightobject->Model = plMakeSphere(17.0,ishigh?16:6,ishigh?24:8,mat[3]);

	prevtime = exClock();
}

void render_render()
{
	static float light_sc = 0.2f + 2 * 0.3f;
	float curpos[5];
	int i;
	unsigned int now = exClock();
	unsigned int t = now - prevtime;

	if (t < 0) t = 0;
	splineTime += (float)(t * (0.01 / 33.0)) * speed;
	prevtime = now;

	if (splineTime > spline.numKeys) {
		for (i = 2; i < spline.numKeys - 2; i++)
			initspline(i);
		rpoo = 0;
		splineTime -= spline.numKeys;
	}
	if (!rpoo && splineTime > 3.0) {
		rpoo = 1;
		initspline(0);
		initspline(1);
		initspline(spline.numKeys - 2);
		initspline(spline.numKeys - 1);
	}
	plSplineGetPoint(&spline, splineTime, curpos);

	for (i = OBJ_NUM - 1; i > 0; i--) {
		object[i]->Xa = (object[i - 1]->Xa + object[i]->Xa) / 2;
		object[i]->Ya = (object[i - 1]->Ya + object[i]->Ya) / 2;
		object[i]->Za = (object[i - 1]->Za + object[i]->Za) / 2;
	}
	object[0]->Xa += 3 * curpos[3] * speed;
	object[0]->Ya += 3 * curpos[4] * speed;
	object[0]->Za -= 3 * (curpos[4] * curpos[3]) * speed;

	cam->X = curpos[0];
	cam->Y = curpos[1];
	cam->Z = curpos[2];

	teamobject->Xp = (float)(100.0 * sin((splineTime - 0.1) * 3.14159));
	teamobject->Yp = (float)(75 + 40.0 * cos((splineTime - 0.1) * 3.14159));
	teamobject->Zp = (float)(100.0 * cos((splineTime-0.1) * 3.14159 * 1.5));
	teamobject->Ya += 1.1f * speed;

	lightobject->Xp = (float)(100.0 * sin(splineTime * 3.14159));
	lightobject->Yp = (float)(75+40.0 * cos(splineTime * 3.14159));
	lightobject->Zp = (float)(100.0 * cos(splineTime * 3.14159 * 1.5));
	lightobject->Ya += 1.1f * speed;

	plLightSet(light, PL_LIGHT_POINT, lightobject->Xp, lightobject->Yp,
		   lightobject->Zp, 0.7f - 0.2f / 2 + light_sc / 2,
		   LAND_SIZE / 2);

	plCamSetTarget(cam, lightobject->Xp / 4,
		       lightobject->Yp / 4.0f + 90 * 0.75f,
		       lightobject->Zp / 4);

	billboard->Za += 1.0 * speed;
	billboard->Ya += 1.0 * speed;

	if (watex && watex->Data) {
		makeBallTexture(watex->Data);
		makeBallTexture(watex->Data);
	}

	memset(zbuffer, 0, sizeof(zbuffer));
	memset(framebuffer, 0, sizeof(framebuffer));

	plRenderBegin(cam);
	plRenderLight(light);
	plRenderObj(land);
	plRenderObj(lightobject);
	plRenderObj(billboard);
	plRenderObj(teamobject);
	for (i = 0; i < OBJ_NUM; i++)
		plRenderObj(object[i]);
	plRenderEnd();

	exWaitVSync();
	memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
}

int main()
{
	exSetGraphics();
	render_init(W, H, pal);
	while (!exGetKey()) {
		render_render();
	}
}
