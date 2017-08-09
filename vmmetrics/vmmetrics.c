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

static
DWORD
VmMetricsMakeLabel(
    PVM_METRICS_LABEL pLabel,
    DWORD dwLabelCount,
    PSTR* pszLabelOut
    );

static
DWORD
VmMetricsGetCounterData(
    PVM_METRICS_COUNTER pCounter,
    PSTR pszData,
    DWORD dwDataLen,
    DWORD dwBufLen,
    BOOLEAN bNameUsed
    );

static
DWORD
VmMetricsGetGaugeData(
    PVM_METRICS_GAUGE pGauge,
    PSTR pszData,
    DWORD dwDataLen,
    DWORD dwBufLen,
    BOOLEAN bNameUsed
    );

static
DWORD
VmMetricsGetHistogramData(
    PVM_METRICS_HISTOGRAM pHistogram,
    PSTR pszData,
    DWORD dwDataLen,
    DWORD dwBufLen,
    BOOLEAN bNameUsed
    );

static
VOID
VmMetricsNoopHashMapPairFree(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID pUnused
    );

/*
 * Initialize the metrics context
 */
DWORD
VmMetricsInit(
    PVM_METRICS_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;

    if (!ppContext)
    {
        dwError = VM_METRICS_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    dwError = VmMetricsAllocateMemory(
            sizeof(VM_METRICS_CONTEXT),
            (PVOID*)&pContext);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    pContext->pMetrics = NULL;
    dwError = pthread_rwlock_init(&pContext->rwLock, NULL);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    *ppContext = pContext;

cleanup:
    return dwError;

error:
    VM_METRICS_SAFE_FREE_MEMORY(pContext);
    goto cleanup;
}

/*
 * Add a new counter metric
 */
DWORD
VmMetricsCounterNew(
    PVM_METRICS_CONTEXT pContext,
    PCSTR pszName,
    const PVM_METRICS_LABEL pLabel,
    const DWORD iLabelCount,
    PCSTR pszDescription,
    PVM_METRICS_COUNTER* ppCounter
    )
{
    DWORD dwError = 0;
    PVM_METRICS_LIST_ENTRY pEntry = NULL;
    PVM_METRICS_COUNTER pCounter = NULL;

    if (!ppCounter || !pContext || !pszName || !pszDescription)
    {
        dwError = VM_METRICS_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    dwError = VmMetricsAllocateMemory(
            sizeof(VM_METRICS_LIST_ENTRY),
            (PVOID*)&pEntry);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    dwError = VmMetricsAllocateMemory(
            sizeof(VM_METRICS_COUNTER),
            (PVOID*)&pCounter);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    pCounter->pszName = strdup(pszName);
    if (!pCounter->pszName)
    {
        dwError = VM_METRICS_ERROR_NO_MEMORY;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    if (pLabel && iLabelCount)
    {
        dwError = VmMetricsMakeLabel(pLabel, iLabelCount, &pCounter->pszLabel);
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }
    else
    {
        pCounter->pszLabel = NULL;
    }

    pCounter->pszDescription = strdup(pszDescription);
    if (!pCounter->pszDescription)
    {
        dwError = VM_METRICS_ERROR_NO_MEMORY;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    pCounter->value = 0;

    pEntry->pData = (PVOID)pCounter;
    pEntry->type = VM_METRICS_TYPE_COUNTER;
    pEntry->pNext = NULL;

    pthread_rwlock_wrlock(&pContext->rwLock);
    if (pContext->pMetrics)
    {
        pEntry->pNext = pContext->pMetrics;
    }

    pContext->pMetrics = pEntry;
    pthread_rwlock_unlock(&pContext->rwLock);

    *ppCounter = pCounter;

cleanup:
    return dwError;

error:
    if (pCounter)
    {
        VM_METRICS_SAFE_FREE_MEMORY(pCounter->pszName);
        VM_METRICS_SAFE_FREE_MEMORY(pCounter->pszLabel);
        VM_METRICS_SAFE_FREE_MEMORY(pCounter->pszDescription);
        VM_METRICS_SAFE_FREE_MEMORY(pCounter);
    }
    VM_METRICS_SAFE_FREE_MEMORY(pEntry);

    goto cleanup;
}

/*
 * Add a new gauge metric
 */
DWORD
VmMetricsGaugeNew(
    PVM_METRICS_CONTEXT pContext,
    PCSTR pszName,
    const PVM_METRICS_LABEL pLabel,
    const DWORD iLabelCount,
    PCSTR pszDescription,
    PVM_METRICS_GAUGE* ppGauge
    )
{
    DWORD dwError = 0;
    PVM_METRICS_LIST_ENTRY pEntry = NULL;
    PVM_METRICS_GAUGE pGauge = NULL;

    if (!ppGauge || !pContext || !pszName || !pszDescription)
    {
        dwError = VM_METRICS_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    dwError = VmMetricsAllocateMemory(
            sizeof(VM_METRICS_LIST_ENTRY),
            (PVOID*)&pEntry);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    dwError = VmMetricsAllocateMemory(
            sizeof(VM_METRICS_GAUGE),
            (PVOID*)&pGauge);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    pGauge->pszName = strdup(pszName);
    if (!pGauge->pszName)
    {
        dwError = VM_METRICS_ERROR_NO_MEMORY;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    if (pLabel && iLabelCount)
    {
        dwError = VmMetricsMakeLabel(pLabel, iLabelCount, &pGauge->pszLabel);
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }
    else
    {
        pGauge->pszLabel = NULL;
    }

    pGauge->pszDescription = strdup(pszDescription);
    if (!pGauge->pszDescription)
    {
        dwError = VM_METRICS_ERROR_NO_MEMORY;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    pGauge->value = 0;

    pEntry->pData = (PVOID)pGauge;
    pEntry->type = VM_METRICS_TYPE_GAUGE;
    pEntry->pNext = NULL;

    pthread_rwlock_wrlock(&pContext->rwLock);
    if (pContext->pMetrics)
    {
        pEntry->pNext = pContext->pMetrics;
    }

    pContext->pMetrics = pEntry;
    pthread_rwlock_unlock(&pContext->rwLock);

    *ppGauge = pGauge;

cleanup:
    return dwError;

error:
    if (pGauge)
    {
        VM_METRICS_SAFE_FREE_MEMORY(pGauge->pszName);
        VM_METRICS_SAFE_FREE_MEMORY(pGauge->pszLabel);
        VM_METRICS_SAFE_FREE_MEMORY(pGauge->pszDescription);
        VM_METRICS_SAFE_FREE_MEMORY(pGauge);
    }
    VM_METRICS_SAFE_FREE_MEMORY(pEntry);

    goto cleanup;
}

/*
 * Add a new histogram metric
 */
DWORD
VmMetricsHistogramNew(
    PVM_METRICS_CONTEXT pContext,
    PCSTR pszName,
    const PVM_METRICS_LABEL pLabel,
    const DWORD iLabelCount,
    PCSTR pszDescription,
    const uint64_t iBuckets[],
    const DWORD iBucketSize,
    PVM_METRICS_HISTOGRAM* ppHistogram
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PVM_METRICS_LIST_ENTRY pEntry = NULL;
    PVM_METRICS_HISTOGRAM pHistogram = NULL;

    if (!ppHistogram || !pContext || !pszName || !pszDescription)
    {
        dwError = VM_METRICS_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    dwError = VmMetricsAllocateMemory(
            sizeof(VM_METRICS_LIST_ENTRY),
            (PVOID*)&pEntry);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    dwError = VmMetricsAllocateMemory(
            sizeof(VM_METRICS_HISTOGRAM),
            (PVOID*)&pHistogram);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    pHistogram->pszName = strdup(pszName);
    if (!pHistogram->pszName)
    {
        dwError = VM_METRICS_ERROR_NO_MEMORY;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    if (pLabel && iLabelCount)
    {
        dwError = VmMetricsMakeLabel(pLabel, iLabelCount, &pHistogram->pszLabel);
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }
    else
    {
        pHistogram->pszLabel = NULL;
    }
    pHistogram->pszDescription = strdup(pszDescription);
    if (!pHistogram->pszDescription)
    {
        dwError = VM_METRICS_ERROR_NO_MEMORY;
        BAIL_ON_VM_METRICS_ERROR(dwError);
    }

    pHistogram->bucketSize = iBucketSize;

    dwError = VmMetricsAllocateMemory(
            iBucketSize * sizeof(int64_t),
            (PVOID*)&pHistogram->pBucketKeys);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    dwError = VmMetricsAllocateMemory(
            iBucketSize * sizeof(int64_t),
            (PVOID*)&pHistogram->pBucketValues);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    for (i=0; i<pHistogram->bucketSize; i++)
    {
        pHistogram->pBucketKeys[i] = (int64_t)iBuckets[i];
        pHistogram->pBucketValues[i] = 0;
    }

    pEntry->pData = (PVOID)pHistogram;
    pEntry->type = VM_METRICS_TYPE_HISTOGRAM;
    pEntry->pNext = NULL;

    pthread_rwlock_wrlock(&pContext->rwLock);
    if (pContext->pMetrics)
    {
        pEntry->pNext = pContext->pMetrics;
    }

    pContext->pMetrics = pEntry;
    pthread_rwlock_unlock(&pContext->rwLock);

    *ppHistogram = pHistogram;

cleanup:
    return dwError;

error:
    if (pHistogram)
    {
        VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pszName);
        VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pszLabel);
        VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pszDescription);
        VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pBucketKeys);
        VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pBucketValues);
        VM_METRICS_SAFE_FREE_MEMORY(pHistogram);
    }
    VM_METRICS_SAFE_FREE_MEMORY(pEntry);

    goto cleanup;
}

/*
 * Increment the counter by 1
 */
VOID
VmMetricsCounterIncrement(
    PVM_METRICS_COUNTER pCounter
    )
{
    LwInterlockedIncrement64(&pCounter->value);
}

/*
 * Add the value to the counter
 */
VOID
VmMetricsCounterAdd(
    PVM_METRICS_COUNTER pCounter,
    uint64_t value
    )
{
    LwInterlockedAdd64(&pCounter->value, (int64_t)value);
}

/*
 * Set the value of the gague
 */
VOID
VmMetricsGaugeSet(
    PVM_METRICS_GAUGE pGauge,
    int64_t value
    )
{
    LwInterlockedExchange64(&pGauge->value, value);
}

/*
 * Increment the gauge by 1
 */
VOID
VmMetricsGaugeIncrement(
    PVM_METRICS_GAUGE pGauge
    )
{
    LwInterlockedIncrement64(&pGauge->value);
}

/*
 * Decrement the gauge by 1
 */
VOID
VmMetricsGaugeDecrement(
    PVM_METRICS_GAUGE pGauge
    )
{
    LwInterlockedDecrement64(&pGauge->value);
}

/*
 * Add the value to the gauge
 */
VOID
VmMetricsGaugeAdd(
    PVM_METRICS_GAUGE pGauge,
    uint64_t value
    )
{
    LwInterlockedAdd64(&pGauge->value, (int64_t)value);
}

/*
 * Subtract the value from the gauge
 */
VOID
VmMetricsGaugeSubtract(
    PVM_METRICS_GAUGE pGauge,
    uint64_t value
    )
{
    LwInterlockedSubtract64(&pGauge->value, (int64_t)value);
}

/*
 * Set the value of the gauge to the current Unix time in secondss
 */
VOID
VmMetricsGaugeSetToCurrentTime(
    PVM_METRICS_GAUGE pGauge
    )
{
    LwInterlockedExchange64(&pGauge->value, (int64_t)time(NULL));
}

/*
 * Add the given value to the histogram
 */
VOID
VmMetricsHistogramUpdate(
    PVM_METRICS_HISTOGRAM pHistogram,
    uint64_t value
    )
{
    DWORD i = 0;

    for (i=0; i<pHistogram->bucketSize; i++)
    {
        if (value <= pHistogram->pBucketKeys[i])
        {
            LwInterlockedIncrement64(&pHistogram->pBucketValues[i]);
        }
    }

    LwInterlockedIncrement64(&pHistogram->count);
    LwInterlockedAdd64(&pHistogram->sum, (int64_t)value);
}

/*
 * Return the list of all metrics data in Prometheus format (string/text)
 */
DWORD
VmMetricsGetPrometheusData(
    PVM_METRICS_CONTEXT pContext,
    PSTR* ppszData,
    DWORD* pDataLen
    )
{
    DWORD dwError = 0;
    DWORD bufLen = 0;
    BOOLEAN realloc = FALSE;
    BOOLEAN bNameUsed = FALSE;
    PLW_HASHMAP pNamesMap = NULL;

    PVM_METRICS_LIST_ENTRY pEntry = NULL;
    PVM_METRICS_COUNTER pCounter = NULL;
    PVM_METRICS_GAUGE pGauge = NULL;
    PVM_METRICS_HISTOGRAM pHistogram = NULL;

    PSTR pszData = NULL;
    DWORD allocatedSize = BUFFER_SIZE;
    DWORD dataLen = 0;

    dwError = LwRtlCreateHashMap(&pNamesMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    dwError = VmMetricsAllocateMemory(
            BUFFER_SIZE,
            (PVOID*)&pszData);
    BAIL_ON_VM_METRICS_ERROR(dwError);

    pthread_rwlock_rdlock(&pContext->rwLock);
    pEntry = pContext->pMetrics;
    while (pEntry)
    {
        switch (pEntry->type)
        {
            case VM_METRICS_TYPE_COUNTER:
            {
                pCounter = (PVM_METRICS_COUNTER)pEntry->pData;

                if (LwRtlHashMapFindKey(pNamesMap, NULL, pCounter->pszName) == 0)
                {
                    bNameUsed = TRUE;
                }
                else
                {
                    dwError = LwRtlHashMapInsert(pNamesMap, pCounter->pszName, 0, NULL);
                    BAIL_ON_VM_METRICS_ERROR(dwError);
                }

                bufLen = VmMetricsGetCounterData(pCounter, NULL, 0, 0, bNameUsed);

                while (dataLen + bufLen > allocatedSize)
                {
                    allocatedSize += BUFFER_SIZE;
                    realloc = TRUE;
                }
                if (realloc)
                {
                    dwError = VmMetricsReallocateMemory(
                                (PVOID)pszData,
                                (PVOID*)&pszData,
                                allocatedSize);
                    BAIL_ON_VM_METRICS_ERROR(dwError);
                    realloc = FALSE;
                }

                dataLen += VmMetricsGetCounterData(pCounter, pszData, dataLen, bufLen, bNameUsed);
                dataLen--;
                bNameUsed = FALSE;

                break;
            }

            case VM_METRICS_TYPE_GAUGE:
            {
                pGauge = (PVM_METRICS_GAUGE)pEntry->pData;

                if (LwRtlHashMapFindKey(pNamesMap, NULL, pGauge->pszName) == 0)
                {
                    bNameUsed = TRUE;
                }
                else
                {
                    dwError = LwRtlHashMapInsert(pNamesMap, pGauge->pszName, 0, NULL);
                    BAIL_ON_VM_METRICS_ERROR(dwError);
                }

                bufLen = VmMetricsGetGaugeData(pGauge, NULL, 0, 0, bNameUsed);

                while (dataLen + bufLen > allocatedSize)
                {
                    allocatedSize += BUFFER_SIZE;
                    realloc = TRUE;
                }
                if (realloc)
                {
                    dwError = VmMetricsReallocateMemory(
                                (PVOID)pszData,
                                (PVOID*)&pszData,
                                allocatedSize);
                    BAIL_ON_VM_METRICS_ERROR(dwError);
                    realloc = FALSE;
                }

                dataLen += VmMetricsGetGaugeData(pGauge, pszData, dataLen, bufLen, bNameUsed);
                dataLen--;
                bNameUsed = FALSE;

                break;
            }

            case VM_METRICS_TYPE_HISTOGRAM:
            {
                pHistogram = (PVM_METRICS_HISTOGRAM)pEntry->pData;

                if (LwRtlHashMapFindKey(pNamesMap, NULL, pHistogram->pszName) == 0)
                {
                    bNameUsed = TRUE;
                }
                else
                {
                    dwError = LwRtlHashMapInsert(pNamesMap, pHistogram->pszName, 0, NULL);
                    BAIL_ON_VM_METRICS_ERROR(dwError);
                }

                bufLen = VmMetricsGetHistogramData(pHistogram, NULL, 0, 0, bNameUsed);

                while (dataLen + bufLen > allocatedSize)
                {
                    allocatedSize += BUFFER_SIZE;
                    realloc = TRUE;
                }
                if (realloc)
                {
                    dwError = VmMetricsReallocateMemory(
                                (PVOID)pszData,
                                (PVOID*)&pszData,
                                allocatedSize);
                    BAIL_ON_VM_METRICS_ERROR(dwError);
                    realloc = FALSE;
                }

                dataLen += VmMetricsGetHistogramData(pHistogram, pszData, dataLen, bufLen, bNameUsed);
                dataLen--;
                bNameUsed = FALSE;

                break;
            }

            default:
                break;
        }

        pEntry = pEntry->pNext;
    }
    pthread_rwlock_unlock(&pContext->rwLock);

    *ppszData = pszData;
    *pDataLen = dataLen;

cleanup:
    LwRtlHashMapClear(pNamesMap, VmMetricsNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pNamesMap);
    return dwError;

error:
    VM_METRICS_SAFE_FREE_MEMORY(pszData);
    goto cleanup;
}

/*
 * Free the data returned by VmMetricsGetPrometheusData()
 */
VOID
VmMetricsFreePrometheusData(
    PSTR pszData
    )
{
    VM_METRICS_SAFE_FREE_MEMORY(pszData);
}

/*
 * Destroy the metrics context structure
 */
VOID
VmMetricsDestroy(
    PVM_METRICS_CONTEXT pContext
    )
{
    PVM_METRICS_LIST_ENTRY pEntry = NULL;
    PVM_METRICS_COUNTER pCounter = NULL;
    PVM_METRICS_GAUGE pGauge = NULL;
    PVM_METRICS_HISTOGRAM pHistogram = NULL;

    if (pContext)
    {
        while (pContext->pMetrics)
        {
            pEntry = pContext->pMetrics;

            switch (pEntry->type)
            {
                case VM_METRICS_TYPE_COUNTER:
                {
                    pCounter = pEntry->pData;
                    VM_METRICS_SAFE_FREE_MEMORY(pCounter->pszName);
                    VM_METRICS_SAFE_FREE_MEMORY(pCounter->pszLabel);
                    VM_METRICS_SAFE_FREE_MEMORY(pCounter->pszDescription);
                    VM_METRICS_SAFE_FREE_MEMORY(pCounter);
                    break;
                }
                case VM_METRICS_TYPE_GAUGE:
                {
                    pGauge = pEntry->pData;
                    VM_METRICS_SAFE_FREE_MEMORY(pGauge->pszName);
                    VM_METRICS_SAFE_FREE_MEMORY(pGauge->pszLabel);
                    VM_METRICS_SAFE_FREE_MEMORY(pGauge->pszDescription);
                    VM_METRICS_SAFE_FREE_MEMORY(pGauge);
                    break;
                }
                case VM_METRICS_TYPE_HISTOGRAM:
                {
                    pHistogram = pEntry->pData;
                    VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pszName);
                    VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pszLabel);
                    VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pszDescription);
                    VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pBucketKeys);
                    VM_METRICS_SAFE_FREE_MEMORY(pHistogram->pBucketValues);
                    VM_METRICS_SAFE_FREE_MEMORY(pHistogram);
                    break;
                }
                default:
                {
                    break;
                }
            }

            pContext->pMetrics = pEntry->pNext;
            pEntry->pNext = NULL;
            VM_METRICS_SAFE_FREE_MEMORY(pEntry);
        }

        pthread_rwlock_destroy(&pContext->rwLock);
        VM_METRICS_SAFE_FREE_MEMORY(pContext);
    }
}

/*
 * Converting PVM_METRICS_LABEL into String Label
 */
static
DWORD
VmMetricsMakeLabel(
    PVM_METRICS_LABEL pLabel,
    DWORD dwLabelCount,
    PSTR* pszLabelOut
    )
{
    PSTR pszLabel = NULL;
    DWORD dwError = 0;
    DWORD len = 0;
    DWORD extraLen = 0;
    PSTR appendExtra = "";  // Null or comma character

    while (dwLabelCount)
    {
        if (!pLabel->pszKey || !pLabel->pszValue)
        {
            dwError = VM_METRICS_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_METRICS_ERROR(dwError);
        }
        if (!appendExtra)
        {
            // +4 in below line is for the characters '=' and 2 quotes '"' and '\0'
            len = strlen(pLabel->pszKey) + strlen(pLabel->pszValue) + 4;
        }
        else
        {
            // +5 in below line is for the characters appendExtra ',' and '=' and 2 quotes '"' and '\0'
            extraLen = strlen(pLabel->pszKey) + strlen(pLabel->pszValue) + 5;
        }
        len = len + extraLen;

        dwError = VmMetricsReallocateMemory(
                (PVOID)pszLabel,
                (PVOID*)&pszLabel,
                len);
        BAIL_ON_VM_METRICS_ERROR(dwError);

        snprintf( pszLabel+strlen(pszLabel), extraLen, "%s%s=\"%s\"",
                    appendExtra,
                    pLabel->pszKey,
                    pLabel->pszValue);

        appendExtra = ",";
        pLabel++;
        dwLabelCount--;
    }

    *pszLabelOut = pszLabel;

cleanup:
    return dwError;

error:
    VM_METRICS_SAFE_FREE_MEMORY(pszLabel);
    goto cleanup;
}

static
DWORD
VmMetricsGetCounterData(
    PVM_METRICS_COUNTER pCounter,
    PSTR pszData,
    DWORD dwDataLen,
    DWORD dwBufLen,
    BOOLEAN bNameUsed
    )
{
    DWORD len = 0;

    if (!bNameUsed)
    {
        len += snprintf( pszData+dwDataLen, dwBufLen, "# HELP %s %s\n# TYPE %s counter\n",
                    pCounter->pszName,
                    pCounter->pszDescription,
                    pCounter->pszName);
    }

    if (pCounter->pszLabel)
    {
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s{%s} %" PRIu64 "\n",
                    pCounter->pszName,
                    pCounter->pszLabel,
                    pCounter->value);
    }
    else
    {
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s %" PRIu64 "\n",
                    pCounter->pszName,
                    pCounter->value);
    }

    return len+1;
}

static
DWORD
VmMetricsGetGaugeData(
    PVM_METRICS_GAUGE pGauge,
    PSTR pszData,
    DWORD dwDataLen,
    DWORD dwBufLen,
    BOOLEAN bNameUsed
    )
{
    DWORD len = 0;

    if (!bNameUsed)
    {
        len += snprintf( pszData+dwDataLen, dwBufLen, "# HELP %s %s\n# TYPE %s gauge\n",
                    pGauge->pszName,
                    pGauge->pszDescription,
                    pGauge->pszName);
    }

    if (pGauge->pszLabel)
    {
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s{%s} %" PRIu64 "\n",
                    pGauge->pszName,
                    pGauge->pszLabel,
                    pGauge->value);
    }
    else
    {
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s %" PRIu64 "\n",
                    pGauge->pszName,
                    pGauge->value);
    }

    return len+1;
}

static
DWORD
VmMetricsGetHistogramData(
    PVM_METRICS_HISTOGRAM pHistogram,
    PSTR pszData,
    DWORD dwDataLen,
    DWORD dwBufLen,
    BOOLEAN bNameUsed
    )
{
    DWORD len = 0;
    int i = 0;

    if (!bNameUsed)
    {
        len += snprintf( pszData+dwDataLen, dwBufLen, "# HELP %s %s\n# TYPE %s histogram\n",
                    pHistogram->pszName,
                    pHistogram->pszDescription,
                    pHistogram->pszName);
    }

    for (i=0; i<pHistogram->bucketSize; i++)
    {
        if (pHistogram->pszLabel)
        {
            len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s_bucket{le=\"%" PRIu64 "\",%s} %" PRIu64 "\n",
                        pHistogram->pszName,
                        pHistogram->pBucketKeys[i],
                        pHistogram->pszLabel,
                        pHistogram->pBucketValues[i]);
        }
        else
        {
            len +=snprintf( pszData+dwDataLen+len, dwBufLen, "%s_bucket{le=\"%" PRIu64 "\"} %" PRIu64 "\n",
                        pHistogram->pszName,
                        pHistogram->pBucketKeys[i],
                        pHistogram->pBucketValues[i]);
        }
    }

    if (pHistogram->pszLabel)
    {
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s_bucket{le=\"+Inf\",%s} %" PRIu64 "\n",
                    pHistogram->pszName,
                    pHistogram->pszLabel,
                    pHistogram->count);
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s_count{%s} %" PRIu64 "\n",
                    pHistogram->pszName,
                    pHistogram->pszLabel,
                    pHistogram->count);
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s_sum{%s} %" PRIu64 "\n",
                    pHistogram->pszName,
                    pHistogram->pszLabel,
                    pHistogram->sum);
    }
    else
    {
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s_bucket{le=\"+Inf\"} %" PRIu64 "\n",
                    pHistogram->pszName,
                    pHistogram->count);
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s_count %" PRIu64 "\n",
                    pHistogram->pszName,
                    pHistogram->count);
        len += snprintf( pszData+dwDataLen+len, dwBufLen, "%s_sum %" PRIu64 "\n",
                    pHistogram->pszName,
                    pHistogram->sum);
    }

    return len+1;
}

static
VOID
VmMetricsNoopHashMapPairFree(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID pUnused
    )
{
    return;
}
