/******************************************************************************
Plush Version 1.2
read_pcx.c
PCX Texture Reader
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

#include "readio.h"
#include "texture.h"

static int _plReadPCX(pl_IO *io, void *user, uint16_t *width, uint16_t *height, uint8_t **pal, uint8_t **data);

pl_Texture *plReadPCXTex(const char *fn, bool rescale, bool optimize) {
  uint8_t *data, *pal;
  uint16_t x, y;
  int r;
  FILE *fp = fopen(fn, "rb");
  if (!fp) return 0;
  r = _plReadPCX(&_plIOStdio,fp,&x,&y,&pal,&data);
  fclose(fp);
  if (r < 0) return 0;
  return _plProcessTexture(x, y, data, pal, rescale, optimize);
}

pl_Texture *plReadPCXTexFromMem(void *buf, size_t len, bool rescale, bool optimize) {
  uint8_t *data, *pal;
  uint16_t x, y;
  int r;
  pl_IOMemCtx ctx;
  ctx.buffer = buf;
  ctx.len = len;
  ctx.pos = 0;
  r = _plReadPCX(&_plIOMem,&ctx,&x,&y,&pal,&data);
  if (r < 0) return 0;
  return _plProcessTexture(x, y, data, pal, rescale, optimize);
}

static int _plReadPCX(pl_IO *io, void *user, uint16_t *width, uint16_t *height, uint8_t **pal, uint8_t **data)
{
  uint16_t sx, sy, ex, ey;
  uint8_t *data2;
  io->getchr(user);
  if (io->getchr(user) != 5) return -2;
  if (io->getchr(user) != 1) return -2;
  if (io->getchr(user) != 8) return -3;
  sx = io->getchr(user); sx |= io->getchr(user)<<8;
  sy = io->getchr(user); sy |= io->getchr(user)<<8;
  ex = io->getchr(user); ex |= io->getchr(user)<<8;
  ey = io->getchr(user); ey |= io->getchr(user)<<8;
  *width = ex - sx + 1;
  *height = ey - sy + 1;
  io->seek(user,128,SEEK_SET);
  if (io->eof(user)) return -4;
  *data = (uint8_t *) plMalloc((*width) * (*height));
  if (!*data) return -128;
  sx = *height;
  data2 = *data;
  do { 
    int xpos = 0;
    do {
      char c = io->getchr(user);
      if ((c & 192) == 192) {
        char oc = io->getchr(user);
        c &= ~192;
        do {
          *(data2++) = oc;
          xpos++;
        } while (--c && xpos < *width);
      } else {
        *(data2++) = c;
        xpos++;
      }
    } while (xpos < *width);
  } while (--sx);
  if (io->eof(user)) { plFree(*data); return -5; }
  io->seek(user,-769,SEEK_END);
  if (io->getchr(user) != 12) { plFree(*data); return -6; }
  *pal = (uint8_t *) plMalloc(768);
  if (!*pal) { plFree(*data); return -7; }
  io->read(*pal,3,256,user);
  return 0;
}
