/******************************************************************************
Plush Version 1.2
texture.h
Texture loader helper functions
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2026, erysdren (it/its)
******************************************************************************/

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <plush/plush.h>

uint32_t _plHiBit(uint16_t x);
uint32_t _plOptimizeImage(uint8_t *pal, uint8_t *data, uint32_t len);
void _plRescaleImage(uint8_t *in, uint8_t *out, uint32_t inx, uint32_t iny, uint32_t outx, uint32_t outy);
pl_Texture *_plProcessTexture(uint16_t x, uint16_t y, uint8_t *data, uint8_t *pal, bool rescale, bool optimize);

#endif /* !_TEXTURE_H_ */
