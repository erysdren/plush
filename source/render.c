/******************************************************************************
Plush Version 1.2
render.c
Rendering code: this includes transformation, lighting, etc
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

void plClipSetFrustum(pl_Cam *cam);
void plClipRenderFace(pl_PrepFace *face);
int32_t plClipNeeded(pl_PrepFace *face);

#define MACRO_plMatrixApply(m,x,y,z,outx,outy,outz) \
      ( outx ) = ( x )*( m )[0] + ( y )*( m )[1] + ( z )*( m )[2] + ( m )[3];\
      ( outy ) = ( x )*( m )[4] + ( y )*( m )[5] + ( z )*( m )[6] + ( m )[7];\
      ( outz ) = ( x )*( m )[8] + ( y )*( m )[9] + ( z )*( m )[10] + ( m )[11]

#define MACRO_plDotProduct(x1,y1,z1,x2,y2,z2) \
      ((( x1 )*( x2 ))+(( y1 )*( y2 ))+(( z1 )*( z2 )))

#define MACRO_plNormalizeVector(x,y,z) { \
  double length; \
  length = ( x )*( x )+( y )*( y )+( z )*( z ); \
  if (length > 0.0000000001) { \
    float l = (float) plSqrt(length); \
    ( x ) /= l; \
    ( y ) /= l; \
    ( z ) /= l; \
  } \
}

uint32_t plRender_TriStats[4];

static uint32_t _numfaces;
static pl_PrepFace _faces[PL_MAX_TRIANGLES];

static uint32_t _numvertices;
static pl_PrepVertex _vertices[PL_MAX_VERTICES];

static float _cMatrix[16];
static uint32_t _numlights;
static pl_PrepLight _lights[PL_MAX_LIGHTS];
static pl_Cam *_cam;
static void _RenderObj(pl_Obj *, float *, float *);
static void _sift_down(int L, int U, int dir);
static void _hsort(pl_PrepFace *base, int nel, int dir);

void plRenderBegin(pl_Cam *Camera) {
  float tempMatrix[16];
  plMemSet(plRender_TriStats,0,sizeof(plRender_TriStats));
  _cam = Camera;
  _numlights = 0;
  _numfaces = 0;
  _numvertices = 0;
  plMatrixRotate(_cMatrix,2,-Camera->Pan);
  plMatrixRotate(tempMatrix,1,-Camera->Pitch);
  plMatrixMultiply(_cMatrix,tempMatrix);
  plMatrixRotate(tempMatrix,3,-Camera->Roll);
  plMatrixMultiply(_cMatrix,tempMatrix);
  plClipSetFrustum(_cam);
}

void plRenderLight(pl_Light *light) {
  float *pl, xp, yp, zp;
  if (light->Type == PL_LIGHT_NONE || _numlights >= PL_MAX_LIGHTS) return;
  pl = _lights[_numlights].l;
  if (light->Type == PL_LIGHT_VECTOR) {
    xp = light->Xp;
    yp = light->Yp;
    zp = light->Zp;
    MACRO_plMatrixApply(_cMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
  } else if (light->Type & PL_LIGHT_POINT) {
    xp = light->Xp-_cam->X;
    yp = light->Yp-_cam->Y;
    zp = light->Zp-_cam->Z;
    MACRO_plMatrixApply(_cMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
  }
  _lights[_numlights++].light = light;
}

static void _RenderObj(pl_Obj *obj, float *bmatrix, float *bnmatrix)
{
	uint32_t i, x, facepos;
	float nx = 0.0, ny = 0.0, nz = 0.0;
	double tmp, tmp2;
	float oMatrix[16], nMatrix[16], tempMatrix[16];

	pl_PrepVertex *vertex;
	pl_PrepFace *face;
	pl_Light *light;
	pl_Obj *child;

	if (obj->GenMatrix) {
		plMatrixRotate(nMatrix,1,obj->Xa);
		plMatrixRotate(tempMatrix,2,obj->Ya);
		plMatrixMultiply(nMatrix,tempMatrix);
		plMatrixRotate(tempMatrix,3,obj->Za);
		plMatrixMultiply(nMatrix,tempMatrix);
		plMemCpy(oMatrix,nMatrix,sizeof(float)*16);
	} else plMemCpy(nMatrix,obj->RotMatrix,sizeof(float)*16);

	if (bnmatrix) plMatrixMultiply(nMatrix,bnmatrix);

	if (obj->GenMatrix) {
		plMatrixTranslate(tempMatrix, obj->Xp, obj->Yp, obj->Zp);
		plMatrixMultiply(oMatrix,tempMatrix);
	} else plMemCpy(oMatrix,obj->Matrix,sizeof(float)*16);

	if (bmatrix) plMatrixMultiply(oMatrix,bmatrix);

	// erysdren
	child = obj->Children;
	while (child)
	{
		_RenderObj(child,oMatrix,nMatrix);
		child = child->NextSibling;
	}

	// invalid model
	if (!obj->Model)
		return;
	// invalid model data
	if (!obj->Model->NumFaces || !obj->Model->NumVertices)
		return;
	// exceeded maximum vert count
	if (_numvertices + obj->Model->NumVertices >= PL_MAX_VERTICES)
		return;
	// exceeded maximum face count
	if (_numfaces + obj->Model->NumFaces >= PL_MAX_TRIANGLES)
		return;

	plMatrixTranslate(tempMatrix, -_cam->X, -_cam->Y, -_cam->Z);
	plMatrixMultiply(oMatrix,tempMatrix);
	plMatrixMultiply(oMatrix,_cMatrix);
	plMatrixMultiply(nMatrix,_cMatrix);

	// setup vertices
	for (x = 0; x < obj->Model->NumVertices; x++)
	{
		vertex = _vertices + _numvertices + x;

		vertex->Vertex = obj->Model->Vertices + x;

		MACRO_plMatrixApply(
			oMatrix,
			vertex->Vertex->x, vertex->Vertex->y, vertex->Vertex->z,
			vertex->xformedx, vertex->xformedy, vertex->xformedz
		);

		MACRO_plMatrixApply(
			nMatrix,
			vertex->Vertex->nx, vertex->Vertex->ny, vertex->Vertex->nz,
			vertex->xformednx, vertex->xformedny, vertex->xformednz
		);
	}

	plRender_TriStats[0] += obj->Model->NumFaces;

	for (x = 0, facepos = _numfaces; x < obj->Model->NumFaces; x++)
	{
		face = _faces + facepos;

		face->Face = obj->Model->Faces + x;

		for (i = 0; i < 3; i++)
		{
			face->Vertices[i] = _vertices + _numvertices + (face->Face->Vertices[i] - obj->Model->Vertices);

			face->MappingU[i] = face->Face->MappingU[i];
			face->MappingV[i] = face->Face->MappingV[i];
			face->eMappingU[i] = face->Face->eMappingU[i];
			face->eMappingV[i] = face->Face->eMappingV[i];
		}

		if (obj->BackfaceCull || face->Face->Material->_st & PL_SHADE_FLAT)
		{
			MACRO_plMatrixApply(nMatrix, face->Face->nx, face->Face->ny, face->Face->nz, nx, ny, nz);
		}
		if (!obj->BackfaceCull || (MACRO_plDotProduct(nx,ny,nz,face->Vertices[0]->xformedx, face->Vertices[0]->xformedy,face->Vertices[0]->xformedz) < 0.0000001))
		{
			/* Is it in our area Check */
			if (!plClipNeeded(face))
				continue;

			if (face->Face->Material->_st & (PL_SHADE_FLAT|PL_SHADE_FLAT_DISTANCE))
			{
				tmp = face->Face->sLighting;
				if (face->Face->Material->_st & PL_SHADE_FLAT)
				{
					for (i = 0; i < _numlights; i ++)
					{
						tmp2 = 0.0;
						light = _lights[i].light;
						if (light->Type & PL_LIGHT_POINT_ANGLE)
						{
							double nx2 = _lights[i].l[0] - face->Vertices[0]->xformedx;
							double ny2 = _lights[i].l[1] - face->Vertices[0]->xformedy;
							double nz2 = _lights[i].l[2] - face->Vertices[0]->xformedz;
							MACRO_plNormalizeVector(nx2,ny2,nz2);
							tmp2 = MACRO_plDotProduct(nx,ny,nz,nx2,ny2,nz2)*light->Intensity;
						}
						if (light->Type & PL_LIGHT_POINT_DISTANCE)
						{
							double nx2 = _lights[i].l[0] - face->Vertices[0]->xformedx;
							double ny2 = _lights[i].l[1] - face->Vertices[0]->xformedy;
							double nz2 = _lights[i].l[2] - face->Vertices[0]->xformedz;
							if (light->Type & PL_LIGHT_POINT_ANGLE)
							{
								nx2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
								tmp2 *= plMax(0,plMin(1.0,nx2))*light->Intensity;
							}
							else
							{
								tmp2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
								tmp2 = plMax(0,plMin(1.0,tmp2))*light->Intensity;
							}
						}
						if (light->Type == PL_LIGHT_VECTOR)
							tmp2 = MACRO_plDotProduct(nx,ny,nz,_lights[i].l[0],_lights[i].l[1],_lights[i].l[2]) * light->Intensity;
						if (tmp2 > 0.0)
							tmp += tmp2;
						else if (obj->BackfaceIllumination)
							tmp -= tmp2;
					} /* End of light loop */
				} /* End of flat shading if */
				if (face->Face->Material->_st & PL_SHADE_FLAT_DISTANCE)
					tmp += 1.0-(face->Vertices[0]->xformedz+face->Vertices[1]->xformedz+face->Vertices[2]->xformedz) / (face->Face->Material->FadeDist*3.0);
				face->fShade = (float) tmp;
			}
			else
			{
				face->fShade = 0.0;
			} /* End of flatmask lighting if */
			if (face->Face->Material->_ft & PL_FILL_ENVIRONMENT)
			{
				face->eMappingU[0] = 32768 + (int32_t) (face->Vertices[0]->xformednx*32768.0);
				face->eMappingV[0] = 32768 - (int32_t) (face->Vertices[0]->xformedny*32768.0);
				face->eMappingU[1] = 32768 + (int32_t) (face->Vertices[1]->xformednx*32768.0);
				face->eMappingV[1] = 32768 - (int32_t) (face->Vertices[1]->xformedny*32768.0);
				face->eMappingU[2] = 32768 + (int32_t) (face->Vertices[2]->xformednx*32768.0);
				face->eMappingV[2] = 32768 - (int32_t) (face->Vertices[2]->xformedny*32768.0);
			}
			if (face->Face->Material->_st &(PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE))
			{
				uint8_t a;
				for (a = 0; a < 3; a ++)
				{
					tmp = face->Face->vsLighting[a];
					if (face->Face->Material->_st & PL_SHADE_GOURAUD)
					{
						for (i = 0; i < _numlights ; i++)
						{
							tmp2 = 0.0;
							light = _lights[i].light;
							if (light->Type & PL_LIGHT_POINT_ANGLE)
							{
								nx = _lights[i].l[0] - face->Vertices[a]->xformedx;
								ny = _lights[i].l[1] - face->Vertices[a]->xformedy;
								nz = _lights[i].l[2] - face->Vertices[a]->xformedz;
								MACRO_plNormalizeVector(nx,ny,nz);
								tmp2 = MACRO_plDotProduct(face->Vertices[a]->xformednx,face->Vertices[a]->xformedny,face->Vertices[a]->xformednz,nx,ny,nz) * light->Intensity;
							}
							if (light->Type & PL_LIGHT_POINT_DISTANCE) {
							double nx2 = _lights[i].l[0] - face->Vertices[a]->xformedx;
							double ny2 = _lights[i].l[1] - face->Vertices[a]->xformedy;
							double nz2 = _lights[i].l[2] - face->Vertices[a]->xformedz;
							if (light->Type & PL_LIGHT_POINT_ANGLE) {
								double t= (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
								tmp2 *= plMax(0,plMin(1.0,t))*light->Intensity;
							} else {
								tmp2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
								tmp2 = plMax(0,plMin(1.0,tmp2))*light->Intensity;
							}
							}
							if (light->Type == PL_LIGHT_VECTOR)
								tmp2 = MACRO_plDotProduct(face->Vertices[a]->xformednx,face->Vertices[a]->xformedny,face->Vertices[a]->xformednz,_lights[i].l[0],_lights[i].l[1],_lights[i].l[2]) * light->Intensity;
							if (tmp2 > 0.0)
								tmp += tmp2;
							else if (obj->BackfaceIllumination)
								tmp -= tmp2;
						} /* End of light loop */
					} /* End of gouraud shading if */
					if (face->Face->Material->_st & PL_SHADE_GOURAUD_DISTANCE)
						tmp += 1.0-face->Vertices[a]->xformedz/face->Face->Material->FadeDist;
					face->Shades[a] = (float) tmp;
				} /* End of vertex loop for */
			} /* End of gouraud shading mask if */
			face->zd = face->Vertices[0]->xformedz+face->Vertices[1]->xformedz+face->Vertices[2]->xformedz;
			facepos++;
			plRender_TriStats[1] ++;
		} /* Backface Check */
		_numfaces = facepos;
	}

	_numvertices += obj->Model->NumVertices;
}

void plRenderObj(pl_Obj *obj) {
  _RenderObj(obj,0,0);
}

void plRenderEnd(void) {
  pl_PrepFace *f;
  if (_cam->Sort > 0) _hsort(_faces,_numfaces,0);
  else if (_cam->Sort < 0) _hsort(_faces,_numfaces,1);
  f = _faces;
  while (_numfaces--) {
    if ( f->Face->Material && f->Face->Material->_PutFace)
    {
      plClipRenderFace(f);
    }
    f++;
  }
  _numfaces = 0;
  _numvertices = 0;
  _numlights = 0;
}

static pl_PrepFace *Base, tmp;

static void _hsort(pl_PrepFace *base, int nel, int dir) {
  static int i;
  Base=base-1;
  for (i=nel/2; i>0; i--) _sift_down(i,nel,dir);
  for (i=nel; i>1; ) {
    tmp = base[0]; base[0] = Base[i]; Base[i] = tmp;
    _sift_down(1,i-=1,dir);
  }
}

#define Comp(x,y) (( x ).zd < ( y ).zd ? 1 : 0)

static void _sift_down(int L, int U, int dir) {	
  static int c;
  while (1) { 
    c=L+L;
    if (c>U) break;
    if ( (c < U) && dir^Comp(Base[c+1],Base[c])) c++;
    if (dir^Comp(Base[L],Base[c])) return;
    tmp = Base[L]; Base[L] = Base[c]; Base[c] = tmp;
    L=c;
  }
}
#undef Comp
