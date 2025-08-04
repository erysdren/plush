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

			obj->Faces[num_tris].Vertices[0] = obj->Vertices + a - 1;
			obj->Faces[num_tris].Vertices[1] = obj->Vertices + b - 1;
			obj->Faces[num_tris].Vertices[2] = obj->Vertices + c - 1;
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

// extended wavefront obj loader - unfinished, feel free to finish it :3
#if 0
bool plReadWavefrontObjEx(const char *filename, pl_Obj **objects, size_t *num_objects, pl_Mat **materials, size_t *num_materials, pl_Mat *fallback_material)
{
	FILE *objfile = NULL, *mtlfile = NULL;
	char line[256];
	uint32_t total_tris = 0, total_coords = 0, total_texcoords = 0, total_normals = 0;
	uint32_t num_tris = 0, num_coords = 0, num_texcoords = 0, num_normals = 0;
	float *coords = NULL, *texcoords = NULL, *normals = NULL;
	uint32_t *tris = NULL;
	char mtllib[256];

	objfile = fopen(filename, "r");
	if (!objfile)
		return false;

	memset(mtllib, 0, sizeof(mtllib));

	while (fgets(line, sizeof(line), objfile) != NULL)
	{
		if (strncmp(line, "mtllib ", 7) == 0)
			strncpy(mtllib, line, sizeof(mtllib));
		else if (strncmp(line, "v ", 2) == 0)
			total_coords++;
		else if (strncmp(line, "vt ", 3) == 0)
			total_texcoords++;
		else if (strncmp(line, "vn ", 3) == 0)
			total_normals++;
		else if (strncmp(line, "f ", 2) == 0)
			total_tris++;
	}

	// required
	if (!total_tris || !total_coords)
	{
		fclose(objfile);
		return false;
	}

	// allocate stuff
	coords = plMalloc(sizeof(*coords) * total_coords * 3);
	tris = plMalloc(sizeof(*tris) * total_tris * 9);
	if (total_texcoords)
		texcoords = plMalloc(sizeof(*texcoords) * total_texcoords * 2);
	if (total_normals)
		normals = plMalloc(sizeof(*normals) * total_normals * 3);

	// parse in mesh data
	rewind(objfile);
	while (fgets(line, sizeof(line), objfile) != NULL)
	{
		if (strncmp(line, "v ", 2) == 0)
		{
			float px, py, pz;
			if (sscanf(line, "v %f %f %f", &px, &py, &pz) != 3)
				continue;
			coords[num_coords * 3 + 0] = px;
			coords[num_coords * 3 + 1] = py;
			coords[num_coords * 3 + 2] = pz;
			num_coords++;
		}
		else if (strncmp(line, "vt ", 3) == 0)
		{
			float s, t;
			if (sscanf(line, "vt %f %f", &s, &t) != 2)
				continue;
			texcoords[num_texcoords * 2 + 0] = s;
			texcoords[num_texcoords * 2 + 1] = t;
			num_texcoords++;
		}
		else if (strncmp(line, "vn ", 3) == 0)
		{
			float nx, ny, nz;
			if (sscanf(line, "vn %f %f %f", &nx, &ny, &nz) != 3)
				continue;
			normals[num_normals * 3 + 0] = nx;
			normals[num_normals * 3 + 1] = ny;
			normals[num_normals * 3 + 2] = nz;
			num_normals++;
		}
		else if (strncmp(line, "f ", 2) == 0)
		{
			// hammer it
			int tri[9];
			bool success = false;
			memset(tri, 0, sizeof(tri));
			if (sscanf(line, "f %d %d %d", &tri[0], &tri[1], &tri[2]) == 3)
				success = true;
			else if (sscanf(line, "f %d/%d %d/%d %d/%d", &tri[0], &tri[3], &tri[1], &tri[4], &tri[2], &tri[5]) == 6)
				success = true;
			else if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &tri[0], &tri[3], &tri[6], &tri[1], &tri[4], &tri[7], &tri[2], &tri[5], &tri[8]) == 9)
				success = true;
			else if (sscanf(line, "f %d//%d %d//%d %d//%d", &tri[0], &tri[6], &tri[1], &tri[7], &tri[2], &tri[8]) == 6)
				success = true;

			if (success)
			{
				tris[num_tris * 9 + 0] = tri[0];
				tris[num_tris * 9 + 1] = tri[1];
				tris[num_tris * 9 + 2] = tri[2];
				tris[num_tris * 9 + 3] = tri[3];
				tris[num_tris * 9 + 4] = tri[4];
				tris[num_tris * 9 + 5] = tri[5];
				tris[num_tris * 9 + 6] = tri[6];
				tris[num_tris * 9 + 7] = tri[7];
				tris[num_tris * 9 + 8] = tri[8];
				num_tris++;
			}
		}
	}

	// parse mtllib
	if (mtllib[0])
	{
		mtlfile = fopen(mtllib, "r");
		if (mtlfile)
		{
			fclose(mtlfile);
		}
	}

	if (coords) plFree(coords);
	if (tris) plFree(tris);
	if (texcoords) plFree(texcoords);
	if (normals) plFree(normals);
	if (mtlfile) fclose(mtlfile);
	if (objfile) fclose(objfile);
	return true;
}
#endif
