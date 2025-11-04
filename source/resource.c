/******************************************************************************
Plush Version 1.2
resource.c
Resource memory management functions
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

/*
** 0 - no error
** 1 - not a valid resource pointer
** 2 - resource pointer was already freed
*/
#define PL_RESOURCE_ERROR_NONE (0)
#define PL_RESOURCE_ERROR_NOT_RESOURCE (1)
#define PL_RESOURCE_ERROR_DOUBLE_FREE (2)

static const size_t _plResAlign = PL_RESOURCE_ALIGNMENT;
static const size_t _plResMagic = 0xBEEFCAFE;

typedef struct _pl_Res {
	struct _pl_Res *Parent;
	struct _pl_Res *PrevSibling;
	struct _pl_Res *NextSibling;
	struct _pl_Res *Children;
	size_t Free;
	size_t Size;
	void *Original;
	size_t Magic;
} pl_Res;

static int _plResUserToResource(void *user, pl_Res **res)
{
	pl_Res *_res;

	if (!user)
		return PL_RESOURCE_ERROR_NONE;

	/* muahaha */
	_res = (pl_Res *)((uint8_t *)user - sizeof(pl_Res));
	if (_res->Magic != _plResMagic)
		return PL_RESOURCE_ERROR_NOT_RESOURCE;

	/* check if free */
	if (_res->Free != 0)
		return PL_RESOURCE_ERROR_DOUBLE_FREE;

	*res = _res;

	return PL_RESOURCE_ERROR_NONE;
}

void *plResCreate(void *parent, size_t size)
{
	int err;
	pl_Res *res;
	uint8_t *user, *original;
	size_t padding, total_size, alignment;

	/* calculate alignment */
	alignment = _plResAlign;
	if (alignment < sizeof(void *))
		alignment = sizeof(void *);

	/* calculate padding */
	padding = (alignment - (size % alignment));

	/* calculate total allocation size */
	/* FIXME: check for overflows */
	total_size = size + alignment + sizeof(pl_Res) + padding;

	/* allocate resource */
	original = (uint8_t *)plMalloc(total_size);
	if (!original)
		return NULL;

	/* align user pointer */
	user = (uint8_t *)original + sizeof(pl_Res);
	user += alignment - (((size_t)user) % alignment);

	/* setup resource structure pointer */
	res = (pl_Res *)(user - sizeof(pl_Res));

	/* initialize the leader area to zero */
	plMemSet(original, 0, user - (uint8_t *)original);

	/* setup fields */
	res->Magic = _plResMagic;
	res->Size = size;
	res->Original = (void *)original;
	res->Free = 0;
	res->Parent = NULL;
	res->Children = NULL;
	res->NextSibling = NULL;
	res->PrevSibling = NULL;

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

	return (void *)user;
}

static int _plResRemoveParent(pl_Res *res)
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

	return PL_RESOURCE_ERROR_NONE;
}

void *plResAddChild(void *parent, void *child)
{
	int err;
	pl_Res *resparent, *reschild;
	if ((err = _plResUserToResource(parent, &resparent)) != PL_RESOURCE_ERROR_NONE)
		return NULL;
	if ((err = _plResUserToResource(child, &reschild)) != PL_RESOURCE_ERROR_NONE)
		return NULL;
	if ((err = _plResRemoveParent(reschild)) != PL_RESOURCE_ERROR_NONE)
		return NULL;
	reschild->Parent = resparent;
	reschild->NextSibling = resparent->Children;
	if (resparent->Children)
		resparent->Children->PrevSibling = reschild;
	resparent->Children = reschild;
	return child;
}

void *plResRemoveParent(void *user)
{
	int err;
	pl_Res *res;
	if ((err = _plResUserToResource(user, &res)) != PL_RESOURCE_ERROR_NONE)
		return NULL;
	if ((err = _plResRemoveParent(res)) != PL_RESOURCE_ERROR_NONE)
		return NULL;
	return user;
}

static int _plResDelete(pl_Res *res)
{
	pl_Res *child, *next;

	/* test for null */
	if (!res)
		return PL_RESOURCE_ERROR_NONE;

	/* check if already free */
	if (res->Free != 0)
		return PL_RESOURCE_ERROR_DOUBLE_FREE;

	/* free any children */
	child = res->Children;
	while (child)
	{
		next = child->NextSibling;
		_plResDelete(child);
		child = next;
	}

	res->Children = NULL;

	/* remove from any parent list */
	_plResRemoveParent(res);

	/* mark as free */
	res->Free = 1;

	/* finally free memory */
	plFree(res->Original);

	return PL_RESOURCE_ERROR_NONE;
}

void plResDelete(void *user)
{
	pl_Res *res;
	if (_plResUserToResource(user, &res) != PL_RESOURCE_ERROR_NONE)
		return;
	_plResDelete(res);
}

const char *plResStrDup(void *parent, const char *s)
{
	char *ret;
	size_t len;
	if (!s)
		return NULL;
	len = plStrLen(s);
	if (!len)
		return NULL;
	ret = plResCreate(parent, len + 1);
	plStrNCpy(ret, s, len);
	ret[len] = 0;
	return ret;
}

void *plResMemDup(void *parent, void *buf, size_t len)
{
	void *ret;
	if (!buf || !len)
		return NULL;
	ret = plResCreate(parent, len);
	plStrNCpy(ret, buf, len);
	return ret;
}
