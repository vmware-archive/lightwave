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


#ifndef _VDCADMINTOOL_PROTOTYPE_H_
#define _VDCADMINTOOL_PROTOTYPE_H_

VOID
VdcadminTestSASLClient(
    VOID
    );

VOID
VdcadminReplNow(
    VOID
    );

VOID
VdcadminForceResetPassword(
    VOID
    );

VOID
VdcadminSetLogParameters(
    VOID
    );

VOID
VdcadminGetLogParameters(
    VOID
    );

VOID
VdcadminSetVmdirState(
    VOID
    );

VOID
VdcadminGetVmdirState(
    VOID
    );

VOID
VdcadminSetSRPAuthData(
    VOID
    );

VOID
VdcadminUrgentReplicationRequest(
    VOID
    );

#endif
