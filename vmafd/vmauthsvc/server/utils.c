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



#include "includes.h"

VOID
VmAuthsvcdStateSet(
    VMAUTHSVC_SERVER_STATE   state)
{
    pthread_mutex_lock(&gVmauthsvcGlobals.mutex);
    gVmauthsvcGlobals.vmauthsvcdState = state;
    pthread_mutex_unlock(&gVmauthsvcGlobals.mutex);
}

VMAUTHSVC_SERVER_STATE
VmAuthsvcdState(
    VOID
    )
{
    VMAUTHSVC_SERVER_STATE rtnState;

    pthread_mutex_lock(&gVmauthsvcGlobals.mutex);
    rtnState = gVmauthsvcGlobals.vmauthsvcdState;
    pthread_mutex_unlock(&gVmauthsvcGlobals.mutex);

    return rtnState;
}
