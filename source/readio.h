/******************************************************************************
Plush Version 1.2
readio.h
I/O callback functions for read_*.c
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2025, erysdren (it/its)
******************************************************************************/

#ifndef _READIO_H_
#define _READIO_H_

typedef struct _pl_IO {
	int (*getchr)(void *user);
	char *(*getstr)(char *buf, size_t len, void *user);
	size_t (*read)(void *buffer, size_t size, size_t count, void *user);
	int (*seek)(void *user, long offset, int origin);
	void (*rewind)(void *user);
	int (*eof)(void *user);
	int (*tell)(void *user);
} pl_IO;

typedef struct _pl_IOMemCtx {
	void *buffer;
	size_t len;
	size_t pos;
} pl_IOMemCtx;

#if !PL_FREESTANDING
extern pl_IO _plIOStdio;
#endif

extern pl_IO _plIOMem;

#endif /* !_READIO_H_ */
