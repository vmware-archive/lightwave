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

#ifndef _VMCA_POLICY_H_
#define _VMCA_POLICY_H_

#define VMCA_POLICY_FILE_PATH       VMCA_CONFIG_DIR "/vmca-policy.json"

/* VMCA Policy */

typedef enum _VMCA_POLICY_TYPE
{
    VMCA_POLICY_TYPE_UNDEFINED = 0,
    VMCA_POLICY_TYPE_SN
} VMCA_POLICY_TYPE, *PVMCA_POLICY_TYPE;

// SN Policy Objects

typedef struct _VMCA_SNPOLICY_OPERATION
{
    PSTR                            pszData;
    PSTR                            pszCondition;
    PSTR                            pszWith;
} VMCA_SNPOLICY_OPERATION, *PVMCA_SNPOLICY_OPERATION;

typedef struct _VMCA_SNPOLICY
{
    BOOLEAN                         bEnabled;
    PVMCA_SNPOLICY_OPERATION        pMatch;
    DWORD                           dwValidateLen;
    PVMCA_SNPOLICY_OPERATION        *ppValidate;
} VMCA_SNPOLICY, *PVMCA_SNPOLICY;

// High level policy objects

typedef union _VMCA_POLICY_RULES
{
    VMCA_SNPOLICY                   SN;
} VMCA_POLICY_RULES, *PVMCA_POLICY_RULES;

typedef struct _VMCA_POLICY
{
    VMCA_POLICY_TYPE                type;
    VMCA_POLICY_RULES               Rules;
} VMCA_POLICY, *PVMCA_POLICY;

/* ../common/policy.c */

DWORD
VMCAPolicyInit(
    PSTR                            pszConfigFilePath,
    PVMCA_POLICY                    **pppPolicies
    );

DWORD
VMCAPolicyValidate(
    PVMCA_POLICY                    *ppPolicies,
    PSTR                            pszPKCS10Request,
    PVMCA_REQ_CONTEXT               pReqContext,
    PBOOLEAN                        pbIsValid
    );

VOID
VMCAPolicyFree(
    PVMCA_POLICY                    pPolicy
    );

VOID
VMCAPolicyArrayFree(
    PVMCA_POLICY                    *ppPolicies
    );

#endif /* _VMCA_POLICY_H_ */
