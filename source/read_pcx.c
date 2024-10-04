/******************************************************************************
Plush Version 1.2
read_pcx.c
PCX Texture Reader
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

/* texture.c */
pl_uInt _plHiBit(pl_uInt16 x);
pl_uInt _plOptimizeImage(pl_uChar *pal, pl_uChar *data, pl_uInt32 len);
void _plRescaleImage(pl_uChar *in, pl_uChar *out, pl_uInt inx,
                            pl_uInt iny, pl_uInt outx, pl_uInt outy);

/* read_pcx.c */
static pl_sInt _plReadPCX(char *filename, pl_uInt16 *width, pl_uInt16 *height, 
                          pl_uChar **pal, pl_uChar **data);

pl_Texture *plReadPCXTex(char *fn, pl_Bool rescale, pl_Bool optimize) {
  pl_uChar *data, *pal;
  pl_uInt16 x, y;
  pl_Texture *t;
  if (_plReadPCX(fn,&x,&y,&pal,&data) < 0) return 0;
  t = (pl_Texture *) malloc(sizeof(pl_Texture));
  if (!t) return 0;
  t->Width = _plHiBit(x);
  t->Height = _plHiBit(y);
  if (rescale && (1 << t->Width != x || 1 << t->Height != y)) {
    pl_uChar nx, ny, *newdata;
    nx = t->Width;
    if ((1 << t->Width) != x) nx++;
    ny = t->Height;
    if ((1 << t->Height) != y) ny++;
    newdata = (pl_uChar *) malloc((1<<nx)*(1<<ny));
    if (!newdata) {
      free(t);
      free(data);
      free(pal);
      return 0;
    }
    _plRescaleImage(data,newdata,x,y,1<<nx,1<<ny);
    free(data);
    data = newdata;
    t->Width = nx;
    t->Height = ny;
    x = 1<<nx; y = 1<<ny;
  }
  t->iWidth = x;
  t->iHeight = y;
  t->uScale = (pl_Float) (1<<t->Width);
  t->vScale = (pl_Float) (1<<t->Height);
  if (optimize) t->NumColors = _plOptimizeImage(pal, data,x*y);
  else t->NumColors = 256;
  t->Data = data;
  t->PaletteData = pal;
  return t;
}

static pl_sInt _plReadPCX(char *filename, pl_uInt16 *width, pl_uInt16 *height,
                            pl_uChar **pal, pl_uChar **data) {
  pl_uInt16 sx, sy, ex, ey;
  FILE *fp = fopen(filename,"rb");
  pl_uChar *data2;
  if (!fp) return -1;
  fgetc(fp);
  if (fgetc(fp) != 5) { fclose(fp); return -2; }
  if (fgetc(fp) != 1) { fclose(fp); return -2; }
  if (fgetc(fp) != 8) { fclose(fp); return -3; }
  sx = fgetc(fp); sx |= fgetc(fp)<<8;
  sy = fgetc(fp); sy |= fgetc(fp)<<8;
  ex = fgetc(fp); ex |= fgetc(fp)<<8;
  ey = fgetc(fp); ey |= fgetc(fp)<<8;
  *width = ex - sx + 1;
  *height = ey - sy + 1;
  fseek(fp,128,SEEK_SET);
  if (feof(fp)) { fclose(fp); return -4; }
  *data = (pl_uChar *) malloc((*width) * (*height));
  if (!*data) { fclose(fp); return -128; }
  sx = *height;
  data2 = *data;
  do { 
    int xpos = 0;
    do {
      char c = fgetc(fp);
      if ((c & 192) == 192) {
        char oc = fgetc(fp);
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
  if (feof(fp)) { fclose(fp); free(*data); return -5; }
  fseek(fp,-769,SEEK_END);
  if (fgetc(fp) != 12) { fclose(fp); free(*data); return -6; }
  *pal = (pl_uChar *) malloc(768);
  if (!*pal) { fclose(fp); free(*data); return -7; }
  fread(*pal,3,256,fp);
  fclose(fp);
  return 0;
}
