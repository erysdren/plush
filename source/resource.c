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
	struct _pl_Res *Parent;
	struct _pl_Res *PrevSibling;
	struct _pl_Res *NextSibling;
	struct _pl_Res *Children;
	size_t Free;
	size_t Size;
	void *User;
} pl_Res;

static int _plResUserToResource(void *user, pl_Res **res)
{
	size_t magic;
	pl_Res *_res;

	if (!user)
		return PL_RESOURCE_ERROR_NONE;

	/* muahaha */
	magic = *(size_t *)((uint8_t *)user - sizeof(size_t));
	if (magic != _plResMagic)
		return PL_RESOURCE_ERROR_NOT_RESOURCE;

	_res = (pl_Res *)*(void **)((uint8_t *)user - (sizeof(void *) * 2));

	if (_res->Free != 0)
		return PL_RESOURCE_ERROR_DOUBLE_FREE;

	*res = _res;

	return PL_RESOURCE_ERROR_NONE;
}

void *plResCreate(void *parent, size_t size)
{
	int err;
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
	total_size = size + alignment + sizeof(pl_Res) + sizeof(void *) + sizeof(size_t) + padding;

	/* allocate resource */
	res = plMalloc(total_size);
	if (!res)
		return NULL;

	/* align user pointer */
	user = (uint8_t *)res + sizeof(pl_Res) + sizeof(void *) + sizeof(size_t);
	user += alignment - (((size_t)user) % alignment);

	/* initialize the leader area to zero */
	plMemSet(res, 0, user - (uint8_t *)res);

	/* store a magic identifier and the original pointer right before the returned value */
	*(size_t *)(user - sizeof(size_t)) = _plResMagic;
	*(void **)(user - (sizeof(void *) * 2)) = (void *)res;

	/* setup fields */
	res->Size = size;
	res->User = (void *)user;
	res->Free = 0;
	res->Parent = NULL;
	res->Children = NULL;

	/* add to parent */
	err = _plResUserToResource(parent, &res->Parent);
	if (err != PL_RESOURCE_ERROR_NONE)
		return NULL;

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

static int _plResDelete(pl_Res *res, int mode)
{
	pl_Res *child, *next;

	/* test for null */
	if (!res)
		return PL_RESOURCE_ERROR_NONE;

	/* check if already free */
	if (res->Free != 0)
		return PL_RESOURCE_ERROR_DOUBLE_FREE;

	/* test if we can free children */
	if (mode == PL_RESOURCE_DELETE_PARENT_ONLY && res->Children)
		return PL_RESOURCE_ERROR_CANT_FREE;

	/* free any children */
	child = res->Children;
	while (child)
	{
		next = child->NextSibling;
		_plResDelete(child, PL_RESOURCE_DELETE_ALL);
		child = next;
	}

	res->Children = NULL;

	/* test if we should delete parent */
	if (mode == PL_RESOURCE_DELETE_CHILDREN_ONLY)
		return PL_RESOURCE_ERROR_NONE;

	/* remove from any parent list */
	_plResRemoveParent(res);

	/* mark as free */
	res->Free = 1;

	/* finally free memory */
	plFree(res);

	return PL_RESOURCE_ERROR_NONE;
}

int plResDelete(void *user, int mode)
{
	int err;
	pl_Res *res;
	if ((err = _plResUserToResource(user, &res)) != PL_RESOURCE_ERROR_NONE)
		return err;
	return _plResDelete(res, mode);
}
