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

#ifndef _TESTS_CASES_H_
#define _TESTS_CASES_H_

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include "ssotypes.h"
#include "defines.h"
#include "common_types.h"
#include "common.h"
#include "ssoerrors.h"

#include "oidc_types.h"
#include "oidc.h"
#include "structs.h"
#include "prototypes.h"

SSOERROR
TestInit(
    PCSTRING pszServer,
    PCSTRING pszTenant,
    PCSTRING pszUsername,
    PCSTRING pszPassword,
    bool highAvailabilityEnabled);

void
TestCleanup();

bool
TestPasswordGrantSuccessResponse();

bool
TestPasswordGrantSuccessResponseNoRefreshToken();

bool
TestPasswordGrantErrorResponse();

bool
TestRefreshTokenGrantSuccessResponse();

bool
TestRefreshTokenGrantErrorResponse();

bool
TestIDTokenParseSuccessWithGroups();

bool
TestIDTokenParseSuccessWithoutGroups();

bool
TestIDTokenParseFail();

bool
TestIDTokenBuildFailInvalidSignature();

bool
TestIDTokenBuildFailExpired();

bool
TestAccessTokenParseSuccessWithGroups();

bool
TestAccessTokenParseSuccessWithoutGroups();

bool
TestAccessTokenParseFail();

bool
TestAccessTokenBuildFailInvalidSignature();

bool
TestAccessTokenBuildFailInvalidAudience();

bool
TestAccessTokenBuildFailExpired();

#endif
