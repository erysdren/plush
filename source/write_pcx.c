/******************************************************************************
Plush Version 1.2
write_pcx.c
PCX Image Writer
Copyright (C) 2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

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
	uint8_t filler[58];
} pcx_header_t;

#pragma pack(pop)

bool plWritePCX(const char *fn, int w, int h, int stride, uint8_t *pixels, uint8_t *palette)
{
	int i, x, y;
	FILE *file;
	uint8_t *ptr;
	pcx_header_t *pcx = (pcx_header_t *)plMalloc(w * h * 2 + 1024);
	if (!pcx)
		return false;

	pcx->manufacturer = 10;
	pcx->version = 5;
	pcx->encoding = 1;
	pcx->bits_per_pixel = 8;
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = w - 1;
	pcx->ymax = h - 1;
	pcx->hres = w;
	pcx->vres = h;
	plMemSet(pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;
	pcx->bytes_per_line = w;
	pcx->palette_type = 2;
	plMemSet(pcx->filler, 0, sizeof(pcx->filler));

	ptr = (uint8_t *)(pcx + 1);

	/* write image data */
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if ((*pixels & 0xC0) != 0xC0)
			{
				*ptr++ = *pixels++;
			}
			else
			{
				*ptr++ = 0xC1;
				*ptr++ = *pixels++;
			}
		}

		pixels += stride - w;
	}

	/* write palette */
	*ptr++ = 0x0C;
	for (i = 0; i < 768; i++)
		*ptr++ = *palette++;

	file = fopen(fn, "wb");
	if (!file)
	{
		plFree(pcx);
		return false;
	}

	fwrite(pcx, ptr - (uint8_t *)pcx,  1, file);
	fclose(file);

	plFree(pcx);

	return true;
}
