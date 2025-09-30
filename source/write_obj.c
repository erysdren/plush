/******************************************************************************
Plush Version 1.2
write_obj.c
Wavefront OBJ Writer
Copyright (C) 2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

bool plWriteWavefrontMdl(const char *fn, pl_Mdl *mdl)
{
	uint32_t i, j;
	FILE *file = fopen(fn, "w");
	if (!file)
		return false;

	fprintf(file, "# written by Plush " PL_VERSION "\n");

	/* write vertex positions */
	for (i = 0; i < mdl->NumVertices; i++)
		fprintf(file, "v %f %f %f\n", mdl->Vertices[i].x, mdl->Vertices[i].y, mdl->Vertices[i].z);

	/* write vertex normals */
	for (i = 0; i < mdl->NumVertices; i++)
		fprintf(file, "vn %f %f %f\n", mdl->Vertices[i].nx, mdl->Vertices[i].ny, mdl->Vertices[i].nz);

	/* write texcoords */
	for (i = 0; i < mdl->NumFaces; i++)
		for (j = 0; j < 3; j++)
			fprintf(file, "vt %f %f\n", (float)mdl->Faces[i].MappingU[j] / 65536.0f, (float)mdl->Faces[i].MappingV[j] / 65536.0f);

	/* write faces */
	for (i = 0; i < mdl->NumFaces; i++)
		fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
			(int)(mdl->Faces[i].Vertices[0] - mdl->Vertices) + 1,
			(i * 3 + 0) + 1,
			(int)(mdl->Faces[i].Vertices[0] - mdl->Vertices) + 1,

			(int)(mdl->Faces[i].Vertices[1] - mdl->Vertices) + 1,
			(i * 3 + 1) + 1,
			(int)(mdl->Faces[i].Vertices[1] - mdl->Vertices) + 1,

			(int)(mdl->Faces[i].Vertices[2] - mdl->Vertices) + 1,
			(i * 3 + 2) + 1,
			(int)(mdl->Faces[i].Vertices[2] - mdl->Vertices) + 1
		);

	fclose(file);
	return true;
}
