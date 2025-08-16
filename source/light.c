/******************************************************************************
Plush Version 1.2
light.c
Light Control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include <plush/plush.h>

pl_Light *plLightSet(pl_Light *light, uint8_t mode, float x, float y,
                     float z, float intensity, float halfDist) {
  float m[16], m2[16];
  light->Type = mode;
  light->Intensity = intensity;
  light->HalfDistSquared = halfDist*halfDist;
  switch (mode) {
    case PL_LIGHT_VECTOR:
      plMatrixRotate(m,1,x);
      plMatrixRotate(m2,2,y);
      plMatrixMultiply(m,m2);
      plMatrixRotate(m2,3,z);
      plMatrixMultiply(m,m2);
      plMatrixApply(m,0.0,0.0,-1.0,&light->Xp, &light->Yp, &light->Zp);
    break;
    case PL_LIGHT_POINT_ANGLE:
    case PL_LIGHT_POINT_DISTANCE:
    case PL_LIGHT_POINT:
      light->Xp = x;
      light->Yp = y; 
      light->Zp = z;
    break;
  }
  return light;
}

pl_Light *plLightCreate(void) {
  pl_Light *l;
  l = (pl_Light *)plMalloc(sizeof(pl_Light));
  if (!l) return 0;
  plMemSet(l,0,sizeof(pl_Light));
  return (l);
}

void plLightDelete(pl_Light *l) {
  if (l) plFree(l);
}
