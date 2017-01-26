/*
 * Copyright 2012-2016 VMware, Inc.  All Rights Reserved.
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

#ifndef KDC_SRV_COMMON_H_
#define KDC_SRV_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef tcp_close
#ifdef _WIN32
#define tcp_close( s )  (shutdown( s, SD_BOTH ), closesocket( s ))
#else
#define tcp_close( s )  (shutdown( s, SHUT_RDWR), close( s ))
#endif
#endif


#define VMKDC_ORIG_TIME_STR_LEN         ( 4 /* year */ + 2 /* month */ + 2 /* day */ + 2 /* hour */ + 2 /* minute */ + \
                                          2 /* sec */ + 1 /* . */ + 3 /* milli sec */ + 1 /* null byte terminator */ )

#define VMKDC_MAX_I64_ASCII_STR_LEN     (19 + 1 /* null byte terminator */) /* Max value for i64_t is 9,223,372,036,854,775,807 */
#define VMKDC_UUID_LEN                  16 /* typedef __darwin_uuid_t uuid_t; typedef unsigned char __darwin_uuid_t[16] */
#define VMKDC_MAX_USN_STR_LEN           VMKDC_MAX_I64_ASCII_STR_LEN
#define VMKDC_MAX_VERSION_NO_STR_LEN    VMKDC_MAX_I64_ASCII_STR_LEN /* Version number used in attribute meta-data */


#define VMKDC_GUID_STR_LEN             (32 + 4 /* -s */ + 1 /* \0 */) // "%08x-%04x-%04x-%04x-%04x%08x"

typedef struct _VMKDC_THREAD_INFO
{
    pthread_t                tid;
    BOOLEAN                     bJoinThr;       // join by main thr

    // mutexUsed is real mutex used (i.e. it may not == mutex)
    pthread_mutex_t mutex;
    pthread_mutex_t mutexUsed;

    // conditionUsed is real condition used (i.e. it may not == condition)
    pthread_cond_t               condition;
    pthread_cond_t               conditionUsed;

    struct _VMKDC_THREAD_INFO*   pNext;

} VMKDC_THREAD_INFO, *PVMKDC_THREAD_INFO;

/* util.c */

int
VmKdcQsortPPCHARCmp(
    const void*		ppStr1,
    const void*		ppStr2
    );

int
VmKdcQsortPEIDCmp(
    const void * pEID1,
    const void * pEID2);

const char *VmKdcCtimeTS(
    const time_t *tvalIn,
    char buf[]);

int
VmKdcGenOriginatingTimeStr(
    char * timeStr);


void
VmKdcCurrentGeneralizedTime(
    PSTR    pszTimeBuf,
    int     iBufSize);

VOID
VmKdcSleep(
    DWORD dwMilliseconds);

#ifdef VMDIR_ENABLE_PAC
/* pacops.c */

DWORD
VmKdcEncodeAuthzInfo(
    VMDIR_AUTHZ_INFO *pac,
    long *bufsiz,
    void **buf
    );

DWORD
VmKdcDecodeAuthzInfo(
    long bufsiz,
    void *buf,
    VMDIR_AUTHZ_INFO **pac
    );
#endif

#ifdef __cplusplus
}
#endif

#endif /* KDC_SRV_COMMON_H_ */
