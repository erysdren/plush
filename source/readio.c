/******************************************************************************
Plush Version 1.2
readio.c
I/O callback functions for read_*.c
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

#include "readio.h"

static int _plIOStdioGetc(void *user)
{
	return fgetc((FILE *)user);
}

static char *_plIOStdioGets(char *buf, size_t len, void *user)
{
	return fgets(buf, len, (FILE *)user);
}

static size_t _plIOStdioRead(void *buffer, size_t size, size_t count, void *user)
{
	return fread(buffer, size, count, (FILE *)user);
}

static int _plIOStdioSeek(void *user, long offset, int origin)
{
	return fseek((FILE *)user, offset, origin);
}

static void _plIOStdioRewind(void *user)
{
	rewind((FILE *)user);
}

static int _plIOStdioEof(void *user)
{
	return feof((FILE *)user);
}

static int _plIOStdioTell(void *user)
{
	return ftell((FILE *)user);
}

pl_IO _plIOStdio = {
	_plIOStdioGetc, _plIOStdioGets, _plIOStdioRead, _plIOStdioSeek, _plIOStdioRewind, _plIOStdioEof, _plIOStdioTell
};

static int _plIOMemGetc(void *user)
{
	pl_IOMemCtx *ctx = (pl_IOMemCtx *)user;
	if (ctx->pos >= ctx->len)
		return EOF;
	return (int)((uint8_t *)ctx->buffer)[ctx->pos++];
}

static char *_plIOMemGets(char *buf, size_t len, void *user)
{
	size_t pos = 0;
	if (!buf || len < 2)
		return NULL;
	while (pos < len - 1)
	{
		int c = _plIOMemGetc(user);
		if (c == EOF)
			break;
		buf[pos++] = c;
		if (c == '\n')
			break;
	}
	if (pos == 0)
		return NULL;
	buf[pos] = '\0';
	return buf;
}

static size_t _plIOMemRead(void *buffer, size_t size, size_t count, void *user)
{
	pl_IOMemCtx *ctx = (pl_IOMemCtx *)user;
	size_t len = (size * count) > (ctx->len - ctx->pos) ? (ctx->len - ctx->pos) : (size * count);
	plMemCpy(buffer, (uint8_t *)ctx->buffer + ctx->pos, len);
	ctx->pos += len;
	return len;
}

static int _plIOMemSeek(void *user, long offset, int origin)
{
	pl_IOMemCtx *ctx = (pl_IOMemCtx *)user;
	switch (origin)
	{
		case SEEK_SET:
			ctx->pos = offset;
			break;

		case SEEK_CUR:
			ctx->pos += offset;
			break;

		case SEEK_END:
			ctx->pos = ctx->len + offset;
			break;
	}
	if (ctx->pos > ctx->len) return 1;
	return 0;
}

static void _plIOMemRewind(void *user)
{
	pl_IOMemCtx *ctx = (pl_IOMemCtx *)user;
	ctx->pos = 0;
}

static int _plIOMemEof(void *user)
{
	pl_IOMemCtx *ctx = (pl_IOMemCtx *)user;
	return ctx->pos >= ctx->len ? 1 : 0;
}

static int _plIOMemTell(void *user)
{
	pl_IOMemCtx *ctx = (pl_IOMemCtx *)user;
	return ctx->pos;
}

pl_IO _plIOMem = {
	_plIOMemGetc, _plIOMemGets, _plIOMemRead, _plIOMemSeek, _plIOMemRewind, _plIOMemEof, _plIOMemTell
};
