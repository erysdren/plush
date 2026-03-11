/******************************************************************************
Plush Version 1.2
read_lbm.c
LBM Texture Reader
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2026, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

#include "readio.h"
#include "texture.h"

#pragma pack(push, 1)

typedef struct lbm_bmhd {
	uint16_t width;
	uint16_t height;
	int16_t origin_x;
	int16_t origin_y;
	uint8_t num_planes;
	uint8_t mask;
	uint8_t compression;
	uint8_t pad;
	uint16_t mask_color;
	uint8_t aspect_x;
	uint8_t aspect_y;
	int16_t page_width;
	int16_t page_height;
} lbm_bmhd_t;

#pragma pack(pop)

static int _plReadLBM(pl_IO *io, void *user, uint16_t *width, uint16_t *height, uint8_t **pal, uint8_t **data);

pl_Texture *plReadLBMTex(const char *fn, bool rescale, bool optimize)
{
	uint8_t *data, *pal;
	uint16_t x, y;
	int r;
	FILE *fp = fopen(fn, "rb");
	if (!fp) return 0;
	r = _plReadLBM(&_plIOStdio,fp,&x,&y,&pal,&data);
	fclose(fp);
	if (r < 0) return 0;
	return _plProcessTexture(x, y, data, pal, rescale, optimize);
}

pl_Texture *plReadLBMTexFromMem(void *buf, size_t len, bool rescale, bool optimize)
{
  uint8_t *data, *pal;
  uint16_t x, y;
  int r;
  pl_IOMemCtx ctx;
  ctx.buffer = buf;
  ctx.len = len;
  ctx.pos = 0;
  r = _plReadLBM(&_plIOMem,&ctx,&x,&y,&pal,&data);
  if (r < 0) return 0;
  return _plProcessTexture(x, y, data, pal, rescale, optimize);
}

static int _plReadLBM(pl_IO *io, void *user, uint16_t *width, uint16_t *height, uint8_t **pal, uint8_t **data)
{
	char file_id[4];
	char format_id[4];
	uint32_t file_len;
	uint8_t *pixels = NULL;
	uint8_t *palette = NULL;
	lbm_bmhd_t bmhd;

	/* read file id */
	io->read(file_id, sizeof(file_id), 1, user);
	if (plStrNCmp(file_id, "FORM", 4) != 0)
		return -1;

	/* read file size */
	io->read(&file_len, sizeof(file_len), 1, user);

	/* read format id */
	io->read(format_id, sizeof(format_id), 1, user);
	if (plStrNCmp(format_id, "ILBM", 4) != 0 && plStrNCmp(format_id, "PBM ", 4) != 0)
		return -2;

	while (!io->eof(user))
	{
		char chunk_id[4];
		uint32_t chunk_len;
		uint32_t next_chunk;

		/* read chunk header */
		if (io->read(chunk_id, sizeof(chunk_id), 1, user) != sizeof(chunk_id))
			break;
		io->read(&chunk_len, sizeof(chunk_len), 1, user);

		/* byte swap */
		chunk_len = plBig32(chunk_len);

		/* save next chunk position */
		next_chunk = io->tell(user) + chunk_len + (chunk_len % 2);

		/* act on chunk type */
		if (plStrNCmp(chunk_id, "BMHD", 4) == 0)
		{
			io->read(&bmhd, sizeof(lbm_bmhd_t), 1, user);
			bmhd.width = plBig16(bmhd.width);
			bmhd.height = plBig16(bmhd.height);
			bmhd.origin_x = plBig16(bmhd.origin_x);
			bmhd.origin_y = plBig16(bmhd.origin_y);
			bmhd.mask_color = plBig16(bmhd.mask_color);
			bmhd.page_width = plBig16(bmhd.page_width);
			bmhd.page_height = plBig16(bmhd.page_height);
		}
		else if (plStrNCmp(chunk_id, "BODY", 4) == 0)
		{

		}

		/* seek to next chunk */
		io->seek(user, next_chunk, SEEK_SET);
	}

	if (!pixels || !palette)
	{
		return -3;
	}

	return 0;
}
