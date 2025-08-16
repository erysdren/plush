/******************************************************************************
Plush Version 1.2
math.c
Math and Matrix Control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include <plush/plush.h>

void plMatrixRotate(float matrix[], uint8_t m, float Deg) {
  uint8_t m1, m2;
  double c,s;
  double d= Deg * PL_PI / 180.0;
  plMemSet(matrix,0,sizeof(float)*16);
  matrix[((m-1)<<2)+m-1] = matrix[15] = 1.0;
  m1 = (m % 3);
  m2 = ((m1+1) % 3);
  c = plCos(d); s = plSin(d);
  matrix[(m1<<2)+m1]=(float)c; matrix[(m1<<2)+m2]=(float)s;
  matrix[(m2<<2)+m2]=(float)c; matrix[(m2<<2)+m1]=(float)-s;
}

void plMatrixTranslate(float m[], float x, float y, float z) {
  plMemSet(m,0,sizeof(float)*16);
  m[0] = m[4+1] = m[8+2] = m[12+3] = 1.0;
  m[0+3] = x; m[4+3] = y; m[8+3] = z;
}

void plMatrixMultiply(float *dest, float src[]) {
  float temp[16];
  uint32_t i;
  plMemCpy(temp,dest,sizeof(float)*16);
  for (i = 0; i < 16; i += 4) {
    *dest++ = src[i+0]*temp[(0<<2)+0]+src[i+1]*temp[(1<<2)+0]+
              src[i+2]*temp[(2<<2)+0]+src[i+3]*temp[(3<<2)+0];
    *dest++ = src[i+0]*temp[(0<<2)+1]+src[i+1]*temp[(1<<2)+1]+
              src[i+2]*temp[(2<<2)+1]+src[i+3]*temp[(3<<2)+1];
    *dest++ = src[i+0]*temp[(0<<2)+2]+src[i+1]*temp[(1<<2)+2]+
              src[i+2]*temp[(2<<2)+2]+src[i+3]*temp[(3<<2)+2];
    *dest++ = src[i+0]*temp[(0<<2)+3]+src[i+1]*temp[(1<<2)+3]+
              src[i+2]*temp[(2<<2)+3]+src[i+3]*temp[(3<<2)+3];
  }
}

void plMatrixApply(float *m, float x, float y, float z,
                   float *outx, float *outy, float *outz) {
  *outx = x*m[0] + y*m[1] + z*m[2] + m[3];
  *outy	= x*m[4] + y*m[5] + z*m[6] + m[7];
  *outz = x*m[8] + y*m[9] + z*m[10] + m[11];
}

float plDotProduct(float x1, float y1, float z1,
                      float x2, float y2, float z2) {
  return ((x1*x2)+(y1*y2)+(z1*z2));
}

void plNormalizeVector(float *x, float *y, float *z) {
  double length;
  length = (*x)*(*x)+(*y)*(*y)+(*z)*(*z);
  if (length > 0.0000000001) {
    float t = (float)sqrt(length);
    *x /= t;
    *y /= t;
    *z /= t;
  } else *x = *y = *z = 0.0;
}

