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

#ifndef _DEFINES_H_
#define _DEFINES_H_

#define BAIL_ON_ERROR(x) \
    { if ((x) != 0) { fprintf(stderr, "error [%u] in function [%s] at line [%d]\n", x, __FUNCTION__, __LINE__); goto error; } }

#define BAIL_ON_NULL_ARGUMENT(x) \
    { if (NULL == (x)) { fprintf(stderr, "NULL argument! variable [%s] in function [%s]", #x, __FUNCTION__); e = SSOERROR_INVALID_ARGUMENT; goto error; } }

#define ASSERT_NOT_NULL(x) \
    { if (NULL == (x)) { fprintf(stderr, "NULL argument! variable [%s] in function [%s]", #x, __FUNCTION__); } }

#define IS_NULL_OR_EMPTY_STRING(str) (!(str) || !*(str))

// for test code
#define TEST_ASSERT_TRUE(x) \
    { if (!(x)) { printf("assertion failed:[%s]", #x); return false; } }

#define TEST_ASSERT_EQUAL(expected, actual) \
    { if ((expected) != (actual)) { printf("assertion failed:expected[%s], actual[%s]", SSOErrorToString(expected), SSOErrorToString(actual)); return false; } }

#define TEST_ASSERT_EQUAL_STRINGS(expected, actual) \
    { if (!SSOStringEqual((expected), (actual))) { printf("assertion failed:expected[%s], actual[%s]", (expected), (actual)); return false; } }

#define TEST_ASSERT_SUCCESS(e) \
    { if ((e) != SSOERROR_NONE) { printf("assertion failed:expected success, actual[%s]", SSOErrorToString(e)); return false; } }

#endif
