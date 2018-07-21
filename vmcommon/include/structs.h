/*
 * Copyright © 2017-2018 VMware, Inc.  All Rights Reserved.
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

/*
 * Counter metrics data
 */
typedef struct _VM_METRICS_COUNTER
{
    PSTR                            pszName;
    PSTR                            pszLabel;
    PSTR                            pszDescription;
    int64_t                         value;

} VM_METRICS_COUNTER, *PVM_METRICS_COUNTER;

/*
 * Gauge metrics data
 */
typedef struct _VM_METRICS_GAUGE
{
    PSTR                            pszName;
    PSTR                            pszLabel;
    PSTR                            pszDescription;
    int64_t                         value;

} VM_METRICS_GAUGE, *PVM_METRICS_GAUGE;

/*
 * Histogram metrics data
 */
typedef struct _VM_METRICS_HISTOGRAM
{
    PSTR                            pszName;
    PSTR                            pszLabel;
    PSTR                            pszDescription;
    DWORD                           bucketSize;
    int64_t*                        pBucketKeys;
    int64_t*                        pBucketValues;
    int64_t                         count;
    int64_t                         sum;

} VM_METRICS_HISTOGRAM, *PVM_METRICS_HISTOGRAM;

/*
 * Enumerator for Metrics Type
 */
typedef enum
{
    VM_METRICS_TYPE_COUNTER,
    VM_METRICS_TYPE_GAUGE,
    VM_METRICS_TYPE_HISTOGRAM

} VM_METRICS_TYPE;

/*
 * Linked List Entry
 */
typedef struct _VM_METRICS_LIST_ENTRY
{
    PVOID                           pData;
    VM_METRICS_TYPE                 type;
    struct _VM_METRICS_LIST_ENTRY*  pNext;

} VM_METRICS_LIST_ENTRY, *PVM_METRICS_LIST_ENTRY;

/*
 * Metrics context structure
 */
typedef struct _VM_METRICS_CONTEXT
{
    PVM_METRICS_LIST_ENTRY          pMetrics;
    pthread_rwlock_t                rwLock;

} VM_METRICS_CONTEXT, *PVM_METRICS_CONTEXT;

/* httpclient */
typedef struct _VM_HTTP_CLIENT
{
    CURL* pCurl;
    struct curl_slist *pHeaders;
    PLW_HASHMAP pQueryParamsMap;
    PSTR  pszTlsCAPath;
    PSTR  pszBody;
    long nStatus;
    PSTR pszResult;
    size_t nResultLen;
} VM_HTTP_CLIENT, *PVM_HTTP_CLIENT;

/* json result */
typedef struct _VM_JSON_RESULT
{
    json_t *pJsonRoot;
    int nJsonErrorLine;
    PSTR pszJsonErrorText;
}VM_JSON_RESULT, *PVM_JSON_RESULT;
