/******************************************************************************
Plush Version 1.2
read_obj.c
Wavefront OBJ Reader
Copyright (c) 2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

pl_Obj *plReadWavefrontObj(const char *fn, pl_Mat *m)
{
	FILE *file;
	pl_Obj *obj;
	char line[256];
	uint32_t total_verts = 0, total_tris = 0, total_coords = 0, total_norms = 0;
	uint32_t num_verts = 0, num_tris = 0, num_coords = 0, num_norms = 0;
	float *coords = NULL, *norms = NULL;

	file = fopen(fn, "r");
	if (!file)
		return NULL;

	while (fgets(line, sizeof(line), file) != NULL)
	{
		if (line[0] == 'v' && line[1] == ' ')
			total_verts++;
		else if (line[0] == 'v' && line[1] == 't')
			total_coords++;
		else if (line[0] == 'v' && line[1] == 'n')
			total_norms++;
		else if (line[0] == 'f' && line[1] == ' ')
			total_tris++;
	}

	if (!total_verts || !total_tris)
	{
		fclose(file);
		return NULL;
	}

	if (total_coords)
		coords = plMalloc(sizeof(*coords) * total_coords * 2);
	if (total_norms)
		norms = plMalloc(sizeof(*norms) * total_norms * 3);

	obj = plObjCreate(total_verts, total_tris);

	rewind(file);

	while (fgets(line, sizeof(line), file) != NULL)
	{
		if (line[0] == 'v')
		{
			// vertices
			if (line[1] == ' ')
			{
				float px, py, pz;
				if (sscanf(line, "v %f %f %f", &px, &py, &pz) != 3)
					continue;
				obj->Vertices[num_verts].x = px;
				obj->Vertices[num_verts].y = py;
				obj->Vertices[num_verts].z = pz;
				num_verts++;
			}
			else if (line[1] == 't')
			{
				float s, t;
				if (sscanf(line, "vt %f %f", &s, &t) != 2)
					continue;
				coords[num_coords + 0] = s;
				coords[num_coords + 1] = t;
				num_coords += 2;
			}
			else if (line[1] == 'n')
			{
				float nx, ny, nz;
				if (sscanf(line, "vn %f %f %f", &nx, &ny, &nz) != 3)
					continue;
				norms[num_norms + 0] = nx;
				norms[num_norms + 1] = ny;
				norms[num_norms + 2] = nz;
				num_norms += 3;
			}
		}
		else if (line[0] == 'f' && line[1] == ' ')
		{
			// faces
			int a, b, c;
			if (sscanf(line, "f %d %d %d", &a, &b, &c) != 3)
				continue;

			obj->Faces[num_tris].Vertices[0] = obj->Vertices + a;
			obj->Faces[num_tris].Vertices[1] = obj->Vertices + c;
			obj->Faces[num_tris].Vertices[2] = obj->Vertices + b;
			obj->Faces[num_tris].Material = m;

			num_tris++;
		}
	}

	if (coords) plFree(coords);
	if (norms) plFree(norms);

	fclose(file);
	plObjCalcNormals(obj);
	return obj;
}
