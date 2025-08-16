/******************************************************************************
Plush Version 1.1
text.c
Text code and data (8xX bitmapped)
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>
#include <stdarg.h>

static uint8_t font_height = 16;

static uint8_t *current_font = plText_DefaultFont;

void plTextSetFont(uint8_t *font, uint8_t height) {
  current_font = font;
  font_height = height;
}

void plTextPutChar(pl_Cam *cam, int32_t x, int32_t y, float z,
                   uint8_t color, uint8_t c) {
  uint8_t *font = current_font + (c*font_height);
  int32_t offset = x+(y*cam->ScreenWidth);
  float zz = (float) (1.0/z);
  int32_t xx = x, a;
  uint8_t len = font_height;
  uint8_t ch;
  uint8_t *outmem;
  float *zbuffer;
  if (y+font_height < cam->ClipTop || y >= cam->ClipBottom) return;
  if (y < cam->ClipTop) {
    font += (cam->ClipTop-y);
    offset += (cam->ClipTop-y)*cam->ScreenWidth;
    len -= (cam->ClipTop-y);
    y = cam->ClipTop;
  }
  if (y+font_height >= cam->ClipBottom) {
    len = cam->ClipBottom-y;
  }
  if (len > 0) {
    if (cam->zBuffer && z != 0.0) do {
      outmem = cam->frameBuffer + offset;
      zbuffer = cam->zBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft) 
          if (ch & a)
            if (zz > *zbuffer) {
              *zbuffer = zz;
              *outmem = color;
            }
        zbuffer++;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
    else do {
      outmem = cam->frameBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft) if (ch & a) *outmem = color;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
  }  
}

void plTextPutStr(pl_Cam *cam, int32_t x, int32_t y, float z,
                  uint8_t color, const char *string) {
  int32_t xx = x;
  while (*string) {
    switch (*string) {
      case '\n': y += font_height; xx = x; break;
      case ' ': xx += 8; break;
      case '\r': break;
      case '\t': xx += 8*5; break;
      default:
        plTextPutChar(cam,xx,y,z,color,(uint8_t) *string);
        xx += 8;
      break;
    }
    string++;
  }
}

void plTextPrintf(pl_Cam *cam, int32_t x, int32_t y, float z,
                  uint8_t color, const char *format, ...) {
  va_list arglist;
  char str[256];
  va_start(arglist, format);
  vsprintf((char *)str, (char *) format,arglist);
  va_end(arglist);
  plTextPutStr(cam,x,y,z,color,str);
}
