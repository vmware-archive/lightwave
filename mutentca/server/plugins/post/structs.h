/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

typedef struct _LWCA_POST_HANDLE
{
    /** the Lightwave server IP */
    PSTR                pszLwServer;
    /** the Post server IP */
    PSTR                pszPostServer;
    /** the domain used by POST */
    PSTR                pszDomain;
    /** the access token for communicating with POST's rest-head */
    PSTR                pszAccessToken;
    /** parsed Access token into an object */
    POIDC_ACCESS_TOKEN  pOidcToken;
    /** mutex to check validity and make changes to the access token */
    pthread_mutex_t     accessTokenMutex;

} LWCA_POST_HANDLE, *PLWCA_POST_HANDLE;
