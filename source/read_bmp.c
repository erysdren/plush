/******************************************************************************
Plush Version 1.2
read_bmp.c
BMP Texture Reader
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2026, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

#include "readio.h"
#include "texture.h"

#pragma pack(push, 1)

typedef struct bmp_header {
	char magic[2];
	uint32_t len_file;
	uint16_t reserved[2];
	uint32_t ofs_pixels;
} bmp_header_t;

typedef struct bmp_dib {
	uint32_t len_dib;
	int32_t width;
	int32_t height;
	uint16_t num_planes;
	uint16_t bpp;
	uint32_t compression;
	uint32_t len_pixels;
	int32_t res_x;
	int32_t res_y;
	uint32_t num_colors;
	uint32_t num_important_colors;
} bmp_dib_t;

#pragma pack(pop)

static int _plReadBMP(pl_IO *io, void *user, uint16_t *width, uint16_t *height, uint8_t **pal, uint8_t **data)
{
	bmp_header_t header;
	bmp_dib_t dib;
	uint8_t *picdest, *paldest;
	uint8_t *pic_p, *pal_p;
	uint8_t *temppal, *temppixels, *pixels_p;
	int i, y;
	int stride;
	bool topdown = false;

	/* read header */
	io->read(&header, sizeof(header), 1, user);
	if (plStrNCmp(header.magic, "BM", sizeof(header.magic)) != 0)
		return -1;

	/* byteswap */
	header.len_file = plLittle32(header.len_file);
	header.ofs_pixels = plLittle32(header.ofs_pixels);

	/* read dib */
	io->read(&dib, sizeof(dib), 1, user);

	/* byteswap */
	dib.len_dib = plLittle32(dib.len_dib);
	dib.width = plLittle32(dib.width);
	dib.height = plLittle32(dib.height);
	dib.num_planes = plLittle16(dib.num_planes);
	dib.bpp = plLittle16(dib.bpp);
	dib.compression = plLittle32(dib.compression);
	dib.len_pixels = plLittle32(dib.len_pixels);
	dib.res_x = plLittle32(dib.res_x);
	dib.res_y = plLittle32(dib.res_y);
	dib.num_colors = plLittle32(dib.num_colors);
	dib.num_important_colors = plLittle32(dib.num_important_colors);

	/* validate */
	if (dib.len_dib < sizeof(bmp_dib_t))
		return -2;
	if (dib.num_planes != 1)
		return -3;
	if (dib.bpp != 8)
		return -4;
	if (dib.compression != 0)
		return -5;

	/* check for inverted height value */
	if (dib.height < 0)
	{
		dib.height = -dib.height;
		topdown = true;
	}

	/* allocate */
	paldest = plMalloc(768);
	pic_p = picdest = plMalloc(dib.width * dib.height);

	/* read palette */
	pal_p = temppal = (uint8_t *)plMalloc(dib.num_colors * 4);
	io->read(temppal, 4, dib.num_colors, user);

	/* copy palette */
	for (i = 0; i < dib.num_colors; i++)
	{
		paldest[i * 3 + 0] = pal_p[2];
		paldest[i * 3 + 1] = pal_p[1];
		paldest[i * 3 + 2] = pal_p[0];

		pal_p += 4;
	}

	/* fill in the rest of the palette */
	for (i = dib.num_colors; i < 256; i++)
	{
		paldest[i * 3 + 0] = 0;
		paldest[i * 3 + 1] = 0;
		paldest[i * 3 + 2] = 0;
	}

	/* read pixel data */
	pixels_p = temppixels = (uint8_t *)plMalloc(dib.len_pixels);
	io->seek(user, header.ofs_pixels, SEEK_SET);
	io->read(temppixels, dib.len_pixels, 1, user);

	stride = (dib.width + 3) & ~3;

	/* copy pixel data */
	if (topdown)
	{
		for (y = 0; y < dib.height; y++)
		{
			memcpy(pic_p, pixels_p, dib.width);
			pic_p += dib.width;
			pixels_p += stride;
		}
	}
	else
	{
		pixels_p += (dib.height - 1) * stride;
		for (y = 0; y < dib.height; y++)
		{
			memcpy(pic_p, pixels_p, dib.width);
			pic_p += dib.width;
			pixels_p -= stride;
		}
	}

	/* clean up */
	plFree(temppal);
	plFree(temppixels);

	/* return stuff */
	*width = dib.width;
	*height = dib.height;
	*pal = paldest;
	*data = picdest;

	return 0;
}

pl_Texture *plReadBMPTex(const char *fn, bool rescale, bool optimize)
{
	uint8_t *data, *pal;
	uint16_t x, y;
	int r;
	FILE *fp = fopen(fn, "rb");
	if (!fp) return 0;
	r = _plReadBMP(&_plIOStdio,fp,&x,&y,&pal,&data);
	fclose(fp);
	if (r < 0) return 0;
	return _plProcessTexture(x, y, data, pal, rescale, optimize);
}

pl_Texture *plReadBMPTexFromMem(void *buf, size_t len, bool rescale, bool optimize)
{
	uint8_t *data, *pal;
	uint16_t x, y;
	int r;
	pl_IOMemCtx ctx;
	ctx.buffer = buf;
	ctx.len = len;
	ctx.pos = 0;
	r = _plReadBMP(&_plIOMem,&ctx,&x,&y,&pal,&data);
	if (r < 0) return 0;
	return _plProcessTexture(x, y, data, pal, rescale, optimize);
}
