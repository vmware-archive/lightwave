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

#include "includes.h"

 #define BAIL_ON_ERROR(dwError) do { if (dwError) goto error; } while(0)

#define ERROR_INVALID_FUNCTION  1
#define ERROR_BAD_UNIT          20
#define ERROR_INVALID_PARAMETER 87
#define ERROR_INVALID_DATATYPE  1804
#define ERROR_OBJEC_NOT_FOUND   4312
#define ERROR_FILE_NOT_FOUND    2
#define ERROR_ALREADY_EXISTS    183

static
DWORD
JniGetPointer(
    JNIEnv  *env,
    jobject jpStore,
    PVOID*  pPointer
    );

static
DWORD
JniSetPointer(
    JNIEnv  *env,
    jobject jpStore,
    PVOID   pointer
    );

JNIEXPORT jint JNICALL
Java_com_vmware_identity_heartbeat_VmAfdHeartbeatAdapter_VmAfdOpenServerW(
        JNIEnv  *env,
        jobject clazz,
        jstring jServerName,
        jstring jUserName,
        jstring jPassword,
        jobject jpServer
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PCWSTR pwszServerName = NULL;
    PCWSTR pwszUserName = NULL;
    PCWSTR pwszPassword = NULL;

#ifndef _WIN32
    setlocale (LC_ALL, "");
#endif

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (jServerName != NULL)
    {
        pwszServerName = (*env)->GetStringChars(env, jServerName, NULL);
    }

    if (jUserName != NULL)
    {
       pwszUserName = (*env)->GetStringChars(env, jUserName, NULL);
    }

    if (jPassword != NULL)
    {
        pwszPassword = (*env)->GetStringChars(env, jPassword, NULL);
    }

    dwError = VmAfdOpenServerW(
            pwszServerName,
            pwszUserName,
            pwszPassword,
            &pServer
            );
    BAIL_ON_ERROR(dwError);

    dwError = JniSetPointer(env, jpServer, (PVOID)pServer);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszServerName)
    {
        (*env)->ReleaseStringChars(env, jServerName, pwszServerName);
    }
    if (pwszUserName)
    {
        (*env)->ReleaseStringChars(env, jUserName, pwszUserName);
    }
    if (pwszPassword)
    {
        (*env)->ReleaseStringChars(env, jPassword, pwszPassword);
    }

    return dwError;

error:
    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_heartbeat_VmAfdHeartbeatAdapter_VmAfdStartHeartBeatW(
        JNIEnv  *env,
        jobject clazz,
        jstring jServiceName,
        jint    jPort,
        jobject jpHandle
        )
{
    DWORD dwError = 0;
    PVMAFD_HB_HANDLE pHandle = NULL;
    PCWSTR pwszServiceName = NULL;
    DWORD dwPort = 0;

    if (jpHandle == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (jServiceName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    else
    {
        pwszServiceName = (*env)->GetStringChars(env, jServiceName, NULL);
    }

    dwPort = (DWORD) jPort;

    dwError = VmAfdStartHeartbeatW(
            pwszServiceName,
            dwPort,
            &pHandle
            );
    BAIL_ON_ERROR(dwError);

    dwError = JniSetPointer(env, jpHandle, (PVOID)pHandle);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszServiceName)
    {
        (*env)->ReleaseStringChars(env, jServiceName, pwszServiceName);
    }

    return dwError;

error:
    if (pHandle)
    {
        VmAfdStopHeartbeat(pHandle);
        pHandle = NULL;
    }
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_heartbeat_VmAfdHeartbeatAdapter_VmAfdStopHeartbeat(
        JNIEnv  *env,
        jobject clazz,
        jobject jpHandle
        )
{
    DWORD dwError = 0;
    PVMAFD_HB_HANDLE pHandle = NULL;

    if (jpHandle == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpHandle, (PVOID*)&pHandle);
    BAIL_ON_ERROR(dwError);

    VmAfdStopHeartbeat(pHandle);

    pHandle = NULL;
    dwError = JniSetPointer(env, jpHandle, (PVOID)pHandle);
    BAIL_ON_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_heartbeat_VmAfdHeartbeatAdapter_VmAfdCloseServer(
        JNIEnv  *env,
        jclass  clazz,
        jobject jpServer
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    VmAfdCloseServer(pServer);
    pServer = NULL;

    dwError = JniSetPointer(env, jpServer, (PVOID)pServer);
    BAIL_ON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

///Helper methods
static
DWORD
JniGetPointer(
    JNIEnv  *env,
    jobject jpStore,
    PVOID*   pPointer
    )
{
    DWORD dwError = 0;
    jfieldID fidPointer = NULL;
    jclass jPointerClass = NULL;
    jlong address = 0;
    *pPointer = NULL;

    if (jpStore == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    jPointerClass = (*env)->GetObjectClass(env, jpStore);
    if (jPointerClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidPointer = (*env)->GetFieldID(env, jPointerClass, "pointer", "J");
    if (fidPointer == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    address = (*env)->GetLongField(env, jpStore, fidPointer);
    if (address == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    *pPointer = (PVOID)address;
cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
JniSetPointer(
    JNIEnv  *env,
    jobject jpStore,
    PVOID   pointer
    )
{
    DWORD dwError = 0;
    jfieldID fidPointer = NULL;
    jlong address = 0;

    jclass jPointerClass = (*env)->GetObjectClass(env, jpStore);

    if (jPointerClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }
    fidPointer = (*env)->GetFieldID(env, jPointerClass, "pointer", "J");
    if (fidPointer == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    address = (jlong)pointer;
    (*env)->SetLongField(env, jpStore, fidPointer, address);

cleanup:
    return dwError;

error:
    goto cleanup;
}
