/******************************************************************************
Plush Version 1.2
read_pcx.c
PCX Texture Reader
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

#include "readio.h"

/* texture.c */
uint32_t _plHiBit(uint16_t x);
uint32_t _plOptimizeImage(uint8_t *pal, uint8_t *data, uint32_t len);
void _plRescaleImage(uint8_t *in, uint8_t *out, uint32_t inx, uint32_t iny, uint32_t outx, uint32_t outy);

/* read_pcx.c */
static int _plReadPCX(pl_IO *io, void *user, uint16_t *width, uint16_t *height, uint8_t **pal, uint8_t **data);

static pl_Texture *_plProcessPCX(uint16_t x, uint16_t y, uint8_t *data, uint8_t *pal, bool rescale, bool optimize)
{
  pl_Texture *t = (pl_Texture *) plMalloc(sizeof(pl_Texture));
  if (!t) return 0;
  t->Width = _plHiBit(x);
  t->Height = _plHiBit(y);
  if (rescale && (1 << t->Width != x || 1 << t->Height != y)) {
    uint8_t nx, ny, *newdata;
    nx = t->Width;
    if ((1 << t->Width) != x) nx++;
    ny = t->Height;
    if ((1 << t->Height) != y) ny++;
    newdata = (uint8_t *) plMalloc((1<<nx)*(1<<ny));
    if (!newdata) {
      plFree(t);
      plFree(data);
      plFree(pal);
      return 0;
    }
    _plRescaleImage(data,newdata,x,y,1<<nx,1<<ny);
    plFree(data);
    data = newdata;
    t->Width = nx;
    t->Height = ny;
    x = 1<<nx; y = 1<<ny;
  }
  t->iWidth = x;
  t->iHeight = y;
  t->uScale = (float) (1<<t->Width);
  t->vScale = (float) (1<<t->Height);
  if (optimize) t->NumColors = _plOptimizeImage(pal, data,x*y);
  else t->NumColors = 256;
  t->Data = data;
  t->PaletteData = pal;
  return t;
}

pl_Texture *plReadPCXTex(const char *fn, bool rescale, bool optimize) {
  uint8_t *data, *pal;
  uint16_t x, y;
  int r;
  FILE *fp = fopen(fn, "rb");
  if (!fp) return 0;
  r = _plReadPCX(&_plIOStdio,fp,&x,&y,&pal,&data);
  fclose(fp);
  if (r < 0) return 0;
  return _plProcessPCX(x, y, data, pal, rescale, optimize);
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
  return _plProcessPCX(x, y, data, pal, rescale, optimize);
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
