/******************************************************************************
  plush.h
  PLUSH 3D VERSION 1.2 MAIN HEADER
  Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
  Copyright (C) 2024-2025, erysdren (it/its)

  For more information on Plush and the latest updates, please visit
    http://www.nullsoft.com

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Justin Frankel
  justin@nullsoft.com

******************************************************************************/

#ifndef _PLUSH_H_
#define _PLUSH_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pl_conf.h"
#include "pl_defs.h"
#include "pl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char plVersionString[];      /* Version string */
extern char plCopyrightString[];    /* Copyright string */
extern uint8_t plText_DefaultFont[256*16]; /* Default 8x16 font for plText* */
extern uint32_t plRender_TriStats[4]; /* Three different triangle counts from 
                                          the last plRender() block:
                                          0: initial tris
                                          1: tris after culling
                                          2: final polys after real clipping
                                          3: final tris after tesselation
                                       */

/******************************************************************************
** Freestanding Functions (plush.c)
******************************************************************************/

#if PL_FREESTANDING
void *plMemSet(void *ptr, int val, size_t sz);
void *plMemCpy(void *dst, void *src, size_t sz);
int plMemCmp(void *a, void *b, size_t n);
size_t plStrLen(const char *s);
int plStrCmp(const char *a, const char *b);
char *plStrCpy(char *dst, const char *src);
char *plStrNCpy(char *dst, const char *src, size_t n);
#endif

/******************************************************************************
** Memory Functions (plush.c)
******************************************************************************/

void *plMalloc(size_t sz);
void plFree(void *ptr);
void *plRealloc(void *ptr, size_t sz);
void *plCalloc(size_t n, size_t sz);

void plAllocatorSet(pl_Alloc func, void *user);
pl_Alloc plAllocatorGet(void **user);

/******************************************************************************
** Material Functions (mat.c)
******************************************************************************/

/*
  plMatCreate() creates a material.
  Parameters:
    none
  Returns:
    a pointer to the material on success, 0 on failure 
*/
pl_Mat *plMatCreate(void);

/*
  plMatDelete() deletes a material that was created with plMatCreate().
  Parameters:
    m: a pointer to the material to be deleted
  Returns:
    nothing
*/
void plMatDelete(pl_Mat *m);

/*
  plMatInit() initializes a material that was created with plMatCreate().
  Parameters:
    m: a pointer to the material to be intialized
  Returns:
    nothing
  Notes: 
    you *must* do this before calling plMatMapToPal() or plMatMakeOptPal().
*/
void plMatInit(pl_Mat *m);

/*
  plMatMapToPal() maps a material that was created with plMatCreate() and 
    initialized with plMatInit() to a palette.
  Parameters:
    mat: material to map
    pal: a 768 byte array of unsigned chars, each 3 being a rgb triplet
         (0-255, *not* the cheesy vga 0-63)
    pstart: starting offset to use colors of, usually 0
    pend: ending offset to use colors of, usually 255
  Returns:
    nothing
  Notes:
    Mapping a material with > 2000 colors can take up to a second or two. 
      Be careful, and go easy on plMat.NumGradients ;)
*/
void plMatMapToPal(pl_Mat *m, uint8_t *pal, int32_t pstart, int32_t pend);


/*
  plMatMakeOptPal() makes an almost optimal palette from materials 
    created with plMatCreate() and initialized with plMatInit().
  Paramters:
    p: palette to create
    pstart: first color entry to use
    pend: last color entry to use
    materials: an array of pointers to materials to generate the palette from
    nmats: number of materials
  Returns:
    nothing
*/
void plMatMakeOptPal(uint8_t *p, int32_t pstart, 
                     int32_t pend, pl_Mat **materials, int32_t nmats);

/******************************************************************************
** Model Functions (mdl.c)
******************************************************************************/

/*
  plMdlCreate() allocates a model
  Paramters:
    np: Number of vertices in model
    nf: Number of faces in model
  Returns:
    a pointer to the model on success, 0 on failure
*/
pl_Mdl *plMdlCreate(uint32_t np, uint32_t nf);

/*
  plMdlDelete() frees a model
    that was allocated with plMdlCreate();
  Paramters:
    mdl: model to delete
  Returns:
    nothing
*/
void plMdlDelete(pl_Mdl *mdl);

/*
   plMdlCalcNormals() calculates all face and vertex normals for a model
   Paramters:
     mdl: the model
   Returns:
     nothing
*/
void plMdlCalcNormals(pl_Mdl *mdl);

/*
   plMdlCalcBoundingBox() calculates the model's bounding box from its vertices
   Paramters:
     mdl: the model
   Returns:
     nothing
*/
void plMdlCalcBoundingBox(pl_Mdl *mdl);

/*
  plMdlSetMat() sets the material of all faces in a model
  Paramters:
    mdl: the model to set the material of
    mat: the material to set it to
  Returns:
    nothing
*/
void plMdlSetMat(pl_Mdl *mdl, pl_Mat *mat);

/*
  plMdlScale() scales a model
  Paramters:
    mdl: a pointer to the model to scale
    s: the scaling factor
  Returns:
    a pointer to mdl.
  Notes: This scales it slowly, by going through each vertex and scaling it's
    position. Avoid doing this in realtime.
*/
pl_Mdl *plMdlScale(pl_Mdl *mdl, float s);

/*
  plMdlStretch() stretches a model
  Parameters:
    mdl: a pointer to the model to stretch
    x,y,z: the x y and z stretch factors
  Returns:
    a pointer to mdl.
  Notes: same as plMdlScale(). Note that the normals are preserved.
*/
pl_Mdl *plMdlStretch(pl_Mdl *mdl, float x, float y, float z);

/*
   plMdlTranslate() translates an model
   Parameters:
     mdl: a pointer to the model to translate
     x,y,z: translation in model space
   Returns:
     a pointer to mdl
   Notes: same as plMdlScale().
*/
pl_Mdl *plMdlTranslate(pl_Mdl *mdl, float x, float y, float z);

/*
  plMdlFlipNormals() flips all vertex and face normals of a model
  Parameters:
    mdl: a pointer to the model to flip normals of
  Returns:
    a pointer to mdl
  Notes:
    Not especially fast.
    A call to plMdlFlipNormals() or plMdlCalcNormals() will restore the normals
*/
pl_Mdl *plMdlFlipNormals(pl_Mdl *mdl);

/*
  plMdlClone() creates an exact but independent duplicate of a model
  Paramters:
    mdl: the model to clone
  Returns:
    a pointer to the new model on success, 0 on failure
*/
pl_Mdl *plMdlClone(pl_Mdl *mdl);

/******************************************************************************
** Object Functions (obj.c)
******************************************************************************/

/* 
  plObjCreate() allocates an object
  Paramters:
    np: Number of vertices in object
    nf: Number of faces in object
  Returns:
    a pointer to the object on success, 0 on failure
*/
pl_Obj *plObjCreate(pl_Obj *parent);

/*
  plObjDelete() frees an object and all of it's subobjects
    that was allocated with plObjCreate();
  Paramters:
    o: object to delete
  Returns: 
    nothing
*/
void plObjDelete(pl_Obj *o);

/*
  plObjAddChild() adds an object as a child of the given parent object
  Paramters:
    parent: parent object
    child: object to add as child
  Returns:
    child object
*/
pl_Obj *plObjAddChild(pl_Obj *parent, pl_Obj *child);

/*
  plObjRemoveParent() removes a given object from its parent hierarchy
  Paramters:
    o: object to remove from parent hierarchy
  Returns:
    child object
*/
pl_Obj *plObjRemoveParent(pl_Obj *o);

/*
  plObjSetName() sets the name of an object
  Paramters:
    o: object to name
    name: null terminated name string
  Returns:
    child object
*/
void plObjSetName(pl_Obj *o, const char *name);

/*
  plObjEnumerate() runs a function for every object in a tree
  Paramters:
    obj: object tree to enumerate
    func: callback function, return nonzero to stop the enumeration
    user: user pointer for callback
  Returns:
    return value of callback
*/
int plObjEnumerate(pl_Obj *obj, int (*func)(pl_Obj *obj, void *user), void *user);

/*
  plObjFind() finds an object in a tree by name
  Paramters:
    obj: object tree to search
    name: null terminated name string to search by
  Returns:
    found object or NULL
*/
pl_Obj *plObjFind(pl_Obj *obj, const char *name);

/******************************************************************************
** Frustum Clipping Functions (clip.c)
******************************************************************************/

/*
  plClipSetFrustum() sets up the clipping frustum.
  Parameters:
    cam: a camera allocated with plCamCreate().
  Returns:
    nothing
  Notes:
    Sets up the internal structures. 
    DO NOT CALL THIS ROUTINE FROM WITHIN A plRender*() block.
*/
void plClipSetFrustum(pl_Cam *cam);

/*
  plClipRenderFace() renders a face and clips it to the frustum initialized
    with plClipSetFrustum().
  Parameters:
    face: the face to render
  Returns:
    nothing
  Notes: this is used internally by plRender*(), so be careful. Kinda slow too.
*/
void plClipRenderFace(pl_Face *face);

/*
  plClipNeeded() decides whether the face is in the frustum, intersecting 
    the frustum, or completely out of the frustum craeted with 
    plClipSetFrustum().
  Parameters:
    face: the face to check
  Returns:
    0: the face is out of the frustum, no drawing necessary
    1: the face is intersecting the frustum, splitting and drawing necessary
  Notes: this is used internally by plRender*(), so be careful. Kinda slow too.
*/
int32_t plClipNeeded(pl_Face *face);

/******************************************************************************
** Light Handling Routines (light.c)
******************************************************************************/

/*
  plLightCreate() creates a new light
  Parameters:
    none
  Returns:
    a pointer to the light
*/
pl_Light *plLightCreate(void);

/*
  plLightSet() sets up a light allocated with plLightCreate()
  Parameters:
    light: the light to set up
    mode: the mode of the light (PL_LIGHT_*)
    x,y,z: either the position of the light (PL_LIGHT_POINT*) or the angle
           in degrees of the light (PL_LIGHT_VECTOR)
    intensity: the intensity of the light (0.0-1.0)
    halfDist: the distance at which PL_LIGHT_POINT_DISTANCE is 1/2 intensity
  Returns: 
    a pointer to light.
*/
pl_Light *plLightSet(pl_Light *light, uint8_t mode, float x, float y, 
                     float z, float intensity, float halfDist);

/*
  plLightDelete() frees a light allocated with plLightCreate().
  Paramters:
    l: light to delete
  Returns:
    nothing
*/
void plLightDelete(pl_Light *l);

/******************************************************************************
** Texture Handling Routines (texture.c)
******************************************************************************/

/*
  plTexCreate() creates a texture with the provided pixel and palette data
  Parameters:
    w: the width of the image data in pixels
    h: the height of the image data in pixels
    p: pointer to a tightly packed pixel array (w * h bytes in size)
    nc: number of colors in the palette
    c: palette data (nc * 3 bytes in size)
  Returns:
    a pointer to a new texture.
*/
pl_Texture *plTexCreate(uint32_t w, uint32_t h, uint8_t *p, uint32_t nc, uint8_t *c);

/*
** plTexDelete() frees all memory associated with "t"
*/
void plTexDelete(pl_Texture *t);


/******************************************************************************
** Camera Handling Routines (cam.c)
******************************************************************************/

/* 
  plCamCreate() allocates a new camera
  Parameters:
    sw: screen width
    sh: screen height
    ar: aspect ratio (usually 1.0)
    fov: field of view (usually 45-120)
    fb: pointer to framebuffer
    zb: pointer to Z buffer (or NULL)
  Returns:
    a pointer to the newly allocated camera
*/
pl_Cam *plCamCreate(uint32_t sw, uint32_t sh, float ar, float fov,
                    uint8_t *fb, float *zb);

/*
  plCamSetTarget() sets the target of a camera allocated with plCamCreate().
  Parameters:
    c: the camera to set the target of
    x,y,z: the worldspace coordinate of the target
  Returns: 
    nothing
  Notes:
    Sets the pitch and pan of the camera. Does not touch the roll.
*/
void plCamSetTarget(pl_Cam *c, float x, float y, float z);

/*
   plCamDelete() frees all memory associated with a camera excluding 
     framebuffers and Z buffers
   Paramters:
     c: camera to free
   Returns: 
     nothing
*/
void plCamDelete(pl_Cam *c);

/******************************************************************************
** Easy Rendering Interface (render.c)
******************************************************************************/

/*
 plRenderBegin() begins the rendering process.
   Parameters: 
     Camera: camera to use for rendering
   Returns: 
     nothing
   Notes: 
     Only one rendering process can occur at a time. 
     Uses plClip*(), so don't use them within or around a plRender() block.
*/
void plRenderBegin(pl_Cam *Camera);

/*
   plRenderLight() adds a light to the scene. 
   Parameters:
     light: light to add to scene
   Returns:
     nothing
   Notes: Any objects rendered before will be unaffected by this.
*/
void plRenderLight(pl_Light *light);

/* 
   plRenderObj() adds an object and all of it's subobjects to the scene.
   Parameters:
     obj: object to render
   Returns:
     nothing
   Notes: if Camera->Sort is zero, objects are rendered in the order that 
     they are added to the scene.
*/
void plRenderObj(pl_Obj *obj);

/* 
   plRenderEnd() actually does the rendering, and closes the rendering process
   Paramters: 
     none
   Returns: 
     nothing
*/
void plRenderEnd(void);

/******************************************************************************
** Object Primitives Code (make.c)
******************************************************************************/

/* 
  plMakePlane() makes a plane centered at the origin facing up the y axis.
  Parameters:
    w: width of the plane (along the x axis)
    d: depth of the plane (along the z axis)
    res: resolution of plane, i.e. subdivisions
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Mdl *plMakePlane(float w, float d, uint32_t res, pl_Mat *m);

/*
  plMakeBox() makes a box centered at the origin
  Parameters:
    w: width of the box (x axis)
    d: depth of the box (z axis)
    h: height of the box (y axis)
  Returns:
    pointer to object created.
*/
pl_Mdl *plMakeBox(float w, float d, float h, pl_Mat *m);

/* 
  plMakeCone() makes a cone centered at the origin
  Parameters:
    r: radius of the cone (x-z axis)
    h: height of the cone (y axis)
    div: division of cone (>=3)
    cap: close the big end?
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Mdl *plMakeCone(float r, float h, uint32_t div, bool cap, pl_Mat *m);

/*
  plMakeCylinder() makes a cylinder centered at the origin
  Parameters:
    r: radius of the cylinder (x-z axis)
    h: height of the cylinder (y axis)
    divr: division of of cylinder (around the circle) (>=3)
    captop: close the top
    capbottom: close the bottom
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Mdl *plMakeCylinder(float r, float h, uint32_t divr, bool captop,
                       bool capbottom, pl_Mat *m);

/*
  plMakeSphere() makes a sphere centered at the origin.
  Parameters:
    r: radius of the sphere
    divr: division of the sphere (around the y axis) (>=3)
    divh: division of the sphere (around the x,z axis) (>=3)
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Mdl *plMakeSphere(float r, uint32_t divr, uint32_t divh, pl_Mat *m);

/*
  plMakeTorus() makes a torus centered at the origin
  Parameters:
    r1: inner radius of the torus
    r2: outer radius of the torus
    divrot: division of the torus (around the y axis) (>=3)
    divrad: division of the radius of the torus (x>=3)
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Mdl *plMakeTorus(float r1, float r2, uint32_t divrot,
                    uint32_t divrad, pl_Mat *m);

/******************************************************************************
** File Readers (read_*.c)
******************************************************************************/

/* 
  plRead3DSObj() reads a 3DS object
  Parameters:
    fn: filename of object to read
    m: material to assign it
  Returns:
    pointer to object
  Notes:
    This reader organizes multiple objects like so:
      1) the first object is returned
      2) the second object is the first's first child
      3) the third object is the second's first child
      4) etc
*/
pl_Obj *plRead3DSObj(const char *fn, pl_Mat *m);

/*
  plReadCOBObj() reads an ascii .COB object
  Parameters:
    fn: filename of object to read
    mat: material to assign it
  Returns:
    pointer to object
  Notes:
    This is Caligari's ASCII object format.
    This reader doesn't handle multiple objects. It just reads the first one.
    Polygons with lots of sides are not always tesselated correctly. Just
      use the "Tesselate" button from within truespace to improve the results.
*/
pl_Mdl *plReadCOBMdl(const char *fn, pl_Mat *mat);

/*
  plReadJAWObj() reads a .JAW object.
  Parameters:
    fn: filename of object to read
    m: material to assign it
  Returns:
    pointer to object
  Notes:
    For information on the .JAW format, please see the jaw3D homepage,
      http://www.tc.umn.edu/nlhome/g346/kari0022/jaw3d/
*/
pl_Mdl *plReadJAWMdl(const char *fn, pl_Mat *m);

/*
  plReadWavefrontObj() reads a Wavefront OBJ object.
  Parameters:
    fn: filename of object to read
    m: material to assign it
  Returns:
    pointer to object
*/
pl_Mdl *plReadWavefrontMdl(const char *fn, pl_Mat *m);

/*
  plReadWavefrontMdlEx() reads a Wavefront OBJ object and its materials.
  Parameters:
    filename: filename of object to read
    materials: array of materials to read into
    max_materials: number of materials in array
    num_materials: returned number of read materials
    fallback_material: fallback material if none are read
  Returns:
    pointer to model
  Notes:
    Each material must be properly initialized after calling this function! Set
    the material's ShadeType and use plMatInit() accordingly!
*/
pl_Mdl *plReadWavefrontMdlEx(const char *filename, pl_Mat **materials, size_t max_materials, size_t *num_materials, pl_Mat *fallback_material);

/*
  plReadPCXTex() reads a 8bpp PCX texture
  Parameters:
    fn: filename of texture to read
    rescale: will rescale image if not whole log2 dimensions (USE THIS)
    optimize: will optimize colors (USE THIS TOO)
  Returns:
    pointer to texture
  Notes: 
    The PCX must be a 8bpp zSoft version 5 PCX. The texture's palette will 
      be optimized, and the texture might be scaled up so that it's dimensions
      will be a nice power of two.
*/
pl_Texture *plReadPCXTex(const char *fn, bool rescale, bool optimize);

/*
  plReadPCXTexFromMem() reads an 8bpp PCX texture from the provided buffer
  Parameters:
    buf: pcx file buffer
    len: size of buf in bytes
    rescale: will rescale image if not whole log2 dimensions (USE THIS)
    optimize: will optimize colors (USE THIS TOO)
  Returns:
    pointer to texture
  Notes:
    The PCX must be a 8bpp zSoft version 5 PCX. The texture's palette will
      be optimized, and the texture might be scaled up so that it's dimensions
      will be a nice power of two.
*/
pl_Texture *plReadPCXTexFromMem(void *buf, size_t len, bool rescale, bool optimize);

/******************************************************************************
** File Writers (write_*.c)
******************************************************************************/

/*
  plWritePCX() write an 8bpp PCX image
  Parameters:
    fn: filename of image to write
    w,h: pixel dimensions of the image
    stride: byte stride of the image
    pixels: source pixel pointer
    palette: 256*3 bytes of palette data
  Returns:
    true on success, false on failure
*/
bool plWritePCX(const char *fn, int w, int h, int stride, uint8_t *pixels, uint8_t *palette);

/******************************************************************************
** Math Code (math.c)
******************************************************************************/

/*
  plMatrixRotate() generates a rotation matrix
  Parameters:
    matrix: an array of 16 floats that is a 4x4 matrix
    m: the axis to rotate around, 1=X, 2=Y, 3=Z.
    Deg: the angle in degrees to rotate
  Returns: 
    nothing
*/
void plMatrixRotate(float matrix[], uint8_t m, float Deg);

/*
  plMatrixTranslate() generates a translation matrix
  Parameters:
    m: the matrix (see plMatrixRotate for more info)
    x,y,z: the translation coordinates
  Returns:
    nothing
*/
void plMatrixTranslate(float m[], float x, float y, float z);

/* 
  plMatrixMultiply() multiplies two matrices
  Parameters:
    dest: destination matrix will be multipled by src
    src: source matrix
  Returns:
    nothing
  Notes: 
    this is the same as dest = dest*src (since the order *does* matter);
*/
void plMatrixMultiply(float *dest, float src[]);

/*
   plMatrixApply() applies a matrix.
  Parameters:
    m: matrix to apply
    x,y,z: input coordinate
    outx,outy,outz: pointers to output coords.
  Returns:
    nothing
  Notes: 
    applies the matrix to the 3d point to produce the transformed 3d point
*/
void plMatrixApply(float *m, float x, float y, float z, 
                   float *outx, float *outy, float *outz);

/*
  plNormalizeVector() makes a vector a unit vector
  Parameters:
    x,y,z: pointers to the vector
  Returns:
    nothing
*/
void plNormalizeVector(float *x, float *y, float *z);

/*
  plDotProduct() returns the dot product of two vectors
  Parameters:
    x1,y1,z1: the first vector
    x2,y2,z2: the second vector
  Returns:
    the dot product of the two vectors
*/
float plDotProduct(float x1, float y1, float z1,
                      float x2, float y2, float z2);

/*
  plDotProductEx() returns the dot product of two vectors of arbitrary length
  Parameters:
    v1: pointer to tightly packed float array
    v2: pointer to tightly packed float array
    n: number of floats in each array
  Returns:
    the dot product of the two vectors
*/
float plDotProductEx(float *v1, float *v2, int n);

/******************************************************************************
** Spline Interpolation (spline.c)
******************************************************************************/

/*
  plSplineInit() initializes a spline
  Parameters:
    s: the spline
  Returns:
    nothing
  Notes:
    Intializes the spline. Do this once, or when you change any of the settings
*/
void plSplineInit(pl_Spline *s);

/*
  plSplineGetPoint() gets a point on the spline
  Parameters:
    s: spline
    frame: time into spline. 0.0 is start, 1.0 is second key point, etc.
    out: a pointer to an array of s->keyWidth floats that will be filled in.
  Returns:
    nothing
*/
void plSplineGetPoint(pl_Spline *s, float frame, float *out);

/******************************************************************************
** 8xX  Bitmapped Text
******************************************************************************/
/*
  plTextSetFont() sets the font to be used by the plText*() functions.
    Parameters:
      font: a pointer to a 8xX bitmapped font
      height: the height of the font (X)
    Returns: 
      nothing
*/

void plTextSetFont(uint8_t *font, uint8_t height);

/*
  plTextPutChar() puts a character to a camera
  Parameters:
    cam: The camera. If the camera has a zBuffer, it will be used.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    color: the color to make the text
    c: the character to put. Special characters such as '\n' aren't handled.
  Returns:
    nothing
*/

void plTextPutChar(pl_Cam *cam, int32_t x, int32_t y, float z,
                   uint8_t color, uint8_t c);

/*
  plTextPutString() puts an array of characters to a camera
  Parameters:
    cam: The camera. If the camera has a zBuffer, it will be used.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    color: the color to make the text
    string: 
      the characters to put. '\n' and '\t' are handled as one would expect
  Returns:
    nothing
*/
void plTextPutStr(pl_Cam *cam, int32_t x, int32_t y, float z,
                  uint8_t color, const char *string);

/*
  plTextPrintf() is printf() for graphics
  Parameters:
    cam: The camera. If the camera has a zBuffer, it will be used.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    color: the color to make the text
    format: 
      the characters to put, with printf() formatting codes.
      '\n' and '\t' are handled as one would expect
    ...: any additional parameters specified by format
  Returns:
    nothing
*/
void plTextPrintf(pl_Cam *cam, int32_t x, int32_t y, float z,
                  uint8_t color, const char *format, ...);

/******************************************************************************
** Built-in Rasterizers
******************************************************************************/

void plPF_SolidF(pl_Cam *, pl_Face *);
void plPF_SolidG(pl_Cam *, pl_Face *);
void plPF_TexF(pl_Cam *, pl_Face *);
void plPF_TexG(pl_Cam *, pl_Face *);
void plPF_TexEnv(pl_Cam *, pl_Face *);
void plPF_PTexF(pl_Cam *, pl_Face *);
void plPF_PTexG(pl_Cam *, pl_Face *);
void plPF_TransF(pl_Cam *, pl_Face *);
void plPF_TransG(pl_Cam *, pl_Face *);

#ifdef __cplusplus
}
#endif

#endif /* !_PLUSH_H_ */
