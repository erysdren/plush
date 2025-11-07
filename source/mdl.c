/******************************************************************************
Plush Version 1.2
mdl.c
Model control
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

pl_Mdl *plMdlScale(pl_Mdl *mdl, float s) {
  uint32_t i = mdl->NumVertices;
  pl_Vertex *v = mdl->Vertices;
  while (i--) {
    v->x *= s; v->y *= s; v->z *= s; v++;
  }
  return mdl;
}

pl_Mdl *plMdlStretch(pl_Mdl *mdl, float x, float y, float z) {
  uint32_t i = mdl->NumVertices;
  pl_Vertex *v = mdl->Vertices;
  while (i--) {
    v->x *= x; v->y *= y; v->z *= z; v++;
  }
  return mdl;
}

pl_Mdl *plMdlTranslate(pl_Mdl *mdl, float x, float y, float z) {
  uint32_t i = mdl->NumVertices;
  pl_Vertex *v = mdl->Vertices;
  while (i--) {
    v->x += x; v->y += y; v->z += z; v++;
  }
  return mdl;
}

pl_Mdl *plMdlFlipNormals(pl_Mdl *mdl) {
  uint32_t i = mdl->NumVertices;
  pl_Vertex *v = mdl->Vertices;
  pl_Face *f = mdl->Faces;
  while (i--) {
    v->nx = - v->nx; v->ny = - v->ny; v->nz = - v->nz; v++;
  }
  i = mdl->NumFaces;
  while (i--) {
    f->nx = - f->nx; f->ny = - f->ny; f->nz = - f->nz;
    f++;
  }
  return mdl;
}

void plMdlDelete(pl_Mdl *mdl)
{
	plResDelete(mdl);
}

pl_Mdl *plMdlCreate(uint32_t nv, uint32_t nf)
{
	pl_Mdl *mdl;

	mdl = plResCreate(NULL, sizeof(pl_Mdl));

	plMemSet(mdl, 0, sizeof(pl_Mdl));

	mdl->NumVertices = nv;
	mdl->NumFaces = nf;

	if (nv)
	{
		mdl->Vertices = plResCreate(mdl, sizeof(pl_Vertex) * mdl->NumVertices);
		plMemSet(mdl->Vertices, 0, sizeof(pl_Vertex) * mdl->NumVertices);
	}

	if (nf)
	{
		mdl->Faces = plResCreate(mdl, sizeof(pl_Face) * mdl->NumFaces);
		plMemSet(mdl->Faces, 0, sizeof(pl_Face) * mdl->NumFaces);
	}

	return mdl;
}

void plMdlSetMat(pl_Mdl *mdl, pl_Mat *mat) {
  int32_t i = mdl->NumFaces;
  pl_Face *f = mdl->Faces;
  while (i--) (f++)->Material = mat;
}

void plMdlCalcNormals(pl_Mdl *mdl) {
  uint32_t i;
  pl_Vertex *v = mdl->Vertices;
  pl_Face *f = mdl->Faces;
  double x1, x2, y1, y2, z1, z2;
  i = mdl->NumVertices;
  while (i--) {
    v->nx = 0.0; v->ny = 0.0; v->nz = 0.0;
    v++;
  }
  i = mdl->NumFaces;
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
  v = mdl->Vertices;
  i = mdl->NumVertices;
  do {
    plNormalizeVector(&v->nx, &v->ny, &v->nz);
    v++;
  } while (--i);
}

void plMdlCalcBoundingBox(pl_Mdl *mdl, pl_BBox *bbox)
{
	uint32_t i;

	plMemSet(bbox, 0, sizeof(pl_BBox));

	for (i = 0; i < mdl->NumVertices; i++)
	{
		if (mdl->Vertices[i].x < bbox->Min[0])
			bbox->Min[0] = mdl->Vertices[i].x;
		if (mdl->Vertices[i].x > bbox->Max[0])
			bbox->Max[0] = mdl->Vertices[i].x;

		if (mdl->Vertices[i].y < bbox->Min[1])
			bbox->Min[1] = mdl->Vertices[i].y;
		if (mdl->Vertices[i].y > bbox->Max[1])
			bbox->Max[1] = mdl->Vertices[i].y;

		if (mdl->Vertices[i].z < bbox->Min[2])
			bbox->Min[2] = mdl->Vertices[i].z;
		if (mdl->Vertices[i].z > bbox->Max[2])
			bbox->Max[2] = mdl->Vertices[i].z;
	}
}

pl_Mdl *plMdlClone(pl_Mdl *mdl) {
  pl_Face *iff, *of;
  uint32_t i;
  pl_Mdl *out;
  if (!(out = plMdlCreate(mdl->NumVertices,mdl->NumFaces))) return 0;
  plMemCpy(out->Vertices, mdl->Vertices, sizeof(pl_Vertex) * mdl->NumVertices);
  iff = mdl->Faces;
  of = out->Faces;
  i = out->NumFaces;
  while (i--) {
    of->Vertices[0] = (pl_Vertex *)
      out->Vertices + (iff->Vertices[0] - mdl->Vertices);
    of->Vertices[1] = (pl_Vertex *)
      out->Vertices + (iff->Vertices[1] - mdl->Vertices);
    of->Vertices[2] = (pl_Vertex *)
      out->Vertices + (iff->Vertices[2] - mdl->Vertices);
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
