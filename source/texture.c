/******************************************************************************
Plush Version 1.2
texture.c
Texture routines
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

uint32_t _plHiBit(uint16_t x) {
  uint32_t i = 16, mask = 1<<15;
  while (mask) {
    if (x & mask) return i;
    mask >>= 1; i--;
  }
  return 0;
}

uint32_t _plOptimizeImage(uint8_t *pal, uint8_t *data, uint32_t len) {
  uint8_t colors[256], *dd = data;
  uint8_t remap[256];
  int32_t lastused, firstunused;
  uint32_t x;
  plMemSet(colors,0,256);
  for (x = 0; x < len; x ++) colors[(uint32_t) *dd++] = 1;
  lastused = -1;
  for (x = 0; x < 256; x ++) remap[x] = (uint8_t)x;
  lastused = 255;
  firstunused = 0;
  for (;;) {
    while (firstunused < 256 && colors[firstunused]) firstunused++;
    if (firstunused > 255) break;
    while (lastused >= 0 && !colors[lastused]) lastused--;
    if (lastused < 0) break;
	if (lastused <= firstunused) break;
    pal[firstunused*3] = pal[lastused*3];
    pal[firstunused*3+1] = pal[lastused*3+1];
    pal[firstunused*3+2] = pal[lastused*3+2];
    colors[lastused] = 0;
    colors[firstunused] = 1;
	  remap[lastused] = (uint8_t) firstunused;
  }
  x = len;
  while (x--) *data++ = remap[(uint32_t) *data];
  return (lastused+1);
}

void _plRescaleImage(uint8_t *in, uint8_t *out, uint32_t inw,
                            uint32_t inh, uint32_t outx, uint32_t outy) {
  uint32_t x;
  uint32_t X, dX, dY, Y;
  dX = (inw<<16) / outx;
  dY = (inh<<16) / outy;
  Y = 0;
  do {
    uint8_t *ptr = in + inw*(Y>>16);
    X = 0;
    Y += dY;
    x = outx;
    do {
      *out++ = ptr[X>>16];
      X += dX;
    } while (--x);
  } while (--outy);
}

pl_Texture *plTexCreate(uint32_t w, uint32_t h, uint8_t *p, uint32_t nc, uint8_t *c)
{
	pl_Texture *t = (pl_Texture *)plMalloc(sizeof(pl_Texture));

	/* create copy of palette data */
	t->PaletteData = (uint8_t *)plMalloc(nc * 3);
	plMemCpy(t->PaletteData, c, nc * 3);

	/* rescale image */
	t->Width = _plHiBit(w);
	t->Height = _plHiBit(h);
	if (1 << t->Width != w || 1 << t->Height != h)
	{
		uint8_t nw, nh;

		nw = t->Width;
		if ((1 << t->Width) != w) nw++;

		nh = t->Height;
		if ((1 << t->Height) != h) nh++;

		t->Data = (uint8_t *)plMalloc((1 << nw) * (1 << nh));

		_plRescaleImage(p, t->Data, w, h, 1 << nw, 1 << nh);

		t->Width = nw;
		t->Height = nh;
		w = 1 << nw;
		h = 1 << nh;
	}
	else
	{
		/* create copy of pixel data */
		t->Data = (uint8_t *)plMalloc(w * h);
		plMemCpy(t->Data, p, w * h);
	}

	/* setup fields */
	t->iWidth = w;
	t->iHeight = h;
	t->uScale = (float)(1 << t->Width);
	t->vScale = (float)(1 << t->Height);
	t->NumColors = nc;

	return t;
}

void plTexDelete(pl_Texture *t) {
  if (t) {
    if (t->Data) plFree(t->Data);
    if (t->PaletteData) plFree(t->PaletteData);
    plFree(t);
  }
}
