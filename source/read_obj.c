/******************************************************************************
Plush Version 1.2
read_obj.c
Wavefront OBJ Reader
Copyright (c) 2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

pl_Mdl *plReadWavefrontMdl(const char *filename, pl_Mat *material)
{
	pl_Mdl *mdl;
	bool calcnorms = false;
	fastObjMesh *mesh = fast_obj_read(filename);
	if (!mesh)
		return NULL;

	if (!mesh->position_count || !mesh->face_count)
	{
		fast_obj_destroy(mesh);
		return NULL;
	}

	mdl = plMdlCreate(mesh->position_count - 1, mesh->face_count);

	for (int i = 0; i < mesh->object_count; i++)
	{
		int idx = 0;
		for (int j = 0; j < mesh->objects[i].face_count; j++)
		{
			int fv = mesh->face_vertices[mesh->objects[i].face_offset + j];
			if (fv != 3)
				continue;

			for (int k = 0; k < fv; k++)
			{
				fastObjIndex *index = &mesh->indices[mesh->objects[i].index_offset + idx];

				mdl->Faces[mesh->objects[i].face_offset + j].Vertices[k] = mdl->Vertices + index->p - 1;

				mdl->Faces[mesh->objects[i].face_offset + j].Vertices[k]->x = mesh->positions[3 * index->p + 0];
				mdl->Faces[mesh->objects[i].face_offset + j].Vertices[k]->y = mesh->positions[3 * index->p + 1];
				mdl->Faces[mesh->objects[i].face_offset + j].Vertices[k]->z = mesh->positions[3 * index->p + 2];

				if (index->t)
				{
					mdl->Faces[mesh->objects[i].face_offset + j].MappingU[k] = mesh->texcoords[2 * index->t + 0] * 65536.0f;
					mdl->Faces[mesh->objects[i].face_offset + j].MappingV[k] = mesh->texcoords[2 * index->t + 1] * 65536.0f;
				}

				if (index->n)
				{
					mdl->Faces[mesh->objects[i].face_offset + j].Vertices[k]->nx = mesh->normals[3 * index->n + 0];
					mdl->Faces[mesh->objects[i].face_offset + j].Vertices[k]->ny = mesh->normals[3 * index->n + 1];
					mdl->Faces[mesh->objects[i].face_offset + j].Vertices[k]->nz = mesh->normals[3 * index->n + 2];
				}
				else
				{
					calcnorms = true;
				}

				idx++;
			}

			mdl->Faces[mesh->objects[i].face_offset + j].Material = material;
		}
	}

	if (calcnorms)
		plMdlCalcNormals(mdl);

	fast_obj_destroy(mesh);
	return mdl;
}

#if 0
bool plReadWavefrontMdlEx(const char *filename, pl_Mdl **models, size_t *num_models, pl_Mat **materials, size_t *num_materials, pl_Mat *fallback_material)
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
