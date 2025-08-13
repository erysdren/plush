/******************************************************************************
Plush Version 1.2
readio.h
I/O callback functions for read_*.c
Copyright (c) 2025, erysdren (it/its)
******************************************************************************/

#ifndef _READIO_H_
#define _READIO_H_

typedef struct _pl_IO {
	int (*getc)(void *user);
	char *(*gets)(char *buf, size_t len, void *user);
	size_t (*read)(void *buffer, size_t size, size_t count, void *user);
	int (*seek)(void *user, long offset, int origin);
	void (*rewind)(void *user);
	int (*eof)(void *user);
} pl_IO;

typedef struct _pl_IOMemCtx {
	void *buffer;
	size_t len;
	size_t pos;
} pl_IOMemCtx;

extern pl_IO _plIOStdio;
extern pl_IO _plIOMem;

#endif /* !_READIO_H_ */
