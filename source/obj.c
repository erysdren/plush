/******************************************************************************
Plush Version 1.2
obj.c
Object control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include <plush/plush.h>

pl_Obj *plObjScale(pl_Obj *o, float s) {
  pl_Obj *child;
  uint32_t i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  while (i--) {
    v->x *= s; v->y *= s; v->z *= s; v++;
  }
  child = o->Children;
  while (child)
  {
    plObjScale(child,s);
    child = child->NextSibling;
  }
  return o;
}

pl_Obj *plObjStretch(pl_Obj *o, float x, float y, float z) {
  pl_Obj *child;
  uint32_t i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  while (i--) {
    v->x *= x; v->y *= y; v->z *= z; v++;
  }
  child = o->Children;
  while (child)
  {
    plObjStretch(child,x,y,z);
    child = child->NextSibling;
  }
  return o;
}

pl_Obj *plObjTranslate(pl_Obj *o, float x, float y, float z) {
  uint32_t i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  while (i--) {
    v->x += x; v->y += y; v->z += z; v++;
  }
  return o;
}

pl_Obj *plObjFlipNormals(pl_Obj *o) {
  pl_Obj *child;
  uint32_t i = o->NumVertices;
  pl_Vertex *v = o->Vertices;
  pl_Face *f = o->Faces;
  while (i--) {
    v->nx = - v->nx; v->ny = - v->ny; v->nz = - v->nz; v++;
  } 
  i = o->NumFaces;
  while (i--) {
    f->nx = - f->nx; f->ny = - f->ny; f->nz = - f->nz;
    f++;
  }
  child = o->Children;
  while (child)
  {
    plObjFlipNormals(child);
    child = child->NextSibling;
  }
  return o;
}

void plObjDelete(pl_Obj *o) {
  pl_Obj *child;
  uint32_t i;
  if (o) {
    child = o->Children;
    while (child)
    {
      plObjDelete(child);
      child = child->NextSibling;
    }
    if (o->Vertices) plFree(o->Vertices);
    if (o->Faces) plFree(o->Faces);
    plFree(o);
  }
}

pl_Obj *plObjCreate(uint32_t nv, uint32_t nf) {
  pl_Obj *o;
  if (!(o = (pl_Obj *) plMalloc(sizeof(pl_Obj)))) return 0;
  memset(o,0,sizeof(pl_Obj));
  o->GenMatrix = 1;
  o->BackfaceCull = 1;
  o->NumVertices = nv;
  o->NumFaces = nf;
  if (nv && !(o->Vertices=(pl_Vertex *) plMalloc(sizeof(pl_Vertex)*nv))) {
    plFree(o);
    return 0;
  }
  if (nf && !(o->Faces = (pl_Face *) plMalloc(sizeof(pl_Face)*nf))) {
    plFree(o->Vertices);
    plFree(o); 
    return 0;
  }
  memset(o->Vertices,0,sizeof(pl_Vertex)*nv);
  memset(o->Faces,0,sizeof(pl_Face)*nf);
  o->Parent = NULL;
  o->NextSibling = NULL;
  o->Children = NULL;
  return o;
}

pl_Obj *plObjClone(pl_Obj *o) {
  pl_Face *iff, *of;
  uint32_t i;
  pl_Obj *child;
  pl_Obj *out;
  if (!(out = plObjCreate(o->NumVertices,o->NumFaces))) return 0;
  child = o->Children;
  while (child)
  {
    plObjAddChild(out, plObjClone(child));
    child = child->NextSibling;
  }
  out->Xa = o->Xa; out->Ya = o->Ya; out->Za = o->Za;
  out->Xp = o->Xp; out->Yp = o->Yp; out->Zp = o->Zp;
  out->BackfaceCull = o->BackfaceCull;
  out->BackfaceIllumination = o->BackfaceIllumination;
  out->GenMatrix = o->GenMatrix;
  memcpy(out->Vertices, o->Vertices, sizeof(pl_Vertex) * o->NumVertices);
  iff = o->Faces;
  of = out->Faces;
  i = out->NumFaces;
  while (i--) {
    of->Vertices[0] = (pl_Vertex *) 
      out->Vertices + (iff->Vertices[0] - o->Vertices);
    of->Vertices[1] = (pl_Vertex *) 
      out->Vertices + (iff->Vertices[1] - o->Vertices);
    of->Vertices[2] = (pl_Vertex *) 
      out->Vertices + (iff->Vertices[2] - o->Vertices);
    of->MappingU[0] = iff->MappingU[0];
    of->MappingV[0] = iff->MappingV[0];
    of->MappingU[1] = iff->MappingU[1];
    of->MappingV[1] = iff->MappingV[1];
    of->MappingU[2] = iff->MappingU[2];
    of->MappingV[2] = iff->MappingV[2];
    of->nx = iff->nx;
    of->ny = iff->ny;
    of->nz = iff->nz;
    of->Material = iff->Material;
    of++;
    iff++;
  }
  return out;
}

void plObjSetMat(pl_Obj *o, pl_Mat *m, bool th) {
  int32_t i = o->NumFaces;
  pl_Face *f = o->Faces;
  while (i--) (f++)->Material = m; 
  if (th) {
    pl_Obj *child = o->Children;
    while (child)
    {
      plObjSetMat(child,m,th);
      child = child->NextSibling;
    }
  }
}

void plObjCalcNormals(pl_Obj *obj) {
  pl_Obj *child;
  uint32_t i;
  pl_Vertex *v = obj->Vertices;
  pl_Face *f = obj->Faces;
  double x1, x2, y1, y2, z1, z2;
  i = obj->NumVertices;
  while (i--) {
    v->nx = 0.0; v->ny = 0.0; v->nz = 0.0;
    v++;
  }
  i = obj->NumFaces;
  while (i--) { 
    x1 = f->Vertices[0]->x-f->Vertices[1]->x;
    x2 = f->Vertices[0]->x-f->Vertices[2]->x;
    y1 = f->Vertices[0]->y-f->Vertices[1]->y;
    y2 = f->Vertices[0]->y-f->Vertices[2]->y;
    z1 = f->Vertices[0]->z-f->Vertices[1]->z;
    z2 = f->Vertices[0]->z-f->Vertices[2]->z;
    f->nx = (float) (y1*z2 - z1*y2);
    f->ny = (float) (z1*x2 - x1*z2);
    f->nz = (float) (x1*y2 - y1*x2);
    plNormalizeVector(&f->nx, &f->ny, &f->nz);
    f->Vertices[0]->nx += f->nx;
    f->Vertices[0]->ny += f->ny;
    f->Vertices[0]->nz += f->nz;
    f->Vertices[1]->nx += f->nx;
    f->Vertices[1]->ny += f->ny;
    f->Vertices[1]->nz += f->nz;
    f->Vertices[2]->nx += f->nx;
    f->Vertices[2]->ny += f->ny;
    f->Vertices[2]->nz += f->nz;
    f++;
  }
  v = obj->Vertices;
  i = obj->NumVertices;
  do {
    plNormalizeVector(&v->nx, &v->ny, &v->nz);
    v++;
  } while (--i);
  child = obj->Children;
  while (child)
  {
    plObjCalcNormals(child);
    child = child->NextSibling;
  }
}

pl_Obj *plObjAddChild(pl_Obj *parent, pl_Obj *child)
{
	plObjRemoveParent(child);
	child->Parent = parent;
	child->NextSibling = parent->Children;
	if (parent->Children)
		parent->Children->PrevSibling = child;
	parent->Children = child;
	return child;
}

pl_Obj *plObjRemoveParent(pl_Obj *o)
{
	if (o->Parent && o->Parent->Children == o)
		o->Parent->Children = NULL;

	if (o->NextSibling)
		o->NextSibling->PrevSibling = o->PrevSibling;
	if (o->PrevSibling)
		o->PrevSibling->NextSibling = o->NextSibling;
	o->Parent = NULL;
	o->PrevSibling = NULL;
	o->NextSibling = NULL;
	return o;
}
