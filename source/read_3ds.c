/******************************************************************************
Plush Version 1.2
read_3ds.c
3DS Object Reader
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

typedef struct {
    uint16_t id;
    void (*func)(FILE *f, uint32_t p);
} _pl_3DSChunk;

static pl_Obj *obj;
static pl_Obj *bobj;
static pl_Obj *lobj;
static int16_t currentobj;
static pl_Mat *_m;

static float _pl3DSReadFloat(FILE *f);
static uint32_t _pl3DSReadDWord(FILE *f);
static uint16_t _pl3DSReadWord(FILE *f);
static void _pl3DSChunkReader(FILE *f, uint32_t p);
static void _pl3DSRGBFReader(FILE *f, uint32_t p);
static void _pl3DSRGBBReader(FILE *f, uint32_t p);
static void _pl3DSASCIIZReader(FILE *f, uint32_t p, char *as);
static void _pl3DSObjBlockReader(FILE *f, uint32_t p);
static void _pl3DSTriMeshReader(FILE *f, uint32_t p);
static void _pl3DSVertListReader(FILE *f, uint32_t p);
static void _pl3DSFaceListReader(FILE *f, uint32_t p);
static void _pl3DSFaceMatReader(FILE *f, uint32_t p);
static void MapListReader(FILE *f, uint32_t p);
static int16_t _pl3DSFindChunk(uint16_t id);

static _pl_3DSChunk _pl3DSChunkNames[] = {
    {0x4D4D,NULL}, /* Main */
    {0x3D3D,NULL}, /* Object Mesh */
    {0x4000,_pl3DSObjBlockReader},
    {0x4100,_pl3DSTriMeshReader},
    {0x4110,_pl3DSVertListReader},
    {0x4120,_pl3DSFaceListReader},
    {0x4130,_pl3DSFaceMatReader},
    {0x4140,MapListReader},
    {0xAFFF,NULL}, /* Material */
    {0xA010,NULL}, /* Ambient */
    {0xA020,NULL}, /* Diff */
    {0xA030,NULL}, /* Specular */
    {0xA200,NULL}, /* Texture */
    {0x0010,_pl3DSRGBFReader},
    {0x0011,_pl3DSRGBBReader},
};

pl_Obj *plRead3DSObj(const char *fn, pl_Mat *m) {
  FILE *f;
  uint32_t p;
  _m = m;
  obj = bobj = lobj = 0;
  currentobj = 0;
  f = fopen(fn, "rb");
  if (!f) return 0;
  fseek(f, 0, 2);
  p = ftell(f);
  rewind(f);
  _pl3DSChunkReader(f, p);
  fclose(f);
  return bobj;
}

static float _pl3DSReadFloat(FILE *f) {
  uint32_t *i;
  float c;
  i = (uint32_t *) &c;
  *i = _pl3DSReadDWord(f);
  return ((float) c);
}

static uint32_t _pl3DSReadDWord(FILE *f) {
  uint32_t r;
  r = fgetc(f);
  r |= fgetc(f)<<8;
  r |= fgetc(f)<<16;
  r |= fgetc(f)<<24;
  return r;
}

static uint16_t _pl3DSReadWord(FILE *f) {
  uint16_t r;
  r = fgetc(f);
  r |= fgetc(f)<<8;
  return r;
}

static void _pl3DSRGBFReader(FILE *f, uint32_t p) {
  float c[3];
  c[0] = _pl3DSReadFloat(f);
  c[1] = _pl3DSReadFloat(f);
  c[2] = _pl3DSReadFloat(f);
}

static void _pl3DSRGBBReader(FILE *f, uint32_t p) {
  unsigned char c[3];
  if (fread(&c, sizeof(c), 1, f) != 1) return;
}

static void _pl3DSASCIIZReader(FILE *f, uint32_t p, char *as) {
  char c;
  if (!as) while ((c = fgetc(f)) != EOF && c != '\0');
  else { 
    while ((c = fgetc(f)) != EOF && c != '\0') *as++ = c;
    *as = 0;
  }
}

static void _pl3DSObjBlockReader(FILE *f, uint32_t p) {
  _pl3DSASCIIZReader(f,p,0);
  _pl3DSChunkReader(f, p);
}

static void _pl3DSTriMeshReader(FILE *f, uint32_t p) {
  uint32_t i; 
  pl_Face *face;
  obj = plObjCreate(NULL);
  obj->Model = plMdlCreate(0,0);
  _pl3DSChunkReader(f, p);
  i = obj->Model->NumFaces;
  face = obj->Model->Faces;
  while (i--) {
    face->Vertices[0] = obj->Model->Vertices + (ptrdiff_t) face->Vertices[0];
    face->Vertices[1] = obj->Model->Vertices + (ptrdiff_t) face->Vertices[1];
    face->Vertices[2] = obj->Model->Vertices + (ptrdiff_t) face->Vertices[2];
    face->MappingU[0] = face->Vertices[0]->xformedx;
    face->MappingV[0] = face->Vertices[0]->xformedy;
    face->MappingU[1] = face->Vertices[1]->xformedx;
    face->MappingV[1] = face->Vertices[1]->xformedy;
    face->MappingU[2] = face->Vertices[2]->xformedx;
    face->MappingV[2] = face->Vertices[2]->xformedy;
    face++;
  }
  plMdlCalcNormals(obj->Model);
  if (currentobj == 0) {
    currentobj = 1;
    lobj = bobj = obj;
  } else {
    plObjAddChild(lobj, obj);
    lobj = obj;
  }
}

static void _pl3DSVertListReader(FILE *f, uint32_t p) {
  uint16_t nv;
  pl_Vertex *v;
  nv = _pl3DSReadWord(f);
  obj->Model->NumVertices = nv;
  v = obj->Model->Vertices = (pl_Vertex *) plCalloc(sizeof(pl_Vertex)*nv,1);
  while (nv--) {
    v->x = _pl3DSReadFloat(f);
    v->y = _pl3DSReadFloat(f);
    v->z = _pl3DSReadFloat(f);
    if (feof(f)) return;
    v++;
  }
}

static void _pl3DSFaceListReader(FILE *f, uint32_t p) {
  uint16_t nv;
  uint16_t c[3];
  uint16_t flags;
  pl_Face *face;

  nv = _pl3DSReadWord(f);
  obj->Model->NumFaces = nv;
  face = obj->Model->Faces = (pl_Face *) plCalloc(sizeof(pl_Face)*nv,1);
  while (nv--) {
    c[0] = _pl3DSReadWord(f);
    c[1] = _pl3DSReadWord(f);
    c[2] = _pl3DSReadWord(f);
    flags = _pl3DSReadWord(f);
    if (feof(f)) return;
    face->Vertices[0] = (pl_Vertex *) ((ptrdiff_t)c[0]);
    face->Vertices[1] = (pl_Vertex *) ((ptrdiff_t)c[1]);
    face->Vertices[2] = (pl_Vertex *) ((ptrdiff_t)c[2]);
    face->Material = _m;
    face++;
  }
  _pl3DSChunkReader(f, p);
}

static void _pl3DSFaceMatReader(FILE *f, uint32_t p) {
  uint16_t n, nf;

  _pl3DSASCIIZReader(f, p,0);

  n = _pl3DSReadWord(f);
  while (n--) {
    nf = _pl3DSReadWord(f);
  }
}

static void MapListReader(FILE *f, uint32_t p) {
  uint16_t nv;
  float c[2];
  pl_Vertex *v;
  nv = _pl3DSReadWord(f);
  v = obj->Model->Vertices;
  if (nv == obj->Model->NumVertices) while (nv--) {
    c[0] = _pl3DSReadFloat(f);
    c[1] = _pl3DSReadFloat(f);
    if (feof(f)) return;
    v->xformedx = (int32_t) (c[0]*65536.0);
    v->xformedy = (int32_t) (c[1]*65536.0);
    v++;
  }
}

static int16_t _pl3DSFindChunk(uint16_t id) {
  int16_t i;
  for (i = 0; i < sizeof(_pl3DSChunkNames)/sizeof(_pl3DSChunkNames[0]); i++)
    if (id == _pl3DSChunkNames[i].id) return i;
  return -1;
}

static void _pl3DSChunkReader(FILE *f, uint32_t p) {
  uint32_t hlen;
  uint16_t hid;
  int16_t n;
  uint32_t pc;

  while (ftell(f) < (int)p) {
    pc = ftell(f);
    hid = _pl3DSReadWord(f); if (feof(f)) return;
    hlen = _pl3DSReadDWord(f); if (feof(f)) return;
    if (hlen == 0) return;
    n = _pl3DSFindChunk(hid);
    if (n < 0) fseek(f, pc + hlen, 0);
    else {
      pc += hlen;
      if (_pl3DSChunkNames[n].func != NULL) _pl3DSChunkNames[n].func(f, pc);
      else _pl3DSChunkReader(f, pc);
      fseek(f, pc, 0);
    }
    if (ferror(f)) break;
  }
}

