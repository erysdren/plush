/******************************************************************************
Plush Version 1.2
read_jaw.c
Jaw3D Model Reader
Copyright (c) 1996-2000, Justin Frankel
*******************************************************************************
 Notes on .JAW files:
   This is a file format created by Jawed Karim for Jaw3D 
     (http://jaw3d.home.ml.org).
          -- updated 11/6/00 - www.jawed.com
   It is very simple, and lets one easily create ones own models using only
   a text editor. The format is pretty simple:
     The first line must be "Light: (x,y,z)" where x,y, and z are the x y and
        z components of the lightsource vector (I think ;)
     A series of lines, numbered 0 to n, in the format of 
        "i: x y z", where i is the vertex number (which should be listed in 
        order, and x y and z are the coordinates of that vertex.
     A series of lines, having the format "tri a, b, c" where a b and c are 
        the vertices that the face uses. It is unclear at this time which
        way the vertices are listed (ccw or cw), so just make em consistent
        and you can always use plFlipMdlectNormals() on the loaded object.
   That is it! (I told ya it was simple).
******************************************************************************/

#include <plush/plush.h>

pl_Mdl *plReadJAWMdl(const char *filename, pl_Mat *m) {
  FILE *jawfile;
  pl_Mdl *mdl;
  uint32_t i;
  int32_t crap;
  char line[256];
  uint32_t total_points = 0, total_polys = 0;
  if ((jawfile = fopen(filename, "r")) == NULL) return 0;
  fgets(line, 256, jawfile); /* Ignores lightsource info */
  while (fgets(line, 256, jawfile) != NULL)
    if (strstr(line, ":") != NULL) total_points++;

  rewind(jawfile); fgets(line, 256, jawfile);
  while (fgets(line, 256, jawfile) != NULL) 
    if (strstr(line, "tri") != NULL) total_polys++;

  rewind(jawfile); fgets(line, 256, jawfile);
  mdl = plMdlCreate(total_points,total_polys);

  i = 0;
  while (fgets(line, 256, jawfile) != NULL) if (strstr(line, ":") != NULL) {
    float x, y, z;
    sscanf(line, "%d: %f %f %f",&crap,&x,&y,&z);
    mdl->Vertices[i].x = (float) x;
    mdl->Vertices[i].y = (float) y;
    mdl->Vertices[i].z = (float) z;
    i++;
  }
  rewind(jawfile); fgets(line, 256, jawfile);
  i = 0;
  while (fgets(line, 256, jawfile) != NULL) if (strstr(line, "tri") != NULL) {
    int a,b,c;
    sscanf(line, "tri %d, %d, %d", &a, &b, &c);
    mdl->Faces[i].Vertices[0] = mdl->Vertices + a;
    mdl->Faces[i].Vertices[1] = mdl->Vertices + c;
    mdl->Faces[i].Vertices[2] = mdl->Vertices + b;
    mdl->Faces[i].Material = m;
    i++;
  }
  fclose(jawfile);
  plMdlCalcNormals(mdl);
  return mdl;
}	
