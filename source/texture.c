/******************************************************************************
Plush Version 1.2
texture.c
Texture routines
Copyright (c) 2024, erysdren (it/she/they)
******************************************************************************/

#include "plush.h"

pl_uInt _plHiBit(pl_uInt16 x) {
  pl_uInt i = 16, mask = 1<<15;
  while (mask) {
    if (x & mask) return i;
    mask >>= 1; i--;
  }
  return 0;
}

pl_uInt _plOptimizeImage(pl_uChar *pal, pl_uChar *data, pl_uInt32 len) {
  pl_uChar colors[256], *dd = data;
  pl_uChar remap[256];
  pl_sInt32 lastused, firstunused;
  pl_uInt32 x;
  memset(colors,0,256);
  for (x = 0; x < len; x ++) colors[(pl_uInt) *dd++] = 1;
  lastused = -1;
  for (x = 0; x < 256; x ++) remap[x] = (pl_uChar)x;
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
	  remap[lastused] = (pl_uChar) firstunused;
  }
  x = len;
  while (x--) *data++ = remap[(pl_uInt) *data];
  return (lastused+1);
}

void _plRescaleImage(pl_uChar *in, pl_uChar *out, pl_uInt inw,
                            pl_uInt inh, pl_uInt outx, pl_uInt outy) {
  pl_uInt x;
  pl_uInt32 X, dX, dY, Y;
  dX = (inw<<16) / outx;
  dY = (inh<<16) / outy;
  Y = 0;
  do {
    pl_uChar *ptr = in + inw*(Y>>16);
    X = 0;
    Y += dY;
    x = outx;
    do {
      *out++ = ptr[X>>16];
      X += dX;
    } while (--x);
  } while (--outy);
}

pl_Texture *plTexCreate(pl_uInt w, pl_uInt h, pl_uChar *p, pl_uInt nc, pl_uChar *c)
{
	pl_Texture *t = (pl_Texture *)malloc(sizeof(pl_Texture));

	/* create copy of palette data */
	t->PaletteData = (pl_uChar *)malloc(nc * 3);
	memcpy(t->PaletteData, c, nc * 3);

	/* rescale image */
	t->Width = _plHiBit(w);
	t->Height = _plHiBit(h);
	if (1 << t->Width != w || 1 << t->Height != h)
	{
		pl_uChar nw, nh;

		nw = t->Width;
		if ((1 << t->Width) != w) nw++;

		nh = t->Height;
		if ((1 << t->Height) != h) nh++;

		t->Data = (pl_uChar *)malloc((1 << nw) * (1 << nh));

		_plRescaleImage(p, t->Data, w, h, 1 << nw, 1 << nh);

		t->Width = nw;
		t->Height = nh;
		w = 1 << nw;
		h = 1 << nh;
	}
	else
	{
		/* create copy of pixel data */
		t->Data = (pl_uChar *)malloc(w * h);
		memcpy(t->Data, p, w * h);
	}

	/* setup fields */
	t->iWidth = w;
	t->iHeight = h;
	t->uScale = (pl_Float)(1 << t->Width);
	t->vScale = (pl_Float)(1 << t->Height);
	t->NumColors = nc;

	return t;
}

void plTexDelete(pl_Texture *t) {
  if (t) {
    if (t->Data) free(t->Data);
    if (t->PaletteData) free(t->PaletteData);
    free(t);
  }
}
