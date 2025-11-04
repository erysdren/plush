/******************************************************************************
Plush Version 1.2
putface.h
Triangle Vertex Sorting Code for pf_*.c
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#ifndef _PUTFACE_H_
#define _PUTFACE_H_

#define PUTFACE_SORT() \
  i0 = 0; i1 = 1; i2 = 2; \
  if (TriFace->Scry[0] > TriFace->Scry[1]) { \
     i0 = 1; i1 = 0; \
  } \
  if (TriFace->Scry[i0] > TriFace->Scry[2]) { \
     i2 ^= i0; i0 ^= i2; i2 ^= i0; \
  } \
  if (TriFace->Scry[i1] > TriFace->Scry[i2]) { \
     i2 ^= i1; i1 ^= i2; i2 ^= i1; \
  }


#define PUTFACE_SORT_ENV() \
  PUTFACE_SORT(); \
  MappingU1=TriFace->Face->eMappingU[i0]*Texture->uScale*\
            TriFace->Face->Material->EnvScaling;\
  MappingV1=TriFace->Face->eMappingV[i0]*Texture->vScale*\
            TriFace->Face->Material->EnvScaling;\
  MappingU2=TriFace->Face->eMappingU[i1]*Texture->uScale*\
            TriFace->Face->Material->EnvScaling;\
  MappingV2=TriFace->Face->eMappingV[i1]*Texture->vScale*\
            TriFace->Face->Material->EnvScaling;\
  MappingU3=TriFace->Face->eMappingU[i2]*Texture->uScale*\
            TriFace->Face->Material->EnvScaling;\
  MappingV3=TriFace->Face->eMappingV[i2]*Texture->vScale*\
            TriFace->Face->Material->EnvScaling;

#define PUTFACE_SORT_TEX() \
  PUTFACE_SORT(); \
  MappingU1=TriFace->Face->MappingU[i0]*Texture->uScale*\
            TriFace->Face->Material->TexScaling;\
  MappingV1=TriFace->Face->MappingV[i0]*Texture->vScale*\
            TriFace->Face->Material->TexScaling;\
  MappingU2=TriFace->Face->MappingU[i1]*Texture->uScale*\
            TriFace->Face->Material->TexScaling;\
  MappingV2=TriFace->Face->MappingV[i1]*Texture->vScale*\
            TriFace->Face->Material->TexScaling;\
  MappingU3=TriFace->Face->MappingU[i2]*Texture->uScale*\
            TriFace->Face->Material->TexScaling;\
  MappingV3=TriFace->Face->MappingV[i2]*Texture->vScale*\
            TriFace->Face->Material->TexScaling;

#endif /* !_PUTFACE_H_ */
