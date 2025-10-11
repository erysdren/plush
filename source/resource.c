/******************************************************************************
Plush Version 1.2
resource.c
Resource memory management functions
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

static const size_t _plResAlign = PL_RESOURCE_ALIGNMENT;
static const size_t _plResMagic = 0xBEEFCAFE;

typedef struct _pl_Res {
	size_t Magic;
	struct _pl_Res *Parent;
	struct _pl_Res *PrevSibling;
	struct _pl_Res *NextSibling;
	struct _pl_Res *Children;
	size_t Size;
	void *User;
} pl_Res;

static pl_Res *_plResUserToResource(void *user)
{
	pl_Res *res;

	if (!user)
		return NULL;

	/* muahaha */
	res = (pl_Res *)*(void **)((uint8_t *)user - sizeof(void *));

	/* FIXME: add better error handling or reporting */
	if (res->Magic != _plResMagic)
		return NULL;
	if (res->User != user)
		return NULL;

	return res;
}

void *plResCreate(void *parent, size_t size)
{
	pl_Res *res;
	uint8_t *user;
	size_t padding, total_size, alignment;

	/* calculate alignment */
	alignment = _plResAlign;
	if (alignment < sizeof(void *))
		alignment = sizeof(void *);

	/* calculate padding */
	padding = (alignment - (size % alignment));

	/* calculate total allocation size */
	/* FIXME: check for overflows */
	total_size = size + alignment + sizeof(pl_Res) + sizeof(void *) + padding;

	/* allocate resource */
	res = plMalloc(total_size);
	if (!res)
		return NULL;

	/* align user pointer */
	user = (uint8_t *)res + sizeof(pl_Res) + sizeof(void *);
	user += alignment - (((size_t)user) % alignment);

	/* initialize the leader area to zero */
	plMemSet(res, 0, user - (uint8_t *)res);

	/* store the original pointer right before the returned value */
	*(void **)(user - sizeof(void *)) = (void *)res;

	/* setup fields */
	res->Size = size;
	res->User = (void *)user;
	res->Magic = _plResMagic;

	/* add to parent */
	res->Parent = _plResUserToResource(parent);
	if (res->Parent)
	{
		res->NextSibling = res->Parent->Children;
		if (res->Parent->Children)
			res->Parent->Children->PrevSibling = res;
		res->Parent->Children = res;
	}

	return res->User;
}

static void _plResRemoveParent(pl_Res *res)
{
	if (res->Parent && res->Parent->Children == res)
		res->Parent->Children = res->NextSibling;

	if (res->NextSibling)
		res->NextSibling->PrevSibling = res->PrevSibling;
	if (res->PrevSibling)
		res->PrevSibling->NextSibling = res->NextSibling;

	res->Parent = NULL;
	res->PrevSibling = NULL;
	res->NextSibling = NULL;
}

static void _plResDelete(pl_Res *res)
{
	pl_Res *child, *next;

	if (!res)
		return;

	/* free any children */
	child = res->Children;
	while (child)
	{
		next = child->NextSibling;
		_plResDelete(child);
		child = next;
	}

	/* remove from any parent list */
	_plResRemoveParent(res);

	/* untag memory */
	plMemSet(res, 0, sizeof(pl_Res));

	/* finally free memory */
	plFree(res);
}

void plResDelete(void *user)
{
	pl_Res *res = _plResUserToResource(user);
	_plResDelete(res);
}
