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
SSOKeyValuePairNew(
    PSSO_KEY_VALUE_PAIR* pp,
    PCSTRING pszKey,
    PCSTRING pszValue)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_KEY_VALUE_PAIR p = NULL;

    ASSERT_NOT_NULL(pp);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(pszValue);

    e = SSOMemoryAllocate(sizeof(SSO_KEY_VALUE_PAIR), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOStringAllocate(pszKey, &p->pszKey);
    BAIL_ON_ERROR(e);

    e = SSOStringAllocate(pszValue, &p->pszValue);
    BAIL_ON_ERROR(e);

    *pp = p;

error:
    if (e != SSOERROR_NONE)
    {
        SSOKeyValuePairDelete(p);
    }
    return e;
}

void
SSOKeyValuePairDelete(
    PSSO_KEY_VALUE_PAIR p)
{
    if (p != NULL)
    {
        SSOStringFree(p->pszKey);
        SSOStringFree(p->pszValue);
        SSOMemoryFree(p, sizeof(SSO_KEY_VALUE_PAIR));
    }
}

PCSTRING
SSOKeyValuePairGetKey(
    PCSSO_KEY_VALUE_PAIR p)
{
    ASSERT_NOT_NULL(p);
    return p->pszKey;
}

PCSTRING
SSOKeyValuePairGetValue(
    PCSSO_KEY_VALUE_PAIR p)
{
    ASSERT_NOT_NULL(p);
    return p->pszValue;
}
