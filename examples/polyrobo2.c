// polyrobo2.c: big fancy robot example in an enrichment room
// owo

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

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

typedef struct ibsp_texture {
	char name[64];
	uint32_t flags;
	uint32_t contents;
} ibsp_texture_t;

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

pl_Mdl **ProcessIBSPModels(ibsp_t *bsp, size_t *n_models, pl_Mat **materials, size_t max_materials, size_t *n_materials, pl_Mat *fallback_material)
{
	uint32_t num_models, num_materials, i, j, k, n;
	pl_Mdl **models;
	ibsp_model_t *bspmodels = (ibsp_model_t *)bsp->lumps[IBSP_LUMP_MODELS].data;
	ibsp_vertex_t *bspverts = (ibsp_vertex_t *)bsp->lumps[IBSP_LUMP_VERTICES].data;
	uint32_t *bspmeshverts = (uint32_t *)bsp->lumps[IBSP_LUMP_MESHVERTS].data;
	ibsp_face_t *bspfaces = (ibsp_face_t *)bsp->lumps[IBSP_LUMP_FACES].data;
	ibsp_texture_t *bsptextures = (ibsp_texture_t *)bsp->lumps[IBSP_LUMP_TEXTURES].data;

	num_models = bsp->lumps[IBSP_LUMP_MODELS].length / sizeof(ibsp_model_t);
	if (!num_models)
		return NULL;

	num_materials = bsp->lumps[IBSP_LUMP_TEXTURES].length / sizeof(ibsp_texture_t);
	if (!num_materials)
		return NULL;

	for (i = 0; i < num_materials; i++)
	{
		char *p;
		char name[64];

		materials[i] = plMatCreate();
		materials[i]->Ambient[0] = materials[i]->Ambient[1] = materials[i]->Ambient[2] = -128;
		materials[i]->Specular[0] = materials[i]->Specular[1] = materials[i]->Specular[2] = 64;
		materials[i]->Diffuse[0] = materials[i]->Diffuse[1] = materials[i]->Diffuse[2] = 128;

		materials[i]->NumGradients = 1000;
		materials[i]->ShadeType = PL_SHADE_GOURAUD;
		materials[i]->PerspectiveCorrect = 0;

		p = strrchr(bsptextures[i].name, '/');

		if (!p)
			p = bsptextures[i].name;
		else
			p += 1;

		snprintf(name, sizeof(name), "%s.pcx", p);

		materials[i]->Texture = plReadPCXTex(name, true, true);
	}

	models = plMalloc(sizeof(pl_Mdl *) * num_models);

	for (i = 0; i < num_models; i++)
	{
		uint32_t num_verts = 0, num_tris = 0;
		uint32_t total_verts = 0, total_tris = 0;

		/* count up number of vertices and triangles */
		for (j = bspmodels[i].first_face; j < bspmodels[i].first_face + bspmodels[i].num_faces; j++)
		{
			num_verts += bspfaces[j].num_meshverts;
			num_tris += bspfaces[j].num_meshverts / 3;
		}

		if (!num_verts || !num_tris)
			continue;

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
					models[i]->Faces[total_tris].vsLighting[n] /= 256.0f;

					models[i]->Faces[total_tris].MappingU[n] = vertex->texcoords[0][0] * 65536.0f;
					models[i]->Faces[total_tris].MappingV[n] = vertex->texcoords[0][1] * 65536.0f;

					if (materials[bspfaces[j].texture])
						models[i]->Faces[total_tris].Material = materials[bspfaces[j].texture];
					else
						models[i]->Faces[total_tris].Material = fallback_material;

					models[i]->Faces[total_tris].Vertices[n] = models[i]->Vertices + total_verts;

					total_verts++;
				}

				total_tris++;
			}
		}

		plMdlCalcNormals(models[i]);
	}

	if (n_materials) *n_materials = num_materials;
	if (n_models) *n_models = num_models;
	return models;
}

#pragma pack(push, 1)

typedef struct pcx_header {
	uint8_t manufacturer;
	uint8_t version;
	uint8_t encoding;
	uint8_t bits_per_pixel;
	uint16_t xmin, ymin, xmax, ymax;
	uint16_t hres, vres;
	uint8_t palette[48];
	uint8_t reserved;
	uint8_t color_planes;
	uint16_t bytes_per_line;
	uint16_t palette_type;
	uint16_t swidth, sheight;
	uint8_t filler[54];
} pcx_header_t;

#pragma pack(pop)

#define MAX_MATERIALS (64)

uint8_t framebuffer[W * H];
float zbuffer[W * H];
uint8_t palette[768];

static void SetupPolyRoboMaterials(pl_Mat *materials[6])
{
	int i;

	for (i = 0; i < 6; i++)
	{
		materials[i]->Ambient[0] = materials[i]->Ambient[1] = materials[i]->Ambient[2] = -128;
		materials[i]->Specular[0] = materials[i]->Specular[1] = materials[i]->Specular[2] = 64;
		materials[i]->Diffuse[0] = materials[i]->Diffuse[1] = materials[i]->Diffuse[2] = 128;
		materials[i]->NumGradients = 1000;
		materials[i]->ShadeType = PL_SHADE_GOURAUD;
		materials[i]->PerspectiveCorrect = 0;
	}

	materials[0]->Texture = plReadPCXTex("black.pcx", true, true);

	materials[1]->Texture = plReadPCXTex("black.pcx", true, true);
	materials[1]->Environment = plReadPCXTex("chrome_grayr.pcx", true, true);
	materials[1]->TexEnvMode = PL_TEXENV_MAX;

	materials[2]->Texture = plReadPCXTex("black.pcx", true, true);
	materials[2]->Environment = plReadPCXTex("chrome_steel1.pcx", true, true);
	materials[2]->TexEnvMode = PL_TEXENV_MAX;

	materials[3]->Texture = plReadPCXTex("droid_legmap.pcx", true, true);

	materials[4]->Texture = plReadPCXTex("ears.pcx", true, true);

	materials[5]->Texture = plReadPCXTex("eye.pcx", true, true);
}

static void SavePCX(const char *filename, int w, int h, int stride, uint8_t *screen, uint8_t *palette)
{
	int i, x, y;
	FILE *file;
	uint8_t *ptr;
	pcx_header_t *pcx = (pcx_header_t *)malloc(w * h * 2 + 1024);

	pcx->manufacturer = 0x0A;
	pcx->version = 5;
	pcx->encoding = 1;
	pcx->bits_per_pixel = 8;
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = w - 1;
	pcx->ymax = h - 1;
	pcx->hres = w;
	pcx->vres = h;
	memset(pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;
	pcx->bytes_per_line = w;
	pcx->palette_type = 2;
	memset(pcx->filler, 0, sizeof(pcx->filler));

	ptr = (uint8_t *)(pcx + 1);

	/* write image data */
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if ((*screen & 0xC0) != 0xC0)
			{
				*ptr++ = *screen++;
			}
			else
			{
				*ptr++ = 0xC1;
				*ptr++ = *screen++;
			}
		}

		screen += stride - w;
	}

	/* write palette */
	*ptr++ = 0x0C;
	for (i = 0; i < 768; i++)
		*ptr++ = *palette++;

	file = fopen(filename, "wb");
	fwrite(pcx, ptr - (uint8_t *)pcx,  1, file);
	fclose(file);

	free(pcx);
}

int main(int argc, char **argv)
{
	pl_Light *light, *light2, *light3;
	pl_Obj *polyrobo;
	pl_Obj *susan;
	pl_Mat *materials[MAX_MATERIALS];
	pl_Cam *camera;
	ibsp_t *bsp;
	pl_Obj **level_objects;
	pl_Mdl **level_models;
	size_t i, n;
	size_t num_materials = 1;
	size_t num_level_models = 0;
	float dist = 128;
	int sign = 1;
	int renderframe = 0;
	bool rendermode = false;
	bool showstats = false;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-render") == 0)
			rendermode = true;
		if (strcmp(argv[i], "-stats") == 0)
			showstats = true;
	}

	/* setup graphics mode */
	if (!rendermode)
		exSetGraphics();

	/* clear */
	memset(materials, 0, sizeof(materials));

	/* create fallback material */
	materials[0] = plMatCreate();
	materials[0]->NumGradients = 100;
	materials[0]->ShadeType = PL_SHADE_GOURAUD;

	/* load susan model */
	susan = plObjCreate(NULL);
	susan->Model = plReadWavefrontMdl("susan.obj", materials[0]);
	susan->Yp += 6;

	/* load polyrobo model */
	polyrobo = plObjCreate(NULL);
	polyrobo->Model = plReadWavefrontMdlEx("polyrobo.obj", &materials[1], 6, &n, materials[0]);
	if (!polyrobo->Model)
	{
		printf("failed to load 'polyrobo.obj'\n");
		goto cleanup;
	}

	/* raise polyrobo a bit */
	polyrobo->Yp += 12;
	polyrobo->Ya = 180;

	/* add polyrobo materials count */
	num_materials += n;

	/* override material setups */
	SetupPolyRoboMaterials(&materials[1]);

	/* load level */
	bsp = LoadIBSP("polyrobo.bsp");
	if (!bsp)
	{
		printf("failed to load 'polyrobo.bsp'\n");
		goto cleanup;
	}

	/* convert to plush models */
	level_models = ProcessIBSPModels(bsp, &num_level_models, &materials[num_materials], MAX_MATERIALS - num_materials, &n, materials[0]);
	if (!level_models)
	{
		printf("failed to load 'polyrobo.bsp'\n");
		goto cleanup;
	}

	/* add level materials count */
	num_materials += n;

	/* create actors for each plush model */
	level_objects = plMalloc(sizeof(pl_Obj *) * num_level_models);
	for (i = 0; i < num_level_models; i++)
	{
		level_objects[i] = plObjCreate(NULL);
		level_objects[i]->Model = level_models[i];
	}

	/* initialize materials */
	for (i = 0; i < num_materials; i++)
		plMatInit(materials[i]);

	/* create palette */
	plMatMakeOptPal(palette, 2, 255, materials, num_materials);
	palette[0] = palette[1] = palette[2] = 0;
	palette[3] = palette[4] = palette[5] = 255;
	for (i = 0; i < num_materials; i++)
		plMatMapToPal(materials[i], palette, 0, 255);

	if (!rendermode)
		exSetPalette(palette);

	/* create camera */
	camera = plCamCreate(W, H, W * 3.0 / (H * 4.0), 110.0, framebuffer, zbuffer);
	camera->X += 64;
	camera->Y += 64;
	camera->Z -= 384;
	camera->Pan = 15;
	camera->Pitch = 10;

	/* create lights */
	light = plLightSet(plLightCreate(), PL_LIGHT_POINT, 0.0, 64.0, -128.0, 1.0, 128.0);
	light2 = plLightSet(plLightCreate(), PL_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);
	light3 = plLightSet(plLightCreate(), PL_LIGHT_POINT, -192.0, 64.0, 32.0, 1.0, dist);

	/* main loop */
	while ((rendermode && renderframe < 360) || (!rendermode && !exGetKey()))
	{
		dist -= (64.0/180.0) * sign;
		if (dist <= 64) sign = -1;
		else if (dist >= 128) sign = 1;
		plLightSet(light3, PL_LIGHT_POINT, -192.0, 64.0, 32.0, 1.0, dist);

		/* rotate model */
		susan->Ya += 1.0;
		polyrobo->Ya += 1.0;

		/* clear back buffer */
		memset(zbuffer, 0, sizeof(zbuffer));
		memset(framebuffer, 0, sizeof(framebuffer));

		/* render frame */
		plRenderBegin(camera);
		plRenderLight(light);
		plRenderLight(light3);
		for (i = 0; i < num_level_models; i++)
			plRenderObj(level_objects[i]);
		plRenderLight(light2);
		plRenderObj(susan);
		plRenderObj(polyrobo);
		plRenderEnd();

		if (showstats)
			plTextPrintf(camera, 0, 0, 0, 1,
				"initial tris:          %u\n"
				"tris after culling:    %u\n"
				"polys after clipping:  %u\n"
				"final tesselated tris: %u\n",
				plRender_TriStats[0],
				plRender_TriStats[1],
				plRender_TriStats[2],
				plRender_TriStats[3]
			);

		/* wait for vsync, then copy to screen */
		if (!rendermode)
		{
			exWaitVSync();
			memcpy(exGraphMem, framebuffer, sizeof(framebuffer));
		}
		else
		{
			char name[64];
			snprintf(name, sizeof(name), "polyrobo%04d.pcx", renderframe);
			SavePCX(name, W, H, W, framebuffer, palette);
			printf("wrote %s\n", name);
			renderframe++;
		}
	}

	/* clean up */
cleanup:
	FreeIBSP(bsp);
	plCamDelete(camera);
	plLightDelete(light);
	plLightDelete(light2);
	plLightDelete(light3);
	for (i = 0; i < num_level_models; i++)
	{
		plMdlDelete(level_models[i]);
		plObjDelete(level_objects[i]);
	}
	plFree(level_objects);
	plFree(level_models);
	plMdlDelete(susan->Model);
	plObjDelete(susan);
	plMdlDelete(polyrobo->Model);
	plObjDelete(polyrobo);
	for (i = 0; i < num_materials; i++)
	{
		if (materials[i]->Texture) plTexDelete(materials[i]->Texture);
		if (materials[i]->Environment) plTexDelete(materials[i]->Environment);
		plMatDelete(materials[i]);
	}

	/* shut down video */
	if (!rendermode)
		exSetText();

	return 0;
}
