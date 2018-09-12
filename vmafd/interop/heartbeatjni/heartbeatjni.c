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

static
DWORD
JniPopulateJavaHeartbeatStatusNative(
        JNIEnv *env,
        jobject jEntry,
        PVMAFD_HB_STATUS_W pHeartbeatStatus
        );

static
DWORD
JniAddEntriesToList(
        JNIEnv *env,
        PVMAFD_HB_INFO_W pwEntries,
        DWORD dwCount,
        jobject *pjList
        );

static
void
JniFreeHeartbeatInfoList(
        JNIEnv *env,
        jobject jList,
        DWORD dwCount
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

JNIEXPORT jint JNICALL
Java_com_vmware_identity_heartbeat_VmAfdHeartbeatAdapter_VmAfdGetHeartbeatStatusW(
        JNIEnv *env,
        jclass clazz,
        jobject jpServer,
        jobject jpHeartbeatStatus
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PVMAFD_HB_STATUS_W pHbStatus = NULL;

    if (jpServer == NULL || jpHeartbeatStatus == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    dwError = VmAfdGetHeartbeatStatusW(pServer, &pHbStatus);
    BAIL_ON_ERROR(dwError);

    dwError = JniPopulateJavaHeartbeatStatusNative(
                                    env,
                                    jpHeartbeatStatus,
                                    pHbStatus);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pHbStatus)
    {
        VmAfdFreeHeartbeatStatusW(pHbStatus);
    }

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

static
DWORD
JniPopulateJavaHeartbeatStatusNative(
        JNIEnv *env,
        jobject jEntry,
        PVMAFD_HB_STATUS_W pHeartbeatStatus
        )
{
    DWORD dwError = 0;
    jfieldID fidIsAlive = NULL;
    jfieldID fidCount = NULL;
    jfieldID fidHbInfoArr = NULL;
    jobject jList = NULL;

    jclass jEntryClass = (*env)->GetObjectClass(env, jEntry);
    if (jEntryClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidIsAlive = (*env)->GetFieldID(env, jEntryClass, "isAlive", "I");
    if (fidIsAlive == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }
    (*env)->SetIntField(env, jEntry, fidIsAlive, (jint)(pHeartbeatStatus->bIsAlive));

    fidCount = (*env)->GetFieldID(env, jEntryClass, "dwCount", "I");
    if (fidCount == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }
    (*env)->SetIntField(env, jEntry, fidCount, (jint)(pHeartbeatStatus->dwCount));

    fidHbInfoArr = (*env)->GetFieldID(env, jEntryClass, "hbInfoArr", "Ljava/util/List;");
    if (fidHbInfoArr == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    // Get List of HeartbeatInfoNative
    dwError = JniAddEntriesToList(
                        env,
                        pHeartbeatStatus->pHeartbeatInfoArr,
                        pHeartbeatStatus->dwCount,
                        &jList);
    BAIL_ON_ERROR(dwError);

    (*env)->SetObjectField(env, jEntry, fidHbInfoArr, jList);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
JniAddEntriesToList(
        JNIEnv *env,
        PVMAFD_HB_INFO_W pwEntries,
        DWORD dwCount,
        jobject *pjList
        )
{
    DWORD dwError = 0;
    jmethodID mid = NULL;
    jmethodID jArrayInit = NULL;
    jfieldID fidPort = NULL;
    jfieldID fidLastHeartbeat = NULL;
    jfieldID fidIsAlive = NULL;
    jfieldID fidServiceName = NULL;
    jstring  jStr = NULL;
    jclass jEntryClass = NULL;
    jclass jListClass = NULL;
    jobject jList = NULL;
    jobject jEntry = NULL;
    jboolean jbSuccess = JNI_FALSE;
    DWORD dwIndex = 0;
    size_t size = 0;

    if (pjList == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    jListClass = (*env)->FindClass(env, "java/util/ArrayList");
    if (jListClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    jArrayInit = (*env)->GetMethodID(env, jListClass, "<init>", "()V");
    if (jArrayInit == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    jList = (*env)->NewObject(env, jListClass, jArrayInit);
    if (jList == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    mid = (*env)->GetMethodID(env, jListClass, "add", "(Ljava/lang/Object;)Z");
    if (mid == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    jEntryClass = (*env)->FindClass(env, "com/vmware/identity/heartbeat/HeartbeatInfoNative");
    if (jEntryClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidPort = (*env)->GetFieldID(env, jEntryClass, "port", "I");
    if (fidPort == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidLastHeartbeat = (*env)->GetFieldID(env, jEntryClass, "lastHeartbeat", "I");
    if (fidLastHeartbeat == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidIsAlive = (*env)->GetFieldID(env, jEntryClass, "isAlive", "I");
    if (fidIsAlive == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidServiceName = (*env)->GetFieldID(env, jEntryClass, "serviceName", "Ljava/lang/String;");
    if (fidServiceName == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    dwIndex = 0;
    for (; dwIndex < dwCount; dwIndex++)
    {
        PVMAFD_HB_INFO_W pwEntry = &pwEntries[dwIndex];
        jEntry = (*env)->AllocObject(env, jEntryClass);
        if (jEntry == NULL)
        {
            dwError = ERROR_BAD_UNIT;
            BAIL_ON_ERROR(dwError);
        }

        (*env)->SetIntField(env, jEntry, fidPort, (jint)(pwEntry->dwPort));
        (*env)->SetIntField(env, jEntry, fidLastHeartbeat, (jint)(pwEntry->dwLastHeartbeat));
        (*env)->SetIntField(env, jEntry, fidIsAlive, (jint)(pwEntry->bIsAlive));

        dwError = VmAfdGetStringLengthW(pwEntry->pszServiceName, &size);
        BAIL_ON_ERROR(dwError);

        jStr = (*env)->NewString(env, pwEntry->pszServiceName, size);
        (*env)->SetObjectField(env, jEntry, fidServiceName, jStr);

        jbSuccess = (*env)->CallBooleanMethod(env, jList, mid, jEntry);
        if (jbSuccess != JNI_TRUE)
        {
            dwError = ERROR_BAD_UNIT;
            BAIL_ON_ERROR(dwError);
        }
        jEntry = NULL;
    }

    *pjList = jList;

cleanup:
    return dwError;

error:
    if (jList)
    {
        JniFreeHeartbeatInfoList(env, jList, dwCount);
    }

    if (jEntry)
    {
        (*env)->DeleteLocalRef(env, jEntry);
    }

    goto cleanup;
}

static
void
JniFreeHeartbeatInfoList(
        JNIEnv *env,
        jobject jList,
        DWORD dwCount
        )
{
    jmethodID midArrayGet = NULL;
    jclass jListClass = NULL;
    jobject jEntry = NULL;
    DWORD dwIndex = 0;

    if (jList == NULL)
    {
        return;
    }

    jListClass = (*env)->FindClass(env, "java/util/ArrayList");
    if (jListClass == NULL)
    {
        return;
    }

    midArrayGet = (*env)->GetMethodID(env, jListClass, "get", "Ljava/lang/Object;");
    if (midArrayGet == NULL) {
        return;
    }

    for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        jEntry = (*env)->CallObjectMethod(env, jList, midArrayGet, dwIndex);
        if (jEntry != NULL)
        {
            (*env)->DeleteLocalRef(env, jEntry);
        }

        if ((*env)->ExceptionCheck(env)) {
            break;
        }
    }

    (*env)->DeleteLocalRef(env, jList);
}

