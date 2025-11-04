/******************************************************************************
Plush Version 1.2
obj.c
Object control
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

void plObjDelete(pl_Obj *o) {
  pl_Obj *child, *next;
  if (o) {
    plObjRemoveParent(o);
    child = o->Children;
    while (child)
    {
      next = child->NextSibling;
      plObjDelete(child);
      child = next;
    }
    plResDelete(o);
  }
}

pl_Obj *plObjCreate(pl_Obj *parent) {
  pl_Obj *o = plResCreate(NULL, sizeof(pl_Obj));
  plMemSet(o,0,sizeof(pl_Obj));
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
		o->Parent->Children = o->NextSibling;

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
	if (!o || !name)
		return;
	if (o->Name)
		plResDelete((void *)o->Name);
	o->Name = plResStrDup(o, name);
}

int plObjEnumerate(pl_Obj *obj, int (*func)(pl_Obj *obj, void *user), void *user)
{
	pl_Obj *child = obj->Children;
	int r = 0;

	if (!func)
		return 0;

	r = func(obj, user);
	if (r != 0)
		return r;

	while (child)
	{
		pl_Obj *next = child->NextSibling;
		r = plObjEnumerate(child, func, user);
		if (r != 0)
			return r;
		child = next;
	}

	return r;
}

static int find_callback(pl_Obj *obj, void *user)
{
	void **in = (void **)user;

	if (!obj->Name)
		return 0;

	if (plStrCmp(obj->Name, (const char *)in[0]) == 0)
	{
		*(pl_Obj **)in[1] = obj;
		return 1;
	}

	return 0;
}

pl_Obj *plObjFind(pl_Obj *obj, const char *name)
{
	void *user[2];
	pl_Obj *r = NULL;
	user[0] = (void *)name;
	user[1] = (void *)&r;
	plObjEnumerate(obj, find_callback, user);
	return r;
}
