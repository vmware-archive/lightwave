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

#ifndef _VMCA_SRV_CONFIG_H_
#define _VMCA_SRV_CONFIG_H_

#define VMCA_CONFIG_FILE_PATH       VMCA_CONFIG_DIR "/vmcaconfig.json"


DWORD
VMCAConfigLoadFile(
    PCSTR                   pcszFilePath,
    PVMCA_JSON_OBJECT       *ppJsonConfig
    );

DWORD
VMCAConfigGetComponent(
    PVMCA_JSON_OBJECT       pJson,
    PCSTR                   pcszComponentName,
    PVMCA_JSON_OBJECT       *ppJsonConfig
    );

DWORD
VMCAConfigGetStringParameter(
    PVMCA_JSON_OBJECT       pJson,
    PCSTR                   pcszParameter,
    PSTR                    *ppszParameterValue
    );

#endif /* _VMCA_SRV_CONFIG_H_ */
