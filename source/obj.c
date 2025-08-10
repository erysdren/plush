/******************************************************************************
Plush Version 1.2
obj.c
Object control
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include <plush/plush.h>

void plObjDelete(pl_Obj *o) {
  pl_Obj *child, *next;
  uint32_t i;
  if (o) {
    if (o->Name) plFree(o->Name);
    plObjRemoveParent(o);
    child = o->Children;
    while (child)
    {
      next = child->NextSibling;
      plObjDelete(child);
      child = next;
    }
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

void plObjSetName(pl_Obj *o, const char *name)
{
	size_t len;
	if (!o || !name)
		return;
	len = strlen(name);
	if (!len)
		return;
	if (o->Name)
		plFree(o->Name);
	o->Name = plMalloc(len + 1);
	strncpy(o->Name, name, len);
	o->Name[len] = 0;
}
