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
JniSetDword(
    JNIEnv  *env,
    jobject jObj,
    DWORD   dwValue,
    PCSTR    pszPropName
    );

static
DWORD
JniAddStringsToList(
        JNIEnv *env,
        jobject jList,
        PWSTR* ppwszStringArray,
        DWORD dwCount
        );

static
DWORD
JniPopulateJavaDCEntryNative(
        JNIEnv *env,
        jobject jEntry,
        PCDC_DC_INFO_W pwEntry
        );


JNIEXPORT jint JNICALL
Java_com_vmware_identity_cdc_CdcAdapter_VmAfdOpenServerW(
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
Java_com_vmware_identity_cdc_CdcAdapter_CdcEnableClientAffinity(
        JNIEnv  *env,
        jobject clazz,
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

    dwError = CdcEnableClientAffinity(
            pServer
            );
    BAIL_ON_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_cdc_CdcAdapter_CdcDisableClientAffinity(
        JNIEnv  *env,
        jobject clazz,
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

    dwError = CdcDisableClientAffinity(
            pServer
            );
    BAIL_ON_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}


JNIEXPORT jint JNICALL
Java_com_vmware_identity_cdc_CdcAdapter_CdcGetDCNameW(
        JNIEnv  *env,
        jobject clazz,
        jobject jpServer,
        jstring jDomainName,
        jint    jFlags,
        jobject jpDCInfo
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PCWSTR pwszDomainName = NULL;
    DWORD  dwFlags = 0;
    PCDC_DC_INFO_W pDomainControllerInfoW = NULL;

    if (jpServer == NULL || jpDCInfo == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    if (jDomainName != NULL)
    {
        pwszDomainName = (*env)->GetStringChars(env, jDomainName, NULL);
    }

    dwFlags = jFlags;


    //TODO: Change CdcGetDCNameW to take const
    dwError = CdcGetDCNameW(
                      pServer,
                      (PWSTR)pwszDomainName,
                      NULL,
                      NULL,
                      dwFlags,
                      &pDomainControllerInfoW
                      );
    BAIL_ON_ERROR(dwError);

    dwError = JniPopulateJavaDCEntryNative(
                                          env,
                                          jpDCInfo,
                                          pDomainControllerInfoW
                                          );
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszDomainName)
    {
        (*env)->ReleaseStringChars(env, jDomainName, pwszDomainName);
    }
    if (pDomainControllerInfoW)
    {
        CdcFreeDomainControllerInfoW(pDomainControllerInfoW);
    }

    return dwError;

error:

    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_cdc_CdcAdapter_CdcEnumDCEntriesW(
        JNIEnv *env,
        jclass clazz,
        jobject jpServer,
        jobject jStringList
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PWSTR* ppwszDCEntriesArray = NULL;
    DWORD dwCount = 0;

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    if (jStringList == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = CdcEnumDCEntriesW(
                            pServer,
                            &ppwszDCEntriesArray,
                            &dwCount
                            );
    BAIL_ON_ERROR(dwError);

    dwError = JniAddStringsToList(env, jStringList, ppwszDCEntriesArray, dwCount);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (ppwszDCEntriesArray)
    {
        VmAfdFreeStringArrayW(ppwszDCEntriesArray, dwCount);
    }

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_cdc_CdcAdapter_CdcGetCurrentState(
        JNIEnv  *env,
        jclass  clazz,
        jobject jpServer,
        jobject pState
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    CDC_DC_STATE  cdcState = CDC_DC_STATE_UNDEFINED;

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    if (pState == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = CdcGetCurrentState(
                            pServer,
                            &cdcState
                            );
    BAIL_ON_ERROR(dwError);

    dwError = JniSetDword(env, pState, (DWORD)cdcState ,"number" );
    BAIL_ON_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;

}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_cdc_CdcAdapter_CdcFreeDomainControllerInfoW(
        JNIEnv  *env,
        jclass  clazz,
        jobject jpDCInfo
        )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pDomainControllerInfoW = NULL;

    if (jpDCInfo == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpDCInfo, (PVOID*)&pDomainControllerInfoW);
    BAIL_ON_ERROR(dwError);

    jpDCInfo = NULL;

    if (pDomainControllerInfoW)
    {
        CdcFreeDomainControllerInfoW(
                            pDomainControllerInfoW
                            );
    }
cleanup:

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_cdc_CdcAdapter_VmAfdCloseServer(
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

static
DWORD
JniSetDword(
    JNIEnv  *env,
    jobject jObj,
    DWORD   dwValue,
    PCSTR    pszPropName
    )
{
    DWORD dwError = 0;
    jfieldID fid = NULL;
    jlong jVal = 0;

    jclass jObjClass = (*env)->GetObjectClass(env, jObj);

    if (jObjClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }
    fid = (*env)->GetFieldID(env, jObjClass, pszPropName, "I");
    if (fid == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    jVal = (jint)dwValue;
    (*env)->SetIntField(env, jObj, fid, jVal);

cleanup:
    return dwError;

error:
    goto cleanup;
}


static
DWORD
JniAddStringsToList(
        JNIEnv *env,
        jobject jList,
        PWSTR* ppwszStringArray,
        DWORD dwCount
        )
{
    DWORD dwError = 0;
    jmethodID midPointer = NULL;
    jstring jName = NULL;
    jboolean jbSuccess = JNI_FALSE;
    DWORD dwIndex = 0;
    size_t size;

    jclass jListClass = (*env)->GetObjectClass(env, jList);

    if (jListClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    midPointer = (*env)->GetMethodID(env, jListClass, "add", "(Ljava/lang/Object;)Z");
    if (midPointer == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    dwIndex = 0;
    for (; dwIndex < dwCount; dwIndex++)
    {
        dwError = VmAfdGetStringLengthW(ppwszStringArray[dwIndex], &size);
        BAIL_ON_ERROR(dwError);
        jName = (*env)->NewString(env, ppwszStringArray[dwIndex], size);
        jbSuccess = (*env)->CallBooleanMethod(env, jList, midPointer, jName);
        if (jbSuccess != JNI_TRUE)
        {
            dwError = ERROR_BAD_UNIT;
            BAIL_ON_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
JniPopulateJavaDCEntryNative(
        JNIEnv *env,
        jobject jEntry,
        PCDC_DC_INFO_W pwEntry
        )
{
    DWORD dwError = 0;
    jfieldID fidAddressType = NULL;
    jfieldID fidDCName = NULL;
    jfieldID fidDCAddress = NULL;
    jfieldID fidDCSiteName = NULL;
    jstring jStr = NULL;
    size_t size = 0;

    jclass jEntryClass = (*env)->GetObjectClass(env, jEntry);
    if (jEntryClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidAddressType = (*env)->GetFieldID(env, jEntryClass, "addressType", "I");
    if (fidAddressType == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidDCName = (*env)->GetFieldID(env, jEntryClass, "dcName", "Ljava/lang/String;");
    if (fidDCName == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidDCAddress = (*env)->GetFieldID(env, jEntryClass, "dcAddress", "Ljava/lang/String;");
    if (fidDCAddress == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidDCSiteName = (*env)->GetFieldID(env, jEntryClass, "dcSiteName", "Ljava/lang/String;");
    if (fidDCSiteName == NULL)
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }


    (*env)->SetIntField(env, jEntry, fidAddressType, (jint)(pwEntry->DcAddressType));

    dwError = VmAfdGetStringLengthW(pwEntry->pszDCName, &size);
    BAIL_ON_ERROR(dwError);
    jStr = (*env)->NewString(env, pwEntry->pszDCName, size);
    (*env)->SetObjectField(env, jEntry, fidDCName, jStr);

    if (pwEntry->pszDCAddress)
    {
        dwError = VmAfdGetStringLengthW(pwEntry->pszDCAddress, &size);
        BAIL_ON_ERROR(dwError);
        jStr = (*env)->NewString(env, pwEntry->pszDCAddress, size);
    }
    else
    {
        jStr = NULL;
    }
    (*env)->SetObjectField(env, jEntry, fidDCAddress, jStr);

    if (pwEntry->pszDcSiteName)
    {
        dwError = VmAfdGetStringLengthW(pwEntry->pszDcSiteName, &size);
        BAIL_ON_ERROR(dwError);
        jStr = (*env)->NewString(env, pwEntry->pszDcSiteName, size);
    }
    else
    {
        jStr = NULL;
    }
    (*env)->SetObjectField(env, jEntry, fidDCSiteName, jStr);

cleanup:
    return dwError;

error:
    goto cleanup;
}
