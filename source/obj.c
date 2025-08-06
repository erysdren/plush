/******************************************************************************
Plush Version 1.2
obj.c
Object control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include <plush/plush.h>

void plObjDelete(pl_Obj *o) {
  pl_Obj *child;
  uint32_t i;
  if (o) {
    plObjRemoveParent(o);
    child = o->Children;
    while (child)
    {
      plObjDelete(child);
      child = child->NextSibling;
    }
    if (o->Model) plMdlDelete(o->Model);
    plFree(o);
  }
}

pl_Obj *plObjCreate(pl_Obj *parent) {
  pl_Obj *o;
  if (!(o = (pl_Obj *) plMalloc(sizeof(pl_Obj)))) return 0;
  memset(o,0,sizeof(pl_Obj));
  o->GenMatrix = 1;
  o->BackfaceCull = 1;
  o->Parent = NULL;
  o->NextSibling = NULL;
  o->Children = NULL;
  if (parent)
    plObjAddChild(parent, o);
  return o;
}

pl_Obj *plObjAddChild(pl_Obj *parent, pl_Obj *child)
{
	plObjRemoveParent(child);
	child->Parent = parent;
	child->NextSibling = parent->Children;
	if (parent->Children)
		parent->Children->PrevSibling = child;
	parent->Children = child;
	return child;
}

pl_Obj *plObjRemoveParent(pl_Obj *o)
{
	if (o->Parent && o->Parent->Children == o)
		o->Parent->Children = NULL;

	if (o->NextSibling)
		o->NextSibling->PrevSibling = o->PrevSibling;
	if (o->PrevSibling)
		o->PrevSibling->NextSibling = o->NextSibling;
	o->Parent = NULL;
	o->PrevSibling = NULL;
	o->NextSibling = NULL;
	return o;
}
