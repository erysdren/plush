/******************************************************************************
  pl_types.h
  PLUSH 3D VERSION 1.2 TYPES DEFINITION HEADER
  Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
  Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#ifndef _PL_TYPES_H_
#define _PL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 
** Texture type. Read textures with plReadPCXTex(), and assign them to
** plMat.Environment or plMat.Texture. 
*/
typedef struct _pl_Texture {
  uint8_t *Data;            /* Texture data */
  uint8_t *PaletteData;     /* Palette data (NumColors bytes) */
  uint8_t Width, Height;    /* Log2 of dimensions */
  uint32_t iWidth, iHeight;   /* Integer dimensions */
  float uScale, vScale;   /* Scaling (usually 2**Width, 2**Height) */
  uint32_t NumColors;         /* Number of colors used in texture */
} pl_Texture;

/* 
** Material type. Create materials with plMatCreate().
*/
typedef struct _pl_Face pl_Face;
typedef struct _pl_Cam pl_Cam;
typedef struct _pl_Mat {
  int32_t Ambient[3];          /* RGB of surface (0-255 is a good range) */
  int32_t Diffuse[3];          /* RGB of diffuse (0-255 is a good range) */
  int32_t Specular[3];         /* RGB of "specular" highlights (0-255) */
  uint32_t Shininess;           /* Shininess of material. 1 is dullest */
  float FadeDist;           /* For distance fading, distance at 
                                  which intensity is 0 */
  uint8_t ShadeType;          /* Shade type: PL_SHADE_* */
  uint8_t Transparent;        /* Transparency index (0 = none), 4 = alot 
                                  Note: transparencies disable textures */
  uint8_t PerspectiveCorrect; /* Correct textures every n pixels */
  pl_Texture *Texture;         /* Texture map (see pl_Texture) above */
  pl_Texture *Environment;     /* Environment map (ditto) */
  float TexScaling;         /* Texture map scaling */
  float EnvScaling;         /* Environment map scaling */
  uint8_t TexEnvMode;         /* TexEnv combining mode (PL_TEXENV_*) */
  bool zBufferable;         /* Can this material be zbuffered? */
  uint32_t NumGradients;        /* Desired number of gradients to be used */
                 /* The following are used mostly internally */
  uint32_t _ColorsUsed;         /* Number of colors actually used */
  uint8_t _st, _ft;           /* The shadetype and filltype */
  uint32_t _tsfact;             /* Translucent shading factor */
  uint16_t *_AddTable;        /* Shading/Translucent/etc table */
  uint8_t *_ReMapTable;       /* Table to remap colors to palette */
  uint8_t *_RequestedColors;  /* _ColorsUsed colors, desired colors */
  void (*_PutFace)(pl_Cam *, pl_Face *);
                               /* Function that renders the triangle with this
                                  material */
} pl_Mat;

/*
** Vertex, used within pl_Obj
*/
typedef struct _pl_Vertex {
  float x, y, z;              /* Vertex coordinate (objectspace) */
  float xformedx, xformedy, xformedz;   
                                 /* Transformed vertex 
                                    coordinate (cameraspace) */
  float nx, ny, nz;           /* Unit vertex normal (objectspace) */
  float xformednx, xformedny, xformednz; 
                                 /* Transformed unit vertex normal 
                                    (cameraspace) */
} pl_Vertex;

/*
** Face
*/
typedef struct _pl_Face {
  pl_Vertex *Vertices[3];      /* Vertices of triangle */
  float nx, ny, nz;         /* Normal of triangle (object space) */
  pl_Mat *Material;            /* Material of triangle */
  int32_t Scrx[3], Scry[3];  /* Projected screen coordinates
                                  (12.20 fixed point) */
  float Scrz[3];            /* Projected 1/Z coordinates */
  int32_t MappingU[3], MappingV[3]; 
                               /* 16.16 Texture mapping coordinates */ 
  int32_t eMappingU[3], eMappingV[3]; 
                               /* 16.16 Environment map coordinates */
  float fShade;             /* Flat intensity */
  float sLighting;          /* Face static lighting. Should usually be 0.0 */
  float Shades[3];          /* Vertex intensity */
  float vsLighting[3];      /* Vertex static lighting. Should be 0.0 */
} pl_Face;

/*
** Model
*/
typedef struct _pl_Mdl {
  uint32_t NumVertices;            /* Number of vertices */
  uint32_t NumFaces;               /* Number of faces */
  pl_Vertex *Vertices;             /* Array of vertices */
  pl_Face *Faces;                  /* Array of faces */
} pl_Mdl;

/* 
** Object 
*/
typedef struct _pl_Obj {
  char *Name;                      /* Identifier name */
  pl_Mdl *Model;                   /* Renderable mesh */
  struct _pl_Obj *Parent;          /* Parent object */
  struct _pl_Obj *PrevSibling;     /* Previous in linked list of siblings */
  struct _pl_Obj *NextSibling;     /* Next in linked list of siblings */
  struct _pl_Obj *Children;        /* First in linked list of children */
  bool BackfaceCull;               /* Are backfacing polys drawn? */
  bool BackfaceIllumination;       /* Illuminated by lights behind them? */ 
  bool GenMatrix;                  /* Generate Matrix from the following
                                         if set */
  float Xp, Yp, Zp, Xa, Ya, Za;    /* Position and rotation of object:
                                         Note: rotations are around 
                                         X then Y then Z. Measured in degrees */
  float Matrix[16];                /* Transformation matrix */
  float RotMatrix[16];             /* Rotation only matrix (for normals) */
} pl_Obj;

/*
** Spline type. See plSpline*().
*/
typedef struct _pl_Spline {
  float *keys;              /* Key data, keyWidth*numKeys */
  int32_t keyWidth;            /* Number of floats per key */
  int32_t numKeys;             /* Number of keys */
  float cont;               /* Continuity. Should be -1.0 -> 1.0 */
  float bias;               /* Bias. -1.0 -> 1.0 */
  float tens;               /* Tension. -1.0 -> 1.0 */
} pl_Spline;

/*
** Light type. See plLight*().
*/
typedef struct _pl_Light {
  uint8_t Type;               /* Type of light: PL_LIGHT_* */
  float Xp, Yp, Zp;         /* If Type=PL_LIGHT_POINT*,
                                  this is Position (PL_LIGHT_POINT_*),
                                  otherwise if PL_LIGHT_VECTOR,
                                  Unit vector */
  float Intensity;           /* Intensity. 0.0 is off, 1.0 is full */
  float HalfDistSquared;     /* Distance squared at which 
                                   PL_LIGHT_POINT_DISTANCE is 50% */
} pl_Light;

/*
** Camera Type.
*/
typedef struct _pl_Cam {
  float Fov;                  /* FOV in degrees valid range is 1-179 */
  float AspectRatio;          /* Aspect ratio (usually 1.0) */
  int8_t Sort;                 /* Sort polygons, -1 f-t-b, 1 b-t-f, 0 no */
  float ClipBack;             /* Far clipping ( < 0.0 is none) */
  int32_t ClipTop, ClipLeft;     /* Screen Clipping */
  int32_t ClipBottom, ClipRight; 
  uint32_t ScreenWidth, ScreenHeight; /* Screen dimensions */
  int32_t CenterX, CenterY;      /* Center of screen */
  float X, Y, Z;              /* Camera position in worldspace */
  float Pitch, Pan, Roll;     /* Camera angle in degrees in worldspace */
  uint8_t *frameBuffer;         /* Framebuffer (ScreenWidth*ScreenHeight) */
  float *zBuffer;           /* Z Buffer (NULL if none) */
} pl_Cam;

#ifdef __cplusplus
}
#endif

#endif /* !_PL_TYPES_H_ */
