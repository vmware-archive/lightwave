/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"

SSOERROR
SSOMemoryAllocate(
    size_t size,
    void** pp /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    void* p = NULL;

    ASSERT_NOT_NULL(pp);

    p = calloc(1 /* num */, size);
    if (NULL == p)
    {
        e = SSOERROR_OUT_OF_MEMORY;
        BAIL_ON_ERROR(e);
    }

    *pp = p;

error:
    return e;
}

SSOERROR
SSOMemoryAllocateArray(
    size_t count,
    size_t size,
    void** pp /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    void* p = NULL;

    ASSERT_NOT_NULL(pp);

    p = calloc(count, size);
    if (NULL == p)
    {
        e = SSOERROR_OUT_OF_MEMORY;
        BAIL_ON_ERROR(e);
    }

    *pp = p;

error:
    return e;
}

SSOERROR
SSOMemoryReallocate(
    size_t size,
    void** pp /* IN,OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    void* p = NULL;

    ASSERT_NOT_NULL(pp);

    p = realloc(*pp, size);
    if (NULL == p)
    {
        e = SSOERROR_OUT_OF_MEMORY;
        BAIL_ON_ERROR(e);
    }

    *pp = p;

error:
    return e;
}

void
SSOMemoryCopy(
    void* destination,
    const void* source,
    size_t size)
{
    ASSERT_NOT_NULL(destination);
    ASSERT_NOT_NULL(source);
    memcpy(destination, source, size);
}

void
SSOMemoryClear(
    void* p, /* OPT */
    size_t size)
{
    if (p != NULL)
    {
        memset(p, 0, size);
    }
}

void
SSOMemoryFree(
    void* p, /* OPT */
    size_t size)
{
    if (p != NULL)
    {
        memset(p, 0, size);
        free(p);
    }
}

void
SSOMemoryFreeArray(
    void* p /* OPT */,
    size_t count,
    size_t size)
{
    if (p != NULL)
    {
        memset(p, 0, count * size);
        free(p);
    }
}

void
SSOMemoryFreeArrayOfObjects(
    void** pp /* OPT */,
    size_t count,
    GenericDestructorFunction pDestructor)
{
    size_t i = 0;

    ASSERT_NOT_NULL(pDestructor);

    if (pp != NULL)
    {
        for (i = 0; i < count; i++)
        {
            pDestructor(pp[i]);
        }

        memset(pp, 0, count * sizeof(void*));
        free(pp);
    }
}
