/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”);; you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/* oidc-auth.c */

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_Valid(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_InvalidIssuer(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_InvalidAud(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_Expired(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_NotInHeader(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_Valid(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidIssuer(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidAud(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_Expired(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_NoPOP(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidPOPData(
    VOID                    **state
    );

VOID
Test_LwCAOIDCTokenAuthenticate_UnknownTokType(
    VOID                    **state
    );
