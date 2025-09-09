
#include <stdio.h>

#include <plush/plush.h>

#include "ex.h"

#define IBSP_MAGIC "IBSP"
#define IBSP_VERSION 46

enum {
	IBSP_LUMP_ENTITES = 0,
	IBSP_LUMP_TEXTURES = 1,
	IBSP_LUMP_PLANES = 2,
	IBSP_LUMP_NODES = 3,
	IBSP_LUMP_LEAFS = 4,
	IBSP_LUMP_LEAFFACES = 5,
	IBSP_LUMP_LEAFBRUSHES = 6,
	IBSP_LUMP_MODELS = 7,
	IBSP_LUMP_BRUSHES = 8,
	IBSP_LUMP_BRUSHSIDES = 9,
	IBSP_LUMP_VERTICES = 10,
	IBSP_LUMP_MESHVERTS = 11,
	IBSP_LUMP_EFFECTS = 12,
	IBSP_LUMP_FACES = 13,
	IBSP_LUMP_LIGHTMAPS = 14,
	IBSP_LUMP_LIGHTVOLS = 15,
	IBSP_LUMP_VISDATA = 16,
	IBSP_NUM_LUMPS = 17
};

typedef struct ibsp_face {
	int32_t texture;
	int32_t effect;
	int32_t type;
	int32_t first_vert;
	int32_t num_verts;
	int32_t first_meshvert;
	int32_t num_meshverts;
	int32_t lightmap;
	int32_t lightmap_start[2];
	int32_t lightmap_size[2];
	float lightmap_origin[3];
	float lightmap_texcoords[2][3];
	float normal[3];
	int32_t patch_size[2];
} ibsp_face_t;

typedef struct ibsp_model {
	float mins[3];
	float maxs[3];
	int32_t first_face;
	int32_t num_faces;
	int32_t first_brush;
	int32_t num_brushes;
} ibsp_model_t;

typedef struct ibsp_vertex {
	float position[3];
	float texcoords[2][2];
	float normal[3];
	uint8_t colour[4];
} ibsp_vertex_t;

typedef struct ibsp {
	struct ibsp_lump {
		uint32_t offset;
		uint32_t length;
		void *data;
	} lumps[IBSP_NUM_LUMPS];
} ibsp_t;

void FreeIBSP(ibsp_t *bsp)
{
	if (bsp)
	{
		int i;
		for (i = 0; i < IBSP_NUM_LUMPS; i++)
			plFree(bsp->lumps[i].data);
		plFree(bsp);
	}
}

ibsp_t *LoadIBSP(const char *filename)
{
	int i;
	char magic[4];
	uint32_t version;
	FILE *file;
	ibsp_t *bsp;

	file = fopen(filename, "rb");
	if (!file)
	{
		printf("Failed to open '%s'\n", filename);
		return NULL;
	}

	fread(magic, sizeof(magic), 1, file);

	if (plMemCmp(magic, IBSP_MAGIC, 4) != 0)
	{
		fclose(file);
		printf("'%s' does not appear to be a valid IBSP\n", filename);
		return NULL;
	}

	fread(&version, sizeof(uint32_t), 1, file);

	if (version != IBSP_VERSION)
	{
		fclose(file);
		printf("'%s' has incorrect IBSP version %d\n", filename, version);
		return NULL;
	}

	/* allocate bsp */
	bsp = plMalloc(sizeof(ibsp_t));

	/* read and allocate lump entries */
	for (i = 0; i < IBSP_NUM_LUMPS; i++)
	{
		fread(&bsp->lumps[i].offset, sizeof(uint32_t), 1, file);
		fread(&bsp->lumps[i].length, sizeof(uint32_t), 1, file);

		bsp->lumps[i].data = plMalloc(bsp->lumps[i].length);
	}

	/* read lump entries */
	for (i = 0; i < IBSP_NUM_LUMPS; i++)
	{
		fseek(file, bsp->lumps[i].offset, SEEK_SET);
		fread(bsp->lumps[i].data, bsp->lumps[i].length, 1, file);
	}

	fclose(file);

	return bsp;
}

pl_Mdl **ProcessIBSPModels(ibsp_t *bsp, uint32_t *n_models, pl_Mat *material)
{
	uint32_t num_models, i, j, k, n;
	pl_Mdl **models;
	ibsp_model_t *bspmodels = (ibsp_model_t *)bsp->lumps[IBSP_LUMP_MODELS].data;
	ibsp_vertex_t *bspverts = (ibsp_vertex_t *)bsp->lumps[IBSP_LUMP_VERTICES].data;
	uint32_t *bspmeshverts = (uint32_t *)bsp->lumps[IBSP_LUMP_MESHVERTS].data;
	ibsp_face_t *bspfaces = (ibsp_face_t *)bsp->lumps[IBSP_LUMP_FACES].data;

	num_models = bsp->lumps[IBSP_LUMP_MODELS].length / sizeof(ibsp_model_t);
	if (!num_models)
	{
		printf("Map contains no models\n");
		return NULL;
	}

	models = plMalloc(sizeof(pl_Mdl *) * num_models);

	for (i = 0; i < num_models; i++)
	{
		uint32_t num_verts = 0, num_tris = 0;
		uint32_t total_verts = 0, total_tris = 0;
		char name[16];

		/* count up number of vertices and triangles */
		for (j = bspmodels[i].first_face; j < bspmodels[i].first_face + bspmodels[i].num_faces; j++)
		{
			num_verts += bspfaces[j].num_meshverts;
			num_tris += bspfaces[j].num_meshverts / 3;
		}

		printf("model %03d: num_verts=%d num_tris=%d\n", i, num_verts, num_tris);

		/* setup model name */
		snprintf(name, sizeof(name), "*%d", i);

		/* allocate model */
		models[i] = plMdlCreate(num_verts, num_tris);

		/* read in data */
		for (j = bspmodels[i].first_face; j < bspmodels[i].first_face + bspmodels[i].num_faces; j++)
		{
			/* read in vertices and triangles */
			for (k = 0; k < bspfaces[j].num_meshverts; k += 3)
			{
				for (n = 0; n < 3; n++)
				{
					ibsp_vertex_t *vertex = bspverts + bspfaces[j].first_vert + bspmeshverts[bspfaces[j].first_meshvert + k + n];

					models[i]->Vertices[total_verts].x = vertex->position[0];
					models[i]->Vertices[total_verts].y = vertex->position[2];
					models[i]->Vertices[total_verts].z = vertex->position[1];

					models[i]->Faces[total_tris].vsLighting[n] = 0.299 * vertex->colour[0] + 0.587 * vertex->colour[1] + 0.114 * vertex->colour[2];
					models[i]->Faces[total_tris].vsLighting[n] /= 128.0f;

					models[i]->Faces[total_tris].Material = material;

					models[i]->Faces[total_tris].Vertices[n] = models[i]->Vertices + total_verts;

					total_verts++;
				}

				total_tris++;
			}

			/* printf("face %02d: num_verts=%d num_tris=%d\n", j, bspfaces[j].num_meshverts, bspfaces[j].num_meshverts / 3); */
		}

		plMdlCalcNormals(models[i]);
	}

	if (n_models) *n_models = num_models;
	return models;
}

static uint8_t framebuffer[W * H];
static float zbuffer[W * H];
static uint8_t palette[768];

int main(int argc, char **argv)
{
	uint32_t num_models = 0;
	pl_Mdl **models = NULL;
	pl_Obj **objects = NULL;
	ibsp_t *bsp = NULL;
	uint32_t i = 0;
	pl_Mat *material = NULL;
	pl_Cam *camera = NULL;

	/* print help */
	if (argc != 2)
	{
		printf("usage: q3bsp MAPNAME.bsp\n");
		return 0;
	}

	/* setup video */
	exSetGraphics();

	/* load bsp */
	bsp = LoadIBSP(argv[1]);
	if (!bsp)
	{
		printf("Failed to load '%s' as IBSP\n", argv[1]);
		return 1;
	}

	/* create material */
	material = plMatCreate();
	material->NumGradients = 100;
	material->ShadeType = PL_SHADE_GOURAUD;
	plMatInit(material);

	/* create palette */
	plMatMakeOptPal(palette, 1, 255, &material, 1);
	palette[0] = palette[1] = palette[2] = 0;

	/* map the material to the palette */
	plMatMapToPal(material, palette, 0, 255);

	/* load submodels from bsp */
	models = ProcessIBSPModels(bsp, &num_models, material);
	if (!models || !num_models)
	{
		FreeIBSP(bsp);
		printf("Failed to load any models from '%s'\n", argv[1]);
		return 1;
	}

	/* cleanup bsp now that we're done with it */
	FreeIBSP(bsp);

	/* create objects for each model */
	objects = plMalloc(sizeof(pl_Obj *) * num_models);
	for (i = 0; i < num_models; i++)
	{
		objects[i] = plObjCreate(NULL);
		objects[i]->Model = models[i];
	}

	/* set palette on screen */
	exSetPalette(palette);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 120.0, framebuffer, zbuffer);
	camera->Y = 64;

	/* main loop */
	while (!exGetKey())
	{
		camera->Pan += 0.5;

		/* clear framebuffer and zbuffer */
		memset(framebuffer, 0, sizeof(framebuffer));
		memset(zbuffer, 0, sizeof(zbuffer));

		/* render frame */
		plRenderBegin(camera);
		for (i = 0; i < num_models; i++)
			plRenderObj(objects[i]);
		plRenderEnd();

		/* wait for vsync, then copy to screen */
		exWaitVSync();
		memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
	}

	/* cleanup objects */
	for (i = 0; i < num_models; i++)
		plObjDelete(objects[i]);
	plFree(objects);

	/* cleanup models */
	for (i = 0; i < num_models; i++)
		plMdlDelete(models[i]);
	plFree(models);

	/* shut down video */
	exSetText();

	return 0;
}
