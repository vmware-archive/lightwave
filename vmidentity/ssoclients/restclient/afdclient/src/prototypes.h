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

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_

// data object to/from Json

SSOERROR
AfdActiveDirectoryJoinInfoDataToJson(
    const AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfo,
    PSSO_JSON pJson);

SSOERROR
AfdJsonToActiveDirectoryJoinInfoData(
    PCSSO_JSON pJson,
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfo);

SSOERROR
AfdActiveDirectoryJoinRequestDataToJson(
    const AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest,
    PSSO_JSON pJson);

SSOERROR
AfdJsonToActiveDirectoryJoinRequestData(
    PCSSO_JSON pJson,
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA** ppActiveDirectoryJoinRequest);

#endif /* PROTOTYPES_H_ */
