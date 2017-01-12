/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* To the extent possible under law, Painless Security, LLC has waived
 * all copyright and related or neighboring rights to GSS-API Memory
 * Management Header. This work is published from: United States.
 */

#ifndef GSSAPI_ALLOC_H
#define GSSAPI_ALLOC_H

#ifdef _WIN32
#include <Windows.h>
#endif

#include <string.h>
#include <stdio.h>


#ifdef _USE_STATIC_INLINE
#define STATIC_INLINE_DEF static inline
#else
#define STATIC_INLINE_DEF
#endif

/* Prototypes */

STATIC_INLINE_DEF void gssalloc_free(void *value);
STATIC_INLINE_DEF void *gssalloc_malloc(size_t size);
STATIC_INLINE_DEF void *gssalloc_calloc(size_t count, size_t size);
STATIC_INLINE_DEF void *gssalloc_realloc(void *value, size_t size);
STATIC_INLINE_DEF char *gssalloc_strdup(const char *str);

#ifdef _GSSAPI_ALLOC_C
#if defined(_WIN32)
STATIC_INLINE_DEF void
gssalloc_free(void *value)
{
    if (value)
        HeapFree(GetProcessHeap(), 0, value);
}

STATIC_INLINE_DEF void *
gssalloc_malloc(size_t size)
{
    void *value = HeapAlloc(GetProcessHeap(), 0, size);
    
    return value;
}

STATIC_INLINE_DEF void *
gssalloc_calloc(size_t count, size_t size)
{
    void *value = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, count * size);

    return value;
}

STATIC_INLINE_DEF void *
gssalloc_realloc(void *value, size_t size)
{
    void *rvalue = HeapReAlloc(GetProcessHeap(), 0, value, size);

    return rvalue;
}

#elif defined(DEBUG_GSSALLOC)

/* Be deliberately incompatible with malloc and free, to allow us to detect
 * mismatched malloc/gssalloc usage on Unix. */

STATIC_INLINE_DEF void
gssalloc_free(void *value)
{
    char *p = (char *)value - 8;

    if (value == NULL)
        return;
    if (memcmp(p, "gssalloc", 8) != 0)
        abort();
    free(p);
}

STATIC_INLINE_DEF void *
gssalloc_malloc(size_t size)
{
    char *p = calloc(size + 8, 1);

    memcpy(p, "gssalloc", 8);
    return p + 8;
}

STATIC_INLINE_DEF void *
gssalloc_calloc(size_t count, size_t size)
{
    return gssalloc_malloc(count * size);
}

STATIC_INLINE_DEF void *
gssalloc_realloc(void *value, size_t size)
{
    char *p = (char *)value - 8;

    if (value == NULL)
        return gssalloc_malloc(size);
    if (memcmp(p, "gssalloc", 8) != 0)
        abort();
    return (char *)realloc(p, size) + 8;
}

#else /* not _WIN32 or DEBUG_GSSALLOC */

/* Normal Unix case, just use free/malloc/calloc/realloc. */

STATIC_INLINE_DEF void
gssalloc_free(void *value)
{
    free(value);
}

STATIC_INLINE_DEF void *
gssalloc_malloc(size_t size)
{
    return malloc(size);
}

STATIC_INLINE_DEF void *
gssalloc_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

STATIC_INLINE_DEF void *
gssalloc_realloc(void *value, size_t size)
{
    return realloc(value, size);
}

#endif /* not _WIN32 or DEBUG_GSSALLOC */

STATIC_INLINE_DEF char *
gssalloc_strdup(const char *str)
{
    size_t size = strlen(str)+1;
    char *copy = gssalloc_malloc(size);
    if (copy) {
        memcpy(copy, str, size);
        copy[size-1] = '\0';
    }
    return copy;
}
#endif /* _GSSAPI_ALLOC_C */
#endif
