/******************************************************************************
Plush Version 1.2
endian.c
Endian handling functions
Copyright (C) 1996-2000, Justin Frankel and Nullsoft, Inc.
Copyright (C) 2024-2026, erysdren (it/its)
******************************************************************************/

#include <plush/plush.h>

int16_t plSwap16(int16_t n)
{
#if defined(__has_builtin) && __has_builtin(__builtin_bswap16)
	return __builtin_bswap16(n);
#else
	return
		((n >> 8) & 0x00FF) |
		((n << 8) & 0xFF00);
#endif
}

int32_t plSwap32(int32_t n)
{
#if defined(__has_builtin) && __has_builtin(__builtin_bswap32)
	return __builtin_bswap32(n);
#else
	return
		((n >> 24) & 0x000000FF) |
		((n >> 8) & 0x0000FF00) |
		((n << 8) & 0x00FF0000) |
		((n << 24) & 0xFF000000);
#endif
}

int32_t plLittle16(int32_t n)
{
#if PL_BIG_ENDIAN
	return plSwap16(n);
#else
	return n;
#endif
}

int32_t plLittle32(int32_t n)
{
#if PL_BIG_ENDIAN
	return plSwap32(n);
#else
	return n;
#endif
}

int32_t plBig16(int32_t n)
{
#if PL_BIG_ENDIAN
	return n;
#else
	return plSwap16(n);
#endif
}

int32_t plBig32(int32_t n)
{
#if PL_BIG_ENDIAN
	return n;
#else
	return plSwap32(n);
#endif
}
