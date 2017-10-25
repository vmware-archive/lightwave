/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

PVM_METRICS_HISTOGRAM pLdapRequestDuration[METRICS_LDAP_OP_COUNT];

PVM_METRICS_COUNTER pLdapErrorCount[METRICS_LDAP_ERROR_COUNT];

DWORD
VmDirLdapMetricsInit(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD i = 0;

    uint64_t buckets[5] = {1, 10, 100, 500, 1000};

    VM_METRICS_LABEL labelOps[METRICS_LDAP_OP_COUNT][1] = {{{"operation","bind"}},
                                        {{"operation","search"}},
                                        {{"operation","add"}},
                                        {{"operation","modify"}},
                                        {{"operation","delete"}},
                                        {{"operation","unbind"}}};

    VM_METRICS_LABEL labelErrors[METRICS_LDAP_ERROR_COUNT][1] = {{{"code","LDAP_SUCCESS"}},
                                            {{"code","LDAP_UNAVAILABLE"}},
                                            {{"code","LDAP_SERVER_DOWN"}},
                                            {{"code","LDAP_UNWILLING_TO_PERFORM"}},
                                            {{"code","LDAP_INVALID_DN_SYNTAX"}},
                                            {{"code","LDAP_NO_SUCH_ATTRIBUTE"}},
                                            {{"code","LDAP_INVALID_SYNTAX"}},
                                            {{"code","LDAP_UNDEFINED_TYPE"}},
                                            {{"code","LDAP_TYPE_OR_VALUE_EXISTS"}},
                                            {{"code","LDAP_OBJECT_CLASS_VIOLATION"}},
                                            {{"code","LDAP_ALREADY_EXISTS"}},
                                            {{"code","LDAP_CONSTRAINT_VIOLATION"}},
                                            {{"code","LDAP_NOT_ALLOWED_ON_NONLEAF"}},
                                            {{"code","LDAP_PROTOCOL_ERROR"}},
                                            {{"code","LDAP_INVALID_CREDENTIALS"}},
                                            {{"code","LDAP_INSUFFICIENT_ACCESS"}},
                                            {{"code","LDAP_AUTH_METHOD_NOT_SUPPORTED"}},
                                            {{"code","LDAP_SASL_BIND_IN_PROGRESS"}},
                                            {{"code","LDAP_TIMELIMIT_EXCEEDED"}},
                                            {{"code","LDAP_SIZELIMIT_EXCEEDED"}},
                                            {{"code","LDAP_NO_SUCH_OBJECT"}},
                                            {{"code","LDAP_BUSY"}},
                                            {{"code","LDAP_OTHER"}}};

    for (i=0; i < METRICS_LDAP_ERROR_COUNT; i++)
    {
        dwError = VmMetricsCounterNew(pmContext,
                                "vmdir_ldap_error_count",
                                labelErrors[i], 1,
                                "Counter for various LDAP errors",
                                &pLdapErrorCount[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0; i < METRICS_LDAP_OP_COUNT; i++)
    {
        dwError = VmMetricsHistogramNew(pmContext,
                                "vmdir_ldap_request_duration",
                                labelOps[i], 1,
                                "Histogram for LDAP Request Durations for different operations",
                                buckets, 5,
                                &pLdapRequestDuration[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapMetricsInit failed (%d)", dwError);

    goto cleanup;
}
