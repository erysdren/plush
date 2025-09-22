// fire.c: procgen texture example
// https://www.flipcode.com/archives/The_Art_of_Demomaking-Issue_05_Filters.shtml
// https://lodev.org/cgtutor/fire.html
// owo

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <plush/plush.h>

#include "ex.h" 

#pragma pack(push, 1)

typedef struct pcx_header {
	uint8_t manufacturer;
	uint8_t version;
	uint8_t encoding;
	uint8_t bits_per_pixel;
	uint16_t xmin, ymin, xmax, ymax;
	uint16_t hres, vres;
	uint8_t palette[48];
	uint8_t reserved;
	uint8_t color_planes;
	uint16_t bytes_per_line;
	uint16_t palette_type;
	uint16_t swidth, sheight;
	uint8_t filler[54];
} pcx_header_t;

#pragma pack(pop)

static void SavePCX(const char *filename, int w, int h, int stride, uint8_t *screen, uint8_t *palette)
{
	int i, x, y;
	FILE *file;
	uint8_t *ptr;
	pcx_header_t *pcx = (pcx_header_t *)malloc(w * h * 2 + 1024);

	pcx->manufacturer = 0x0A;
	pcx->version = 5;
	pcx->encoding = 1;
	pcx->bits_per_pixel = 8;
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = w - 1;
	pcx->ymax = h - 1;
	pcx->hres = w;
	pcx->vres = h;
	memset(pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;
	pcx->bytes_per_line = w;
	pcx->palette_type = 2;
	memset(pcx->filler, 0, sizeof(pcx->filler));

	ptr = (uint8_t *)(pcx + 1);

	/* write image data */
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if ((*screen & 0xC0) != 0xC0)
			{
				*ptr++ = *screen++;
			}
			else
			{
				*ptr++ = 0xC1;
				*ptr++ = *screen++;
			}
		}

		screen += stride - w;
	}

	/* write palette */
	*ptr++ = 0x0C;
	for (i = 0; i < 768; i++)
		*ptr++ = *palette++;

	file = fopen(filename, "wb");
	fwrite(pcx, ptr - (uint8_t *)pcx,  1, file);
	fclose(file);

	free(pcx);
}

/* 0 - fire
 * 1 - skull
 * 2 - crazy
 * 3 - fucking
 * 4 - skeleton
 * 5 - moment
 * 6 - brick
 */
#define NUM_MATERIALS (7)

pl_Light *light;
pl_Obj *firewall, *brickwall, *skull, *text[4];
pl_Mat *materials[NUM_MATERIALS];
pl_Cam *camera;
uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];

#define FIRE_WIDTH (64)
#define FIRE_HEIGHT (64)
#define FIRE_NUM_COLORS (32)

uint8_t fire_pixels[FIRE_HEIGHT][FIRE_HEIGHT];
uint8_t fire_palette[FIRE_NUM_COLORS * 3];
pl_Texture fire_texture;

static void Shade_Pal(uint8_t *pal, int s, int e, int r1, int g1, int b1, int r2, int g2, int b2)
{
	int i;
	float k;
	for (i = 0; i <= e - s; i++)
	{
		k = (float)i / (float)(e - s);
		pal[(s + i) * 3 + 0] = (int)(r1 + (r2 - r1) * k);
		pal[(s + i) * 3 + 1] = (int)(g1 + (g2 - g1) * k);
		pal[(s + i) * 3 + 2] = (int)(b1 + (b2 - b1) * k);
	}
}

int main(int argc, char **argv)
{
	int num_renderframes = 30;
	int renderframe = 0;
	bool rendermode = false;
	int i, x, y;
	int dir = 1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-render") == 0)
		{
			rendermode = true;

			if (i < argc - 1)
			{
				num_renderframes = atoi(argv[i + 1]);
				i++;
			}

			if (num_renderframes < 1 || num_renderframes > 1024)
				num_renderframes = 30;
		}
	}

	/* setup graphics mode */
	if (!rendermode)
		exSetGraphics();

	/* setup fire palette */
	Shade_Pal(fire_palette, 0, (FIRE_NUM_COLORS / 8) - 1, 0, 0, 0, 0, 0, 127);
	Shade_Pal(fire_palette, FIRE_NUM_COLORS / 8, (FIRE_NUM_COLORS / 4) - 1, 0, 0, 127, 255, 0, 0);
	Shade_Pal(fire_palette, FIRE_NUM_COLORS / 4, (FIRE_NUM_COLORS / 2) - 1, 255, 255, 0, 255, 255, 255);
	Shade_Pal(fire_palette, FIRE_NUM_COLORS / 2, FIRE_NUM_COLORS - 1, 255, 255, 255, 255, 255, 255);

	/* hack up fire texture */
	fire_texture.Data = (uint8_t *)fire_pixels;
	fire_texture.PaletteData = fire_palette;
	fire_texture.NumColors = FIRE_NUM_COLORS;
	fire_texture.Width = 6;
	fire_texture.Height = 6;
	fire_texture.iWidth = FIRE_WIDTH;
	fire_texture.iHeight = FIRE_HEIGHT;
	fire_texture.uScale = 256;
	fire_texture.vScale = 64;
	fire_texture.ClearColor = 0;

	/* create fire material */
	materials[0] = plMatCreate();
	materials[0]->ShadeType = PL_SHADE_GOURAUD;
	materials[0]->Shininess = 16;
	materials[0]->NumGradients = 2500;
	materials[0]->Ambient[0] = -128;
	materials[0]->Ambient[1] = -128;
	materials[0]->Ambient[2] = -128;
	materials[0]->Diffuse[0] = 170;
	materials[0]->Diffuse[1] = 140;
	materials[0]->Diffuse[2] = 140;
	materials[0]->Specular[0] = 140;
	materials[0]->Specular[1] = 90;
	materials[0]->Specular[2] = 0;
	materials[0]->Texture = &fire_texture;

	/* create skull material */
	materials[1] = plMatCreate();
	materials[1]->ShadeType = PL_SHADE_GOURAUD;
	materials[1]->Texture = plReadPCXTex("skull.pcx", true, true);
	materials[1]->Environment = &fire_texture;
	materials[1]->TexEnvMode = PL_TEXENV_ADD;
	materials[1]->Ambient[0] = materials[1]->Ambient[1] = materials[1]->Ambient[2] = -128;
	materials[1]->Diffuse[0] = materials[1]->Diffuse[1] = materials[1]->Diffuse[2] = 0;
	materials[1]->NumGradients = 2500;

	/* create text materials */
	for (i = 2; i < 6; i++)
	{
		char filename[64];
		sprintf(filename, "text%02d.pcx", i - 1);

		materials[i] = plMatCreate();
		materials[i]->ShadeType = PL_SHADE_NONE;
		materials[i]->Texture = plReadPCXTex(filename, true, false);
		materials[i]->Texture->ClearColor = 216;
	}

	/* create brick material */
	materials[6] = plMatCreate();
	materials[6]->ShadeType = PL_SHADE_GOURAUD;
	materials[6]->Texture = plReadPCXTex("brick.pcx", true, true);
	materials[6]->Texture->uScale = 1024;
	materials[6]->Texture->vScale = 512;
	materials[6]->Ambient[0] = materials[6]->Ambient[1] = materials[6]->Ambient[2] = -144;
	materials[6]->Diffuse[0] = materials[6]->Diffuse[1] = materials[6]->Diffuse[2] = 0;
	materials[6]->NumGradients = 2500;

	/* initialize materials */
	for (i = 0; i < NUM_MATERIALS; i++)
		plMatInit(materials[i]);

	/* create palette */
	plMatMakeOptPal(palette, 2, 255, materials, NUM_MATERIALS);
	palette[0] = palette[1] = palette[2] = 0;
	palette[3] = palette[4] = palette[5] = 255;

	for (i = 0; i < NUM_MATERIALS; i++)
		plMatMapToPal(materials[i], palette, 0, 255);

	if (!rendermode)
		exSetPalette(palette);

	/* create firewall */
	firewall = plObjCreate(NULL);
	firewall->Model = plMakePlane(512, 256, 1, materials[0]);
	firewall->Xa = 90;
	firewall->Za = 180;

	/* create brickwall */
	brickwall = plObjCreate(NULL);
	brickwall->Model = plMakePlane(1024, 512, 16, materials[6]);
	brickwall->Xa = 90;
	brickwall->Za = 180;
	brickwall->Zp = 256;

	/* create skull */
	skull = plObjCreate(NULL);
	skull->Model = plReadWavefrontMdl("skull.obj", materials[1]);
	skull->Ya = 180;

	/* create text models */
	for (i = 0; i < 4; i++)
	{
		text[i] = plObjCreate(NULL);
		text[i]->Xa = 90;
		text[i]->BackfaceCull = false;
	}

	text[0]->Model = plMakePlane(83, 29, 1, materials[2]);
	text[1]->Model = plMakePlane(110, 29, 1, materials[3]);
	text[2]->Model = plMakePlane(122, 29, 1, materials[4]);
	text[3]->Model = plMakePlane(110, 29, 1, materials[5]);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 90.0, framebuffer, zbuffer);
	camera->Z = -300;

	/* create light */
	light = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

	/* main loop */
	while ((rendermode && renderframe < num_renderframes) || (!rendermode && !exGetKey()))
	{
		/* clear framebuffer and zbuffer */
		memset(framebuffer, 0, sizeof(framebuffer));
		memset(zbuffer, 0, sizeof(zbuffer));

		/* heat bottom row of fire pixels */
		for (x = 0; x < FIRE_WIDTH; x++)
			fire_pixels[FIRE_HEIGHT - 1][x] = abs(32768 + rand()) % FIRE_NUM_COLORS;

		/* do fire filter */
		for (y = 0; y < FIRE_HEIGHT - 1; y++)
		{
			for (x = 0; x < FIRE_WIDTH; x++)
			{
				fire_pixels[y][x] =
					((fire_pixels[(y + 1) % FIRE_HEIGHT][(x - 1 + FIRE_WIDTH) % FIRE_WIDTH]
					+ fire_pixels[(y + 1) % FIRE_HEIGHT][(x) % FIRE_WIDTH]
					+ fire_pixels[(y + 1) % FIRE_HEIGHT][(x + 1) % FIRE_WIDTH]
					+ fire_pixels[(y + 2) % FIRE_HEIGHT][(x) % FIRE_WIDTH])
					* 32) / 129;
			}
		}

		/* move skull around */
		skull->Zp -= dir * 2;
		if (skull->Zp < -64)
			dir = -1;
		if (skull->Zp > 64)
			dir = 1;

		skull->Ya += 4;

		/* shake text around */
		text[0]->Xp = -64 + rand() % 6;
		text[0]->Yp = 96 + rand() % 6;

		text[1]->Xp = 64 + rand() % 6;
		text[1]->Yp = 96 + rand() % 6;

		text[2]->Xp = -64 + rand() % 6;
		text[2]->Yp = -96 + rand() % 6;

		text[3]->Xp = 64 + rand() % 6;
		text[3]->Yp = -96 + rand() % 6;

		/* render frame */
		plRenderBegin(camera);
		plRenderLight(light);
		plRenderObj(skull);
		plRenderObj(brickwall);
		plRenderObj(firewall);
		plRenderEnd();

		/* render text */
		memset(zbuffer, 0, sizeof(zbuffer));
		plRenderBegin(camera);
		plRenderObj(text[0]);
		plRenderObj(text[1]);
		plRenderObj(text[2]);
		plRenderObj(text[3]);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		if (!rendermode)
		{
			exWaitVSync();
			memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
		}
		else
		{
			char name[64];
			snprintf(name, sizeof(name), "fire%04d.pcx", renderframe);
			SavePCX(name, W, H, W, framebuffer, palette);
			printf("wrote %s\n", name);
			renderframe++;
		}
	}

	/* clean up */
	plCamDelete(camera);
	plLightDelete(light);
	plMdlDelete(skull->Model);
	plMdlDelete(firewall->Model);
	plMdlDelete(brickwall->Model);
	plMdlDelete(text[0]->Model);
	plMdlDelete(text[1]->Model);
	plMdlDelete(text[2]->Model);
	plMdlDelete(text[3]->Model);
	plObjDelete(skull);
	plObjDelete(firewall);
	plObjDelete(brickwall);
	plObjDelete(text[0]);
	plObjDelete(text[1]);
	plObjDelete(text[2]);
	plObjDelete(text[3]);
	plTexDelete(materials[1]->Texture);
	plTexDelete(materials[2]->Texture);
	plTexDelete(materials[3]->Texture);
	plTexDelete(materials[4]->Texture);
	plTexDelete(materials[5]->Texture);
	for (i = 0; i < NUM_MATERIALS; i++)
		plMatDelete(materials[i]);

	/* shut down video */
	if (!rendermode)
		exSetText();

	return 0;
}
