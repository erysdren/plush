/******************************************************************************
  pl_defs.h
  PLUSH 3D VERSION 1.2 CONSTANTS DEFINITION HEADER
  Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
  Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#ifndef _PL_DEFS_H_
#define _PL_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* pi! */
#ifdef M_PI
#define PL_PI M_PI
#else
#define PL_PI 3.14159265359
#endif

#define plMemSet(ptr, val, sz) memset(ptr, val, sz)
#define plMemCpy(dst, src, sz) memcpy(dst, src, sz)
#define plMemCmp(a, b, n) memcmp(a, b, n)
#define plStrLen(str) strlen(str)
#define plStrCmp(a, b) strcmp(a, b)
#define plStrNCmp(a, b, len) strncmp(a, b, len)
#define plStrCpy(a, b) strcpy(a, b)
#define plStrNCpy(a, b, n) strncpy(a, b, n)
#define plSin(v) sin(v)
#define plCos(v) cos(v)
#define plTan(v) tan(v)
#define plATan(v) atan(v)
#define plSqrt(v) sqrt(v)

/*
** plResDelete() return values
** 0 - no error
** 1 - not a valid resource pointer
** 2 - resource pointer was already freed
** 3 - can't free for some reason
*/
#define PL_RESOURCE_ERROR_NONE (0)
#define PL_RESOURCE_ERROR_NOT_RESOURCE (1)
#define PL_RESOURCE_ERROR_DOUBLE_FREE (2)
#define PL_RESOURCE_ERROR_CANT_FREE (3)

/*
** plResDelete() options
** 0 - delete parent and all children
** 1 - delete children and leave parent intact
** 2 - delete parent only if it has no children
*/
#define PL_RESOURCE_DELETE_ALL (0)
#define PL_RESOURCE_DELETE_CHILDREN_ONLY (1)
#define PL_RESOURCE_DELETE_PARENT_ONLY (2)

/* Utility min() and max() functions */
#define plMin(x,y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define plMax(x,y) (( ( x ) < ( y ) ? ( y ) : ( x )))

/* Utility rad/deg functions */
#define plRadToDeg(r) ( ( r ) * 180.0 / PL_PI )
#define plDegToRad(d) ( ( d ) * PL_PI / 180.0 )

/* 
** Shade modes. Used with plMat.ShadeType
** Note that (PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE) and
** (PL_SHADE_FLAT|PL_SHADE_FLAT_DISTANCE) are valid shading modes.
*/
#define PL_SHADE_NONE (1)
#define PL_SHADE_FLAT (2)
#define PL_SHADE_FLAT_DISTANCE (4)
#define PL_SHADE_GOURAUD (8)
#define PL_SHADE_GOURAUD_DISTANCE (16)

/*
** Light modes. Used with plLight.Type or plLightSet().
** Note that PL_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
** the light and the point, PL_LIGHT_POINT_DISTANCE has falloff with proportion
** to distance**2 (see plLightSet() for setting it), PL_LIGHT_POINT does both.
*/
#define PL_LIGHT_NONE (0x0)
#define PL_LIGHT_VECTOR (0x1)
#define PL_LIGHT_POINT (0x2|0x4)
#define PL_LIGHT_POINT_DISTANCE (0x2)
#define PL_LIGHT_POINT_ANGLE (0x4)

/* Used internally; PL_FILL_* are stored in plMat._st. */
#define PL_FILL_SOLID (0x0)
#define PL_FILL_TEXTURE (0x1)
#define PL_FILL_ENVIRONMENT (0x2)
#define PL_FILL_TRANSPARENT (0x4)

#define PL_TEXENV_ADD (0)
#define PL_TEXENV_MUL (1)
#define PL_TEXENV_AVG (2)
#define PL_TEXENV_TEXMINUSENV (3)
#define PL_TEXENV_ENVMINUSTEX (4)
#define PL_TEXENV_MIN (5)
#define PL_TEXENV_MAX (6)

#ifdef __cplusplus
}
#endif

#endif /* !_PL_DEFS_H_ */
