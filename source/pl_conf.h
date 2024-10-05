/******************************************************************************
  pl_conf.h
  PLUSH 3D VERSION 1.2 CONFIGURATION HEADER
  Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#ifndef _PL_CONF_H_
#define _PL_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Maximum children per object */
#define PL_MAX_CHILDREN (16)

/* Maximum lights per scene -- if you exceed this, they will be ignored */
#define PL_MAX_LIGHTS (32)

/* Maximum number of triangles per scene -- if you exceed this, entire 
objects will be ignored. You can increase this if you need it. It takes
approximately 8*PL_MAX_TRIANGLES bytes of memory. i.e. the default of
16384 consumes 128kbytes of memory. not really a big deal, 
*/

#define PL_MAX_TRIANGLES (16384)

typedef float pl_ZBuffer;              /* z-buffer type (must be float) */
typedef float pl_Float;                /* IEEE 32 bit floating point */
typedef int32_t pl_sInt32;     /* signed 32 bit integer */
typedef uint32_t pl_uInt32;   /* unsigned 32 bit integer */
typedef int16_t pl_sInt16;    /* signed 16 bit integer */
typedef uint16_t pl_uInt16;  /* unsigned 16 bit integer */
typedef signed int pl_sInt;            /* signed optimal integer */
typedef unsigned int pl_uInt;          /* unsigned optimal integer */
typedef int pl_Bool;                   /* boolean */
typedef uint8_t pl_uChar;        /* unsigned 8 bit integer */
typedef int8_t pl_sChar;          /* signed 8 bit integer */

#ifdef __cplusplus
}
#endif

#endif /* !_PL_CONF_H_ */
