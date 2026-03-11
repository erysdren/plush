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
	int i, j, k;
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
		void *chunk_data;

		/* read chunk header */
		if (io->read(chunk_id, sizeof(chunk_id), 1, user) != sizeof(chunk_id))
			break;
		io->read(&chunk_len, sizeof(chunk_len), 1, user);

		/* byte swap */
		chunk_len = plBig32(chunk_len);

		/* save next chunk position */
		next_chunk = io->tell(user) + chunk_len + (chunk_len % 2);

		/* read chunk data */
		chunk_data = plMalloc(chunk_len);
		io->read(chunk_data, chunk_len, 1, user);

		/* act on chunk type */
		if (plStrNCmp(chunk_id, "BMHD", 4) == 0)
		{
			plMemCpy(&bmhd, chunk_data, sizeof(lbm_bmhd_t));
			bmhd.width = plBig16(bmhd.width);
			bmhd.height = plBig16(bmhd.height);
			bmhd.origin_x = plBig16(bmhd.origin_x);
			bmhd.origin_y = plBig16(bmhd.origin_y);
			bmhd.mask_color = plBig16(bmhd.mask_color);
			bmhd.page_width = plBig16(bmhd.page_width);
			bmhd.page_height = plBig16(bmhd.page_height);

			pixels = plMalloc(bmhd.width * bmhd.height);
		}
		else if (plStrNCmp(chunk_id, "CMAP", 4) == 0)
		{
			uint8_t *colors = (uint8_t *)chunk_data;
			int num_colors = chunk_len / 3;

			if (num_colors > 256)
				return -3;

			palette = plMalloc(768);

			for (i = 0; i < num_colors; i++)
			{
				palette[i * 3 + 0] = colors[i * 3 + 0];
				palette[i * 3 + 1] = colors[i * 3 + 1];
				palette[i * 3 + 2] = colors[i * 3 + 2];
			}
		}
		else if (plStrNCmp(chunk_id, "BODY", 4) == 0)
		{
			if (plStrNCmp(format_id, "PBM ", 4) == 0)
			{
				if (bmhd.compression)
				{
					uint8_t *body, *dst;

					body = (uint8_t *)chunk_data;
					dst = pixels;

					for (i = 0; i < bmhd.height; i++)
					{
						size_t decompressed_length = 0;
						while (decompressed_length < bmhd.width)
						{
							uint8_t marker = *body++;
							if (marker > 128)
							{
								uint8_t value = *body++;
								for (j = 0; j < 257 - marker; j++)
								{
									dst[decompressed_length++] = value;
								}
							}
							else if (marker < 128)
							{
								for (j = 0; j < marker + 1; j++)
								{
									dst[decompressed_length++] = *body++;
								}
							}
							else if (marker == 128)
							{
								break;
							}
						}

						dst += bmhd.width;
					}
				}
				else
				{
					plMemCpy(pixels, chunk_data, chunk_len);
				}
			}
			else
			{
				const uint8_t *body = (const uint8_t *)chunk_data;
				uint8_t *dst = pixels;
				size_t row_size = bmhd.width / 8;
				uint8_t *row_buf = alloca(row_size);
				uint8_t *src;
				int bitidx;

				for (i = 0; i < bmhd.height; i++)
				{
					plMemSet(dst, 0, bmhd.width);

					for (j = 0; j < bmhd.num_planes; j++)
					{
						if (bmhd.compression)
						{
							size_t decompressed_length = 0;
							while (decompressed_length < row_size)
							{
								uint8_t marker = *body++;
								if (marker > 128)
								{
									uint8_t value = *body++;
									for (k = 0; k < 257 - marker; k++)
									{
										row_buf[decompressed_length++] = value;
									}
								}
								else if (marker < 128)
								{
									for (k = 0; k < marker + 1; k++)
									{
										row_buf[decompressed_length++] = *body++;
									}
								}
								else if (marker == 128)
								{
									break;
								}
							}
						}
						else
						{
							plMemCpy(row_buf, body, row_size);
							body += row_size;
						}

						/* distribute all bits across the linear output scanline */
						src = row_buf;
						bitidx = 0;

						for (k = 0; k < bmhd.width; k++)
						{
							dst[k] |= ((*src >> (7 - bitidx)) & 1) << j;

							if (++bitidx >= 8)
							{
								bitidx = 0;
								++src;
							}
						}
					}

					dst += bmhd.width;
				}
			}
		}

		/* clean up */
		plFree(chunk_data);

		/* seek to next chunk */
		io->seek(user, next_chunk, SEEK_SET);
	}

	if (!pixels || !palette)
	{
		return -4;
	}

	*width = bmhd.width;
	*height = bmhd.height;
	*data = pixels;
	*pal = palette;

	return 0;
}
