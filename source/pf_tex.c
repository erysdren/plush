/******************************************************************************
Plush Version 1.2
pf_tex.c
Affine Texture Mapping Rasterizers
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>
#include "putface.h"


void plPF_TexEnv(pl_Cam *cam, pl_PrepFace *TriFace) {
  uint8_t i0, i1, i2;
  uint8_t *gmem = cam->frameBuffer;
  uint8_t *remap;
  float *zbuf = cam->zBuffer;

  int32_t MappingU1, MappingU2, MappingU3;
  int32_t MappingV1, MappingV2, MappingV3;
  int32_t MappingU_AND, MappingV_AND;
  int32_t eMappingU1, eMappingU2, eMappingU3;
  int32_t eMappingV1, eMappingV2, eMappingV3;
  int32_t eMappingU_AND, eMappingV_AND;

  uint8_t *texture, *environment;
  uint8_t vshift;
  uint8_t evshift;
  uint16_t *addtable;
  pl_Texture *Texture, *Environment;
  uint8_t stat;
  bool zb = (zbuf&&TriFace->Face->Material->zBufferable) ? 1 : 0;

  int32_t U1, V1, U2, V2, dU1=0, dU2=0, dV1=0, dV2=0, dUL=0, dVL=0, UL, VL;
  int32_t eU1, eV1, eU2, eV2, edU1=0, edU2=0, edV1=0, 
            edV2=0, edUL=0, edVL=0, eUL, eVL;
  int32_t X1, X2, dX1=0, dX2=0, XL1, XL2;
  float Z1, ZL, dZ1=0, dZ2=0, dZL=0, Z2;
  int32_t Y1, Y2, Y0, dY;
  bool masked;
  uint8_t texel;

  Environment = TriFace->Face->Material->Environment;
  Texture = TriFace->Face->Material->Texture;

  if (!Texture || !Environment) return;
  masked = Texture->ClearColor >= 0 && Texture->ClearColor <= 255;
  texture = Texture->Data;
  environment = Environment->Data;
  addtable = TriFace->Face->Material->_AddTable;
  remap = TriFace->Face->Material->_ReMapTable;

  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;
  eMappingV_AND = ((1<<Environment->Height)-1)<<Environment->Width;
  eMappingU_AND = (1<<Environment->Width)-1;
  evshift = 16 - Environment->Width;

  PUTFACE_SORT_TEX();

  eMappingU1=(int32_t) (TriFace->eMappingU[i0]*Environment->uScale*TriFace->Face->Material->EnvScaling);
  eMappingV1=(int32_t) (TriFace->eMappingV[i0]*Environment->vScale*TriFace->Face->Material->EnvScaling);
  eMappingU2=(int32_t) (TriFace->eMappingU[i1]*Environment->uScale*TriFace->Face->Material->EnvScaling);
  eMappingV2=(int32_t) (TriFace->eMappingV[i1]*Environment->vScale*TriFace->Face->Material->EnvScaling);
  eMappingU3=(int32_t) (TriFace->eMappingU[i2]*Environment->uScale*TriFace->Face->Material->EnvScaling);
  eMappingV3=(int32_t) (TriFace->eMappingV[i2]*Environment->vScale*TriFace->Face->Material->EnvScaling);

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  eU1 = eU2 = eMappingU1;
  eV1 = eV2 = eMappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
    edU2 = (eMappingU3 - eU1) / dY;
    edV2 = (eMappingV3 - eV1) / dY;
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    edU1 = (eMappingU2 - eU1) / dY;
    edV1 = (eMappingV2 - eV1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
      dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
      edU2 ^= edU1; edU1 ^= edU2; edU2 ^= edU1;
      edV2 ^= edV1; edV1 ^= edV2; edV2 ^= edV1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      eU2 = eMappingU2;
      eV2 = eMappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      U1 = MappingU2;
      V1 = MappingV2;
      eU1 = eMappingU2;
      eV1 = eMappingV2;
      stat = 1|8;
    }
  } 

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    edUL = ((edU1-edU2)*dY)/XL1;
    edVL = ((edV1-edV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
  } else {  
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      edUL = (eU2-eU1)/XL1;
      edVL = (eV2-eV1)/XL1;
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
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
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
        edV1 = (eMappingV3 - eV1) / dY;
        edU1 = (eMappingU3 - eU1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    UL = U1;
    VL = V1;
    eUL = eU1;
    eVL = eV1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          texel = texture[((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND)];
          if ((*zbuf < ZL) && (!masked || (masked && texel != Texture->ClearColor))) {
            *zbuf = ZL;
            *gmem = remap[addtable[environment[
                ((eUL>>16)&eMappingU_AND)+((eVL>>evshift)&eMappingV_AND)]] + texel];
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
          eUL += edUL;
          eVL += edVL;
        } while (--XL2);
      else do {
          texel = texture[((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND)];
          if (!masked || (masked && texel != Texture->ClearColor))
            *gmem = remap[addtable[environment[
              ((eUL>>16)&eMappingU_AND)+((eVL>>evshift)&eMappingV_AND)]] + texel];
          gmem++;
          UL += dUL;
          VL += dVL;
          eUL += edUL;
          eVL += edVL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    U1 += dU1;
    V1 += dV1;
    eU1 += edU1;
    eV1 += edV1;
    Y0++;
  }
}

void plPF_TexF(pl_Cam *cam, pl_PrepFace *TriFace) {
  uint8_t i0, i1, i2;
  uint8_t *gmem = cam->frameBuffer;
  float *zbuf = cam->zBuffer;
  int32_t MappingU1, MappingU2, MappingU3;
  int32_t MappingV1, MappingV2, MappingV3;
  int32_t MappingU_AND, MappingV_AND;
  uint8_t *texture;
  uint8_t vshift;
  uint32_t bc;
  uint8_t *remap;
  pl_Texture *Texture;
  uint8_t stat;

  float Z1, ZL, dZ1=0, dZL=0, Z2, dZ2=0;
  int32_t dU1=0, dV1=0, dU2=0, dV2=0, U1, V1, U2, V2;
  int32_t dUL=0, dVL=0, UL, VL;
  int32_t X1, X2, dX1=0, dX2=0, XL1, XL2;
  int32_t Y1, Y2, Y0, dY;
  bool zb = (zbuf&&TriFace->Face->Material->zBufferable) ? 1 : 0;
  int32_t shade;
  bool masked;
  uint8_t texel;

  if (TriFace->Face->Material->Environment) Texture = TriFace->Face->Material->Environment;
  else Texture = TriFace->Face->Material->Texture;

  if (!Texture) return;
  masked = Texture->ClearColor >= 0 && Texture->ClearColor <= 255;
  remap = TriFace->Face->Material->_ReMapTable;
  if (TriFace->Face->Material->_AddTable)
  {
    shade=(int32_t)(TriFace->fShade*255.0f);
    if (shade < 0) shade=0;
    if (shade > 255) shade=255;
    bc = TriFace->Face->Material->_AddTable[shade];
  }
  else bc=0;
  texture = Texture->Data;
  vshift = 16 - Texture->Width;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
 
  if (TriFace->Face->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dV2 = (MappingV3 - V1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
      dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  } 

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
  } else {  
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
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
        dZ1 = (TriFace->Scrz[i2]-Z1) / dY;
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    UL = U1;
    VL = V1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          texel = texture[((UL >> 16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND)];
          if ((*zbuf < ZL) && (!masked || (masked && texel != Texture->ClearColor))) {
            *zbuf = ZL;
            *gmem = remap[bc + texel];
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      else do {
          texel = texture[((UL >> 16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND)];
          if (!masked || (masked && texel != Texture->ClearColor))
            *gmem = remap[bc + texel];
          gmem++;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    } 
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    X1 += dX1;
    X2 += dX2;    
    U1 += dU1; 
    V1 += dV1;
    Z1 += dZ1;
    Y0++;
  }
}

void plPF_TexG(pl_Cam *cam, pl_PrepFace *TriFace) {
  uint8_t i0, i1, i2;
  uint8_t *gmem = cam->frameBuffer;
  float *zbuf = cam->zBuffer;
  int32_t MappingU1, MappingU2, MappingU3;
  int32_t MappingV1, MappingV2, MappingV3;
  int32_t MappingU_AND, MappingV_AND;
  uint8_t *texture;
  uint8_t *remap;
  uint8_t vshift;
  uint16_t *addtable;
  pl_Texture *Texture;
  bool masked;
  uint8_t texel;

  int32_t U1, V1, U2, V2, dU1=0, dU2=0, dV1=0, dV2=0, dUL=0, dVL=0, UL, VL;
  int32_t X1, X2, dX1=0, dX2=0, XL1, XL2;
  int32_t C1, C2, dC1=0, dC2=0, CL, dCL=0;
  float Z1, ZL, dZ1=0, dZ2=0, dZL=0, Z2;
  int32_t Y1, Y2, Y0, dY;
  uint8_t stat;

  bool zb = (zbuf&&TriFace->Face->Material->zBufferable) ? 1 : 0;

  if (TriFace->Face->Material->Environment) Texture = TriFace->Face->Material->Environment;
  else Texture = TriFace->Face->Material->Texture;

  if (!Texture) return;
  masked = Texture->ClearColor >= 0 && Texture->ClearColor <= 255;
  remap = TriFace->Face->Material->_ReMapTable;
  texture = Texture->Data;
  addtable = TriFace->Face->Material->_AddTable;
  vshift = 16 - Texture->Width;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;

  if (TriFace->Face->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  C1 = C2 = TriFace->Shades[i0]*65535.0f;
  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
    dC2 = (TriFace->Shades[i2]*65535.0f - C1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dC1 = (TriFace->Shades[i1]*65535.0f - C1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
      dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
      dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      C2 = TriFace->Shades[i1]*65535.0f;
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      C1 = TriFace->Shades[i1]*65535.0f;
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  } 

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    if (zb) dZL = ((dZ1-dZ2)*dY)/XL1;
    dCL = ((dC1-dC2)*dY)/XL1;
  } else { 
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      if (zb) dZL = (Z2-Z1)/XL1;
      dCL = (C2-C1)/(XL1);
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
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
        dC1 = (TriFace->Shades[i2]*65535.0f-C1)/dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    CL = C1;
    ZL = Z1;
    UL = U1;
    VL = V1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          texel = texture[((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND)];
          if ((*zbuf < ZL) && (!masked || (masked && texel != Texture->ClearColor))) {
            int av;
            if (CL < 0) av=addtable[0];
            else if (CL > (255<<8)) av=addtable[255];
            else av=addtable[CL>>8];
            *zbuf = ZL;
            *gmem = remap[av + texel];
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          CL += dCL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      else do {
          int av;
          if (CL < 0) av=addtable[0];
          else if (CL > (255<<8)) av=addtable[255];
          else av=addtable[CL>>8];
          texel = texture[((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND)];
          if (!masked || (masked && texel != Texture->ClearColor))
            *gmem = remap[av + texel];
          gmem++;
          CL += dCL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
    U1 += dU1;
    V1 += dV1;
    Y0++;
  }
}
