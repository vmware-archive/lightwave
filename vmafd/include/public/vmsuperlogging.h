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




#ifndef __VMAFD_SUPER_LOGGING_INTERFACE_H__
#define __VMAFD_SUPER_LOGGING_INTERFACE_H__

#include <dce/dcethread.h>
#include <vmafddefines.h>

#define MAX_SEARCH_OPT_LEN  16
#define MAX_PARAMETERS_LEN  512

typedef struct _VMAFD_CIRCULAR_BUFFER
{
    //
    // Maximum number of entries.
    //
    DWORD dwCapacity;

    //
    // The spot where we will write the next entry.
    //
    DWORD dwHead;

    //
    // Current number of entries.
    //
    DWORD dwSize;

    //
    // Size of individual elements in the buffer.
    //
    DWORD dwElementSize;

    //
    // Actual objects.
    //
    PBYTE CircularBuffer;

    //
    // Lock for making our operations thread-safe.
    //
    //pthread_mutex_t    mutex;
} VMAFD_CIRCULAR_BUFFER, *PVMAFD_CIRCULAR_BUFFER;

typedef const VMAFD_CIRCULAR_BUFFER* PCVMAFD_CIRCULAR_BUFFER;
typedef BOOLEAN (*CIRCULAR_BUFFER_SELECT_CALLBACK)(PVOID pElement, PVOID pContext);

typedef struct _VMSUPERLOGGING
{
    BOOLEAN bEnabled;
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer;
} VMSUPERLOGGING, *PVMSUPERLOGGING;


#define VMAFD_SUPERLOG_TABLE_COL_NUM   5

typedef enum
{
    DC_NAME,
    CDC_STATE,
    ALIVE_STATUS,
    SL_ERROR_CODE,
    SL_AVG_TIME
} VMAFD_SUPERLOG_TABLE_COLUMN;

typedef struct _VMAFD_SUPERLOG_TABLE_COLUMN_SET
{
    unsigned int isColumnSet[VMAFD_SUPERLOG_TABLE_COL_NUM];
} VMAFD_SUPERLOG_TABLE_COLUMN_SET, *PVMAFD_SUPERLOG_TABLE_COLUMN_SET;

typedef struct _VMAFD_SUPERLOG_TABLE_ROW
{
    char* colVals[VMAFD_SUPERLOG_TABLE_COL_NUM];
    UINT64 totalTime;
    UINT32 count;
} VMAFD_SUPERLOG_TABLE_ROW, *PVMAFD_SUPERLOG_TABLE_ROW;

typedef struct _VMAFD_SUPERLOG_TABLE
{
    UINT32 numRows;
    PVMAFD_SUPERLOG_TABLE_COLUMN_SET cols;
    PVMAFD_SUPERLOG_TABLE_ROW rows;
} VMAFD_SUPERLOG_TABLE, *PVMAFD_SUPERLOG_TABLE;


typedef struct _VMAFD_MUTEX
{
    BOOLEAN bInitialized;
    pthread_mutex_t critSect;
} VMAFD_MUTEX, *PVMAFD_MUTEX;

typedef struct _VMAFD_COND
{
    BOOLEAN bInitialized;
    pthread_cond_t cond;
} VMAFD_COND, *PVMAFD_COND;


typedef struct _VAFD_THREAD_INFO
{
    pthread_t                   tid;
    BOOLEAN                     bJoinThr;       // join by main thr

    // mutexUsed is real mutex used (i.e. it may not == mutex)
    pthread_mutex_t*    mutex;
    pthread_mutex_t*    mutexUsed;

    // conditionUsed is real condition used (i.e. it may not == condition)
    PVMAFD_COND               condition;
    PVMAFD_COND               conditionUsed;

    struct _VAFD_THREAD_INFO*   pNext;

} VMSUPERLOG_THREAD_INFO, *PVMSUPERLOG_THREAD_INFO;


typedef DWORD (VmAfdStartRoutine)(PVOID);
typedef VmAfdStartRoutine* PVMAFD_START_ROUTINE;

typedef struct _VMAFD_THREAD_START_INFO
{
    VmAfdStartRoutine* pStartRoutine;
    PVOID pArgs;
} VMAFD_THREAD_START_INFO, * PVMAFD_THREAD_START_INFO;


DWORD
VmAfdCircularBufferCreate(
    DWORD dwCapacity,
    DWORD dwElementSize,
    PVMAFD_CIRCULAR_BUFFER *ppCircularBuffer
    );

VOID VmAfdCircularBufferFree(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmAfdCircularBufferReset(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmAfdCircularBufferGetCapacity(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pdwCapacity
    );

DWORD
VmAfdCircularBufferSetCapacity(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCapacity
    );

PVOID
VmAfdCircularBufferGetNextEntry(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    );


DWORD
VmAfdSuperLoggingInit(
    PVMSUPERLOGGING *ppLogger
    );

DWORD
VmAfdEnableSuperLogging(
    PVMSUPERLOGGING pLogger
    );

DWORD
VmAfdDisableSuperLogging(
    PVMSUPERLOGGING pLogger
    );

BOOLEAN
VmAfdIsSuperLoggingEnabled(
    PVMSUPERLOGGING pLogger
    );

DWORD
VmAfdFlushSuperLogging(
    PVMSUPERLOGGING pLogger
    );

DWORD
VmAfdGetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    PDWORD pSize
    );

DWORD
VmAfdSetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    DWORD dwSize
    );

DWORD
VmAfdAddDBSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PCDC_DB_ENTRY_W pCdcEntry,
    DWORD dwErrorCode
    );

DWORD
VmAfdAddHBSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PVMAFD_HB_STATUS_W pCdcEntry,
    DWORD dwErrorCode
    );

DWORD
VmAfdAddDCSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PCDC_DC_INFO_W pDCEntry,
    DWORD dwErrorCode
    );

DWORD
VmAfdAddCDCSuperLogEntry(
    PVMSUPERLOGGING pLogger,
    UINT64 iStartTime,
    UINT64 iEndTime,
    PCDC_DC_INFO_W pDCEntry,
    CDC_DC_STATE dwState,
    DWORD dwErrorCode
    );

#endif
