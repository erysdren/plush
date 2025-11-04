/******************************************************************************
Plush Version 1.2
pf_solid.c
Solid Rasterizers
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>
#include "putface.h"

void plPF_SolidF(pl_Cam *cam, pl_PrepFace *TriFace) {
  uint8_t i0, i1, i2;

  uint8_t *gmem = cam->frameBuffer;
  float *zbuf = cam->zBuffer;

  int32_t X1, X2, dX1=0, dX2=0, XL1, XL2;
  float dZL=0, dZ1=0, dZ2=0, Z1, ZL, Z2, Z3;
  int32_t Y1, Y2, Y0, dY;
  uint8_t stat;
  bool zb = (zbuf&&TriFace->Face->Material->zBufferable) ? 1 : 0;
  uint8_t bc;
  int32_t shade;

  PUTFACE_SORT();

  shade=(int32_t) (TriFace->fShade*(TriFace->Face->Material->_ColorsUsed-1));
  if (shade < 0) shade=0;
  if (shade > (int32_t) TriFace->Face->Material->_ColorsUsed-1) shade=TriFace->Face->Material->_ColorsUsed-1;
  bc=TriFace->Face->Material->_ReMapTable[shade];

  X2 = X1 = TriFace->Scrx[i0];
  Z1 = TriFace->Scrz[i0];
  Z2 = TriFace->Scrz[i1];
  Z3 = TriFace->Scrz[i2];
  Y0 = (TriFace->Scry[i0]+(1<<19)) >> 20;
  Y1 = (TriFace->Scry[i1]+(1<<19)) >> 20;
  Y2 = (TriFace->Scry[i2]+(1<<19)) >> 20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (Z3 - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (Z2 - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
    Z2 = Z1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      ZL = Z1; Z1 = Z2; Z2 = ZL;
      stat = 1|8;
    }
  } 

  if (zb) {
    XL1 = ((dX1-dX2)*dY+(1<<19))>>20;
    if (XL1) dZL = ((dZ1-dZ2)*dY)/XL1;
    else { 
      XL1 = (X2-X1+(1<<19))>>20;
      if (zb && XL1) dZL = (Z2-Z1)/XL1;
      else dZL = 0.0;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

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
        dZ1 = (Z3-Z1)/dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    XL2 -= XL1; 
    if (XL2 > 0) {
      zbuf += XL1;
      gmem += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = bc;
          }
          gmem++; 
          zbuf++;
          ZL += dZL;
        } while (--XL2);
      else do *gmem++ = bc; while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    Y0++;
  }
}

void plPF_SolidG(pl_Cam *cam, pl_PrepFace *TriFace) {
  uint8_t i0, i1, i2;
  uint8_t *gmem = cam->frameBuffer;
  uint8_t *remap = TriFace->Face->Material->_ReMapTable;
  float *zbuf = cam->zBuffer;
  float dZL=0, dZ1=0, dZ2=0, Z1, Z2, ZL, Z3;
  int32_t dX1=0, dX2=0, X1, X2, XL1, XL2;
  int32_t C1, C2, dC1=0, dC2=0, dCL=0, CL, C3;
  int32_t Y1, Y2, Y0, dY;
  uint8_t stat;
  bool zb = (zbuf&&TriFace->Face->Material->zBufferable) ? 1 : 0;

  float nc = (TriFace->Face->Material->_ColorsUsed-1)*65536.0f;
  int32_t maxColor=((TriFace->Face->Material->_ColorsUsed-1)<<16);
  int32_t maxColorNonShift=TriFace->Face->Material->_ColorsUsed-1;

  PUTFACE_SORT();

  
  C1 = (int32_t) (TriFace->Face->Shades[i0]*nc);
  C2 = (int32_t) (TriFace->Face->Shades[i1]*nc);
  C3 = (int32_t) (TriFace->Face->Shades[i2]*nc);
  X2 = X1 = TriFace->Scrx[i0];
  Z1 = TriFace->Scrz[i0];
  Z2 = TriFace->Scrz[i1];
  Z3 = TriFace->Scrz[i2];

  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dC2 = (C3 - C1) / dY;
    dZ2 = (Z3 - Z1) / dY;
  }
  dY = Y1 - Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dC1 = (C2 - C1) / dY;
    dZ1 = (Z2 - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
    Z2 = Z1;
    C2 = C1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      stat = 2|4;
    } else {
      X1 = C1; C1 = C2; C2 = X1;
      ZL = Z1; Z1 = Z2; Z2 = ZL;
      X1 = TriFace->Scrx[i1];
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
        dZ1 = (Z3-Z1)/dY;
        dC1 = (C3-C1) / dY;
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
      }
    }
    CL = C1;
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    XL2 -= XL1; 
    if (XL2 > 0) {
      gmem += XL1; 
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            if (CL >= maxColor) *gmem=remap[maxColorNonShift];
            else if (CL > 0) *gmem = remap[CL>>16];
            else *gmem = remap[0];                    
          }
          gmem++;
          zbuf++;
          ZL += dZL;
          CL += dCL;
        } while (--XL2);
      else do {
          if (CL >= maxColor) *gmem++=remap[maxColorNonShift];
          else if (CL > 0) *gmem++ = remap[CL>>16];
          else *gmem++ = remap[0];                    
          CL += dCL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    X1 += dX1;
    X2 += dX2; 
    C1 += dC1;
    Z1 += dZ1;
    Y0++;
  }
}
