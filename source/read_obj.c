/******************************************************************************
Plush Version 1.2
read_obj.c
Wavefront OBJ Reader
Copyright (c) 2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

static void process_group(pl_Mdl *mdl, fastObjMesh *mesh, fastObjGroup *group, pl_Mat **materials, pl_Mat *fallback_material, bool *calcnorms)
{
	int idx = 0;
	for (int j = 0; j < group->face_count; j++)
	{
		int fv = mesh->face_vertices[group->face_offset + j];
		int fm = mesh->face_materials[group->face_offset + j];
		if (fv != 3)
			continue;

		for (int k = 0; k < fv; k++)
		{
			fastObjIndex *index = &mesh->indices[group->index_offset + idx];

			mdl->Faces[group->face_offset + j].Vertices[k] = mdl->Vertices + index->p - 1;

			mdl->Faces[group->face_offset + j].Vertices[k]->x = mesh->positions[3 * index->p + 0];
			mdl->Faces[group->face_offset + j].Vertices[k]->y = mesh->positions[3 * index->p + 1];
			mdl->Faces[group->face_offset + j].Vertices[k]->z = mesh->positions[3 * index->p + 2];

			if (index->t)
			{
				mdl->Faces[group->face_offset + j].MappingU[k] = mesh->texcoords[2 * index->t + 0] * 65536.0f;
				mdl->Faces[group->face_offset + j].MappingV[k] = mesh->texcoords[2 * index->t + 1] * 65536.0f;
			}

			if (index->n)
			{
				mdl->Faces[group->face_offset + j].Vertices[k]->nx = mesh->normals[3 * index->n + 0];
				mdl->Faces[group->face_offset + j].Vertices[k]->ny = mesh->normals[3 * index->n + 1];
				mdl->Faces[group->face_offset + j].Vertices[k]->nz = mesh->normals[3 * index->n + 2];
			}
			else
			{
				if (calcnorms) *calcnorms = true;
			}

			idx++;
		}

		if (fm == 0 || mesh->materials[fm].fallback || !materials)
			mdl->Faces[group->face_offset + j].Material = fallback_material;
		else
			mdl->Faces[group->face_offset + j].Material = materials[fm];
	}
}

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
		process_group(mdl, mesh, &mesh->objects[i], NULL, material, &calcnorms);
	for (int i = 0; i < mesh->group_count; i++)
		process_group(mdl, mesh, &mesh->groups[i], NULL, material, &calcnorms);

	if (calcnorms)
		plMdlCalcNormals(mdl);

	fast_obj_destroy(mesh);
	return mdl;
}

pl_Mdl *plReadWavefrontMdlEx(const char *filename, pl_Mat **materials, size_t max_materials, size_t *num_materials, pl_Mat *fallback_material)
{
	pl_Mdl *mdl;
	bool calcnorms = false;
	fastObjMesh *mesh = fast_obj_read(filename);
	if (!mesh)
	{
		if (num_materials) *num_materials = 0;
		return NULL;
	}

	if (!mesh->position_count || !mesh->face_count)
	{
		if (num_materials) *num_materials = 0;
		fast_obj_destroy(mesh);
		return NULL;
	}

	// process materials first
	if (num_materials) *num_materials = mesh->material_count;
	for (int i = 0; i < mesh->material_count; i++)
	{
		if (i >= max_materials)
		{
			if (num_materials) *num_materials = i;
			break;
		}

		materials[i] = plMatCreate();
		for (int j = 0; j < 3; j++)
		{
			materials[i]->Ambient[j] = mesh->materials[i].Ka[j] * 255.0f;
			materials[i]->Diffuse[j] = mesh->materials[i].Kd[j] * 255.0f;
			materials[i]->Specular[j] = mesh->materials[i].Ks[j] * 255.0f;
		}
		materials[i]->Shininess = mesh->materials[i].Ns;
	}

	mdl = plMdlCreate(mesh->position_count - 1, mesh->face_count);

	for (int i = 0; i < mesh->object_count; i++)
		process_group(mdl, mesh, &mesh->objects[i], materials, fallback_material, &calcnorms);
	for (int i = 0; i < mesh->group_count; i++)
		process_group(mdl, mesh, &mesh->groups[i], materials, fallback_material, &calcnorms);

	if (calcnorms)
		plMdlCalcNormals(mdl);

	fast_obj_destroy(mesh);
	return mdl;
}
