/******************************************************************************
Plush Version 1.2
read_cob.c
ASCII COB Object Reader
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include <plush/plush.h>

#define PL_COB_MAX_LINELENGTH 1024

pl_Obj *plReadCOBObj(const char *fn, pl_Mat *mat) {
  FILE *fp = fopen(fn,"rt");
  long int p1,m1,p2,m2,p3,m3;
  char temp_string[PL_COB_MAX_LINELENGTH];
  float TransMatrix[4][4];
  pl_Obj *obj;
  pl_sInt32 x,i2;
  long int numVertices, numMappingVertices, numFaces, i;
  pl_sInt32 *MappingVertices = 0;
  if (!fp) return 0;

  fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  if (memcmp("Caligari",temp_string,8)) { fclose(fp); return 0; }

  do {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  } while (!feof(fp) && memcmp("Transform",temp_string,9));
  if (feof(fp)) { fclose(fp); return 0; }
  fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  sscanf(temp_string,"%f %f %f %f",
   &TransMatrix[0][0],&TransMatrix[0][1],&TransMatrix[0][2],&TransMatrix[0][3]);
  fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  sscanf(temp_string,"%f %f %f %f",
   &TransMatrix[1][0],&TransMatrix[1][1],&TransMatrix[1][2],&TransMatrix[1][3]);
  fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  sscanf(temp_string,"%f %f %f %f",
   &TransMatrix[2][0],&TransMatrix[2][1],&TransMatrix[2][2],&TransMatrix[2][3]);
  fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  sscanf(temp_string,"%f %f %f %f",
   &TransMatrix[3][0],&TransMatrix[3][1],&TransMatrix[3][2],&TransMatrix[3][3]);

  do {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  } while (!feof(fp) && memcmp("World Vertices",temp_string,12));
  if (feof(fp) ||  sscanf(temp_string,"World Vertices %ld",&numVertices) != 1)
    { fclose(fp); return 0; }

  rewind(fp);
  do {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  } while (!feof(fp) && memcmp("Texture Vertices",temp_string,16));
  if (feof(fp) ||
      sscanf(temp_string,"Texture Vertices %ld",&numMappingVertices) != 1) {
    fclose(fp); return 0;
  }

  rewind(fp);
  do {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  } while (!feof(fp) && memcmp("Faces",temp_string,5));
  if (feof(fp) || sscanf(temp_string,"Faces %ld",&numFaces) != 1) {
    fclose(fp); return 0;
  }
  for (x = numFaces; x; x--) {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
    if (feof(fp) || sscanf(temp_string+4," verts %ld",&i) != 1 || i < 3) {
      fclose(fp);
      return 0;
    }
    numFaces += i-3;
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  }
  obj = plObjCreate(numVertices,numFaces);
  if (!obj) { fclose(fp); return 0; }
  rewind(fp);
  do {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  } while (!feof(fp) && memcmp("World Vertices",temp_string,12));
  if (feof(fp)) { plObjDelete(obj); fclose(fp); return 0; }
  for (x = 0; x < numVertices; x ++) {
    float xp, yp, zp;
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
    if (feof(fp) ||
        sscanf(temp_string,"%f %f %f", &xp, &yp, &zp) != 3) {
      plObjDelete(obj); fclose(fp); return 0;
    }
    obj->Vertices[x].x = (TransMatrix[0][0]*xp+TransMatrix[0][1]*yp+
                          TransMatrix[0][2]*zp+TransMatrix[0][3]);
    obj->Vertices[x].y = (TransMatrix[1][0]*xp+TransMatrix[1][1]*yp+
                          TransMatrix[1][2]*zp+TransMatrix[1][3]);
    obj->Vertices[x].z = (TransMatrix[2][0]*xp+TransMatrix[2][1]*yp+
                          TransMatrix[2][2]*zp+TransMatrix[2][3]);
  }
  rewind(fp);
  do {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  } while (!feof(fp) && memcmp("Texture Vertices",temp_string,16));
  if (!feof(fp)) {
    MappingVertices = (pl_sInt32 *) 
      malloc(sizeof(pl_sInt32) * numMappingVertices * 2);
    if (MappingVertices) {
      for (x = 0; x < numMappingVertices; x ++) {
        float p1, p2;
        fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
        if (feof(fp) || sscanf(temp_string,"%f %f", &p1, &p2) != 2) {
          free(MappingVertices); plObjDelete(obj); fclose(fp); return 0;
        }
        MappingVertices[x*2] = (pl_sInt32) (p1*65536.0);
        MappingVertices[x*2+1] = (pl_sInt32) (p2*65536.0);
      }
    }
  } 
  rewind(fp);
  do {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
  } while (!feof(fp) && memcmp("Faces",temp_string,5));
  if (feof(fp)) { 
    if (MappingVertices) free(MappingVertices); 
    plObjDelete(obj); fclose(fp); return 0; 
  }
  for (x = 0; x < numFaces; x ++) {
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
    sscanf(temp_string+4," verts %ld",&i);
    fgets(temp_string,PL_COB_MAX_LINELENGTH,fp);
    if (i == 3) {
      if (feof(fp) || sscanf(temp_string,"<%ld,%ld> <%ld,%ld> <%ld,%ld>",
                             &p3,&m3,&p2,&m2,&p1,&m1) != 6) {
        if (MappingVertices) free(MappingVertices); 
        plObjDelete(obj); fclose(fp); return 0; 
      }
      obj->Faces[x].Vertices[0] = obj->Vertices + p1; 
      obj->Faces[x].Vertices[1] = obj->Vertices + p2; 
      obj->Faces[x].Vertices[2] = obj->Vertices + p3; 
      if (MappingVertices) {
        obj->Faces[x].MappingU[0] = MappingVertices[m1*2];
        obj->Faces[x].MappingV[0] = MappingVertices[m1*2+1];
        obj->Faces[x].MappingU[1] = MappingVertices[m2*2];
        obj->Faces[x].MappingV[1] = MappingVertices[m2*2+1];
        obj->Faces[x].MappingU[2] = MappingVertices[m3*2];
        obj->Faces[x].MappingV[2] = MappingVertices[m3*2+1];
      }
      obj->Faces[x].Material = mat;
    } else {
      long int p[16],m[16];
      if (feof(fp)) {
        if (MappingVertices) free(MappingVertices); 
        plObjDelete(obj); fclose(fp); return 0; 
      }
      sscanf(temp_string,
         "<%ld,%ld> <%ld,%ld> <%ld,%ld> <%ld,%ld> "
         "<%ld,%ld> <%ld,%ld> <%ld,%ld> <%ld,%ld> "
         "<%ld,%ld> <%ld,%ld> <%ld,%ld> <%ld,%ld> "
         "<%ld,%ld> <%ld,%ld> <%ld,%ld> <%ld,%ld> ",
          p+0,m+0,p+1,m+1,p+2,m+2,p+3,m+3,
          p+4,m+4,p+5,m+5,p+6,m+6,p+7,m+7,
          p+8,m+8,p+9,m+9,p+10,m+10,p+11,m+11,
          p+12,m+12,p+13,m+13,p+14,m+14,p+15,m+15);
      for (i2 = 1; i2 < (i-1); i2 ++) {
        obj->Faces[x].Vertices[0] = obj->Vertices + p[0]; 
        obj->Faces[x].Vertices[1] = obj->Vertices + p[i2+1]; 
        obj->Faces[x].Vertices[2] = obj->Vertices + p[i2]; 
        if (MappingVertices) {
          obj->Faces[x].MappingU[0] = MappingVertices[m[0]*2];
          obj->Faces[x].MappingV[0] = MappingVertices[m[0]*2+1];
          obj->Faces[x].MappingU[1] = MappingVertices[m[i2+1]*2];
          obj->Faces[x].MappingV[1] = MappingVertices[m[i2+1]*2+1];
          obj->Faces[x].MappingU[2] = MappingVertices[m[i2]*2];
          obj->Faces[x].MappingV[2] = MappingVertices[m[i2]*2+1];
        }
        obj->Faces[x].Material = mat;
        x++;
      }
      x--;
    }
  }
  obj->BackfaceCull = 1;
  if (MappingVertices) free(MappingVertices);
  plObjCalcNormals(obj);
  fclose(fp);
  return obj;
}
