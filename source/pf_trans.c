/******************************************************************************
Plush Version 1.2
pf_trans.c
Solid Translucent Rasterizers
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>
#include "putface.h"

void plPF_TransF(pl_Cam *cam, pl_Face *TriFace) {
  uint8_t i0, i1, i2;
  uint8_t *gmem = cam->frameBuffer;
  uint8_t *remap = TriFace->Material->_ReMapTable;
  float *zbuf = cam->zBuffer;
  int32_t X1, X2, dX1=0, dX2=0, XL1, XL2;
  float Z1, ZL, dZ1=0, dZL=0, dZ2=0, Z2;
  int32_t Y1, Y2, Y0, dY;
  uint16_t *lookuptable = TriFace->Material->_AddTable;
  uint8_t stat;
  int32_t bc = (int32_t) TriFace->fShade*TriFace->Material->_tsfact;
  bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;

  PUTFACE_SORT();
  
  if (bc > (int32_t) TriFace->Material->_tsfact-1) bc=TriFace->Material->_tsfact-1;
  if (bc < 0) bc=0;
  remap+=bc;

  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      stat= 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      stat= 1|8;
    }
  } 

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);
  if (zb) {
    XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
    if (XL1) dZL = ((dZ1-dZ2)*dY)/XL1;
    else { 
      XL1 = ((X2-X1+(1<<19))>>20);
      if (XL1) dZL = (Z2-Z1)/XL1;
    }
  }

  while (Y0 < Y2) {
    if (Y0 == Y1) {
      dY = Y2 - ((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]- Z1)/dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1; 
      zbuf += XL1;
      gmem += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = remap[lookuptable[*gmem]];
          }
          gmem++; 
          zbuf++;
          ZL += dZL;
        } while (--XL2);
      else do *gmem++ = remap[lookuptable[*gmem]]; while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    Y0 ++;
  }
}

void plPF_TransG(pl_Cam *cam, pl_Face *TriFace) {
  uint8_t i0, i1, i2;
  uint8_t *gmem = cam->frameBuffer;
  uint8_t *remap = TriFace->Material->_ReMapTable;
  float *zbuf = cam->zBuffer;
  int32_t X1, X2, dX1=0, dX2=0, XL1, XL2;
  float Z1, ZL, dZ1=0, dZL=0, dZ2=0, Z2;
  int32_t dC1=0, dCL=0, CL, C1, C2, dC2=0;
  int32_t Y1, Y2, Y0, dY;
  float nc = (TriFace->Material->_tsfact*65536.0f);
  uint16_t *lookuptable = TriFace->Material->_AddTable;
  bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;
  uint8_t stat;

  int32_t maxColor=((TriFace->Material->_tsfact-1)<<16);
  int32_t maxColorNonShift=TriFace->Material->_tsfact-1;

  PUTFACE_SORT();

  C1 = C2 = (int32_t) (TriFace->Shades[i0]*nc);
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dC2 = (int32_t) ((TriFace->Shades[i2]*nc - C1) / dY);
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dC1 = (int32_t) ((TriFace->Shades[i1]*nc - C1) / dY);
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      C2 = (int32_t) (TriFace->Shades[i1]*nc);
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      C1 = (int32_t) (TriFace->Shades[i1]*nc);
      stat = 1|8;
    }
  } 

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);
  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dCL = ((dC1-dC2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dCL = (C2-C1)/XL1;
      dZL = (Z2-Z1)/XL1;
    }
  }

  while (Y0 < Y2) {
    if (Y0 == Y1) {
      dY = Y2 - ((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1)/dY;
        dC1 = (int32_t) ((TriFace->Shades[i2]*nc - C1) / dY);
      }
    }
    CL = C1;
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1; 
      zbuf += XL1;
      gmem += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            int av;
            if (CL >= maxColor) av=maxColorNonShift;
            else if (CL > 0) av=CL>>16;
            else av=0;
            *zbuf = ZL;
            *gmem = remap[av + lookuptable[*gmem]];
          }
          gmem++; 
          CL += dCL;
          zbuf++;
          ZL += dZL;
        } while (--XL2);
      else do {
          int av;
          if (CL >= maxColor) av=maxColorNonShift;
          else if (CL > 0) av=CL>>16;
          else av=0;
          *gmem++ = remap[av + lookuptable[*gmem]];
          CL += dCL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
    Y0++;
  }
}
