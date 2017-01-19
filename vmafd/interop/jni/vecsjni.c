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
#include "vecsjni.h"

 #define BAIL_ON_ERROR(dwError) do { if (dwError) goto error; } while(0)

#define ERROR_INVALID_FUNCTION  1
#define ERROR_BAD_UNIT          20
#define ERROR_INVALID_PARAMETER 87
#define ERROR_INVALID_DATATYPE  1804

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
JniSetString(
    JNIEnv  *env,
    jobject jObj,
    PCWSTR  pwszValue,
    PCSTR   pszPropName
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
JniAddEntriesToList(
        JNIEnv *env,
        jobject jList,
        PVECS_CERT_ENTRY_W pwEntries,
        DWORD dwCount
        );

static
DWORD
JniAddPermissionsToList(
        JNIEnv *env,
        jobject jList,
        PVECS_STORE_PERMISSION_W pPermissions,
        DWORD dwCount
        );

static
DWORD
JniPopulateJavaVecsEntryNative(
        JNIEnv *env,
        jobject jEntry,
        PVECS_CERT_ENTRY_W pwEntry
        );


JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VmAfdOpenServerW(
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

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
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
Java_com_vmware_identity_vecs_VecsAdapter_VecsCreateCertStoreHW(
        JNIEnv  *env,
        jobject clazz,
        jobject jpServer,
        jstring jStoreName,
        jstring jPassword,
        jobject jpStore
        )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PVMAFD_SERVER pServer = NULL;
    PCWSTR pwszStoreName = NULL;
    PCWSTR pwszPassword = NULL;

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    if (jStoreName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    } else
    {
        pwszStoreName = (*env)->GetStringChars(env, jStoreName, NULL);
    }

    if (jPassword != NULL)
    {
        pwszPassword = (*env)->GetStringChars(env, jPassword, NULL);
    }

    dwError = VecsCreateCertStoreHW(
            pServer,
            pwszStoreName,
            pwszPassword,
            jpStore ? &pStore : NULL
            );
    BAIL_ON_ERROR(dwError);

    if (jpStore != NULL)
    {
        dwError = JniSetPointer(env, jpStore, (PVOID)pStore);
        BAIL_ON_ERROR(dwError);
    }

cleanup:
    if (pwszStoreName)
    {
        (*env)->ReleaseStringChars(env, jStoreName, pwszStoreName);
    }
    if (pwszPassword)
    {
        (*env)->ReleaseStringChars(env, jPassword, pwszPassword);
    }

    return dwError;

error:
    if (pStore)
    {
        DWORD dwError2 = 0;
        dwError2 = VecsCloseCertStore(pStore);
        pStore = NULL;
        // Need to log dwError2;
    }
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsOpenCertStoreHW(
        JNIEnv  *env,
        jobject clazz,
        jobject jpServer,
        jstring jStoreName,
        jstring jPassword,
        jobject jpStore
        )
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PVMAFD_SERVER pServer = NULL;
    PCWSTR pwszStoreName = NULL;
    PCWSTR pwszPassword = NULL;

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    if (jStoreName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    } else
    {
        pwszStoreName = (*env)->GetStringChars(env, jStoreName, NULL);
    }

    if (jPassword != NULL)
    {
        pwszPassword = (*env)->GetStringChars(env, jPassword, NULL);
    }

    if (jpStore == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VecsOpenCertStoreHW(
            pServer,
            pwszStoreName,
            pwszPassword,
            &pStore
            );
    BAIL_ON_ERROR(dwError);

    dwError = JniSetPointer(env, jpStore, (PVOID)pStore);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszStoreName)
    {
        (*env)->ReleaseStringChars(env, jStoreName, pwszStoreName);
    }
    if (pwszPassword)
    {
        (*env)->ReleaseStringChars(env, jPassword, pwszPassword);
    }

    return dwError;

error:
    if (pStore)
    {
        DWORD dwError2 = 0;
        dwError2 = VecsCloseCertStore(pStore);
        // Need to log dwError2;
    }

    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsEnumCertStoreHW(
        JNIEnv *env,
        jclass clazz,
        jobject jpServer,
        jobject jStringList
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PWSTR* ppwszStoreNameArray = NULL;
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

    dwError = VecsEnumCertStoreHW(
            pServer,
            &ppwszStoreNameArray,
            &dwCount
            );
    BAIL_ON_ERROR(dwError);

    dwError = JniAddStringsToList(env, jStringList, ppwszStoreNameArray, dwCount);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (ppwszStoreNameArray)
    {
        VmAfdFreeStringArrayW(ppwszStoreNameArray, dwCount);
    }

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsAddEntryW(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jint jEntityType,
        jstring jAlias,
        jstring jCertificate,
        jstring jPrivateKey,
        jstring jPassword,
        jboolean jAutoRefresh)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;
    PCWSTR pwszAlias = NULL;
    PCWSTR pwszCertificate = NULL;
    PCWSTR pwszPrivateKey = NULL;
    PCWSTR pwszPassword = NULL;
    BOOLEAN bAutoRefresh = FALSE;

    if (jpStore == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (jEntityType == 0) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    entryType = (CERT_ENTRY_TYPE) jEntityType;

    if (jAlias != NULL)
    {
       pwszAlias = (*env)->GetStringChars(env, jAlias, NULL);
    }

    if (jCertificate != NULL)
    {
       pwszCertificate = (*env)->GetStringChars(env, jCertificate, NULL);
    }

    if (jPrivateKey != NULL)
    {
       pwszPrivateKey = (*env)->GetStringChars(env, jPrivateKey, NULL);
    }

    if (pwszPassword != NULL)
    {
       pwszAlias = (*env)->GetStringChars(env, jPassword, NULL);
    }

    bAutoRefresh = (BOOLEAN) jAutoRefresh;

    dwError = VecsAddEntryW(
            pStore,
            entryType,
            pwszAlias,
            pwszCertificate,
            pwszPrivateKey,
            pwszPassword,
            bAutoRefresh
            );
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszAlias)
    {
        (*env)->ReleaseStringChars(env, jAlias, pwszAlias);
    }
    if (pwszCertificate)
    {
        (*env)->ReleaseStringChars(env, jCertificate, pwszCertificate);
    }
    if (pwszPrivateKey)
    {
        (*env)->ReleaseStringChars(env, jPrivateKey, pwszPrivateKey);
    }
    if (pwszPassword)
    {
        (*env)->ReleaseStringChars(env, jPassword, pwszPassword);
    }

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsGetEntryByAliasW(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jstring jAlias,
        jint jInfoLevel,
        jobject jEntry)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PCWSTR pwszAlias = NULL;
    ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_1;
    PVECS_CERT_ENTRY_W pwEntry = NULL;

    if (jpStore == NULL || jEntry == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (jInfoLevel != 0) {
        infoLevel = (ENTRY_INFO_LEVEL) jInfoLevel;
    }

    if (jAlias == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pwszAlias = (*env)->GetStringChars(env, jAlias, NULL);

    dwError = VecsGetEntryByAliasW(
            pStore,
            pwszAlias,
            infoLevel,
            &pwEntry
            );
    BAIL_ON_ERROR(dwError);

    dwError = JniPopulateJavaVecsEntryNative(env, jEntry, pwEntry);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszAlias)
    {
        (*env)->ReleaseStringChars(env, jAlias, pwszAlias);
    }

    if (pwEntry != NULL)
    {
       VecsFreeCertEntryW(pwEntry);
    }
    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsGetKeyByAliasW(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jstring jAlias,
        jstring jPassword,
        jobject jKey)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PCWSTR pwszAlias = NULL;
    PCWSTR pwszPassword = NULL;
    PWSTR pwszKey =NULL;

    if (jpStore == NULL || jKey == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (jAlias == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pwszAlias = (*env)->GetStringChars(env, jAlias, NULL);

    if (jPassword != NULL)
    {
        pwszPassword = (*env)->GetStringChars(env, jPassword, NULL);
    }

    dwError = VecsGetKeyByAliasW(
            pStore,
            pwszAlias,
            pwszPassword,
            &pwszKey
            );
    BAIL_ON_ERROR(dwError);

    dwError = JniSetString(env, jKey, pwszKey, "str");
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszAlias)
    {
        (*env)->ReleaseStringChars(env, jAlias, pwszAlias);
    }

    if (pwszPassword)
    {
        (*env)->ReleaseStringChars(env, jPassword, pwszPassword);
    }

    if (pwszKey)
    {
        VMAFD_SAFE_FREE_MEMORY(pwszKey);
    }
    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsGetEntryCount(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jobject pCount)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    DWORD dwCount = 0;

    if (jpStore == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (pCount == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VecsGetEntryCount(pStore, &dwCount);
    BAIL_ON_ERROR(dwError);

    dwError = JniSetDword(env, pCount, dwCount, "number");
    BAIL_ON_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsBeginEnumEntries(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jint jEntryCount,
        jint jInfoLevel,
        jobject jpEnumContext)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    DWORD dwEntryCount = 0;
    ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_UNDEFINED;
    PVECS_ENUM_CONTEXT pEnumContext = NULL;

    if (jpStore == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (jEntryCount <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwEntryCount = (DWORD) jEntryCount;

    if (jInfoLevel != 0)
    {
        infoLevel = (ENTRY_INFO_LEVEL) jInfoLevel;
    }

    if (jpEnumContext == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VecsBeginEnumEntries(pStore, dwEntryCount, infoLevel, &pEnumContext);
    BAIL_ON_ERROR(dwError);

    dwError = JniSetPointer(env, jpEnumContext, (PVOID)pEnumContext);
    BAIL_ON_ERROR(dwError);

cleanup:

    return dwError;

error:
    if (pEnumContext)
    {
        DWORD dwError2 = 0;
        dwError2 = VecsEndEnumEntries(pEnumContext);
        //Need to log dwError2
    }

    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsEnumEntriesW(
    JNIEnv *env,
    jclass clazz,
    jobject jpEnumContext,
    jobject jEntryList)
{
    DWORD dwError = 0;
    PVECS_ENUM_CONTEXT pEnumContext = NULL;
    PVECS_CERT_ENTRY_W pwEntries = NULL;
    DWORD dwCount = 0;

    if (jpEnumContext == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = JniGetPointer(env, jpEnumContext, (PVOID*)&pEnumContext);
    BAIL_ON_ERROR(dwError);

    if (jEntryList == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VecsEnumEntriesW(
            pEnumContext,
            &pwEntries,
            &dwCount
            );
    if (dwError == ERROR_NO_MORE_ITEMS)
    {
        dwError = 0;
    }
    BAIL_ON_ERROR(dwError);

    dwError = JniAddEntriesToList(env, jEntryList, pwEntries, dwCount);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwEntries)
    {
        VecsFreeCertEntryArrayW(pwEntries, dwCount);
    }
    return dwError;

error:
    goto cleanup;

}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsEndEnumEntries(
        JNIEnv *env,
        jclass clazz,
        jobject jpEnumContext)
{
    DWORD dwError = 0;
    PVECS_ENUM_CONTEXT pEnumContext = NULL;

    if (jpEnumContext == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = JniGetPointer(env, jpEnumContext, (PVOID*)&pEnumContext);
    BAIL_ON_ERROR(dwError);

    dwError = VecsEndEnumEntries(pEnumContext);
    BAIL_ON_ERROR(dwError);

    pEnumContext = NULL;
    dwError = JniSetPointer(env, jpEnumContext, (PVOID)pEnumContext);
    BAIL_ON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsDeleteEntryW(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jstring jAlias)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PCWSTR pwszAlias = NULL;

    if (jpStore == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (jAlias == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pwszAlias = (*env)->GetStringChars(env, jAlias, NULL);

    dwError = VecsDeleteEntryW(
            pStore,
            pwszAlias
            );
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszAlias)
    {
        (*env)->ReleaseStringChars(env, jAlias, pwszAlias);
    }
    return dwError;

error:
    goto cleanup;

}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsSetPermissionW(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jstring jUserName,
        jint jAccessMask)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PCWSTR pwszUserName = NULL;
    DWORD dwAccessMask = 0;

    if (jpStore == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (jUserName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pwszUserName = (*env)->GetStringChars(env, jUserName, NULL);

    if (jAccessMask != READ_STORE && jAccessMask != WRITE_STORE)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwAccessMask = (DWORD) jAccessMask;

    dwError = VecsSetPermissionW(
            pStore,
            pwszUserName,
            dwAccessMask
            );
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszUserName)
    {
        (*env)->ReleaseStringChars(env, jUserName, pwszUserName);
    }
    return dwError;

error:
    goto cleanup;

}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsRevokePermissionW(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jstring jUserName,
        jint jAccessMask)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PCWSTR pwszUserName = NULL;
    DWORD dwAccessMask = 0;

    if (jpStore == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    if (jUserName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pwszUserName = (*env)->GetStringChars(env, jUserName, NULL);

    if (jAccessMask != READ_STORE && jAccessMask != WRITE_STORE)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwAccessMask = (DWORD) jAccessMask;

    dwError = VecsRevokePermissionW(
            pStore,
            pwszUserName,
            dwAccessMask
            );
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszUserName)
    {
        (*env)->ReleaseStringChars(env, jUserName, pwszUserName);
    }
    return dwError;

error:
    goto cleanup;

}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsGetPermissionsW(
        JNIEnv *env,
        jclass clazz,
        jobject jpStore,
        jobject jOwner,
        jobject jpStorePermissions)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PVECS_STORE_PERMISSION_W pStorePermissions = NULL;
    PWSTR pwszOwner = NULL;
    DWORD dwUserCount = 0;

    if (jpStore == NULL || jOwner == NULL || jpStorePermissions == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    dwError = VecsGetPermissionsW(
            pStore,
            &pwszOwner,
            &dwUserCount,
            &pStorePermissions
            );

    dwError = JniSetString(env, jOwner, pwszOwner, "str");
    BAIL_ON_ERROR(dwError);

    dwError = JniAddPermissionsToList(env, jpStorePermissions, pStorePermissions, dwUserCount);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pStorePermissions)
    {
        VecsFreeStorePermissionsArrayW(pStorePermissions, dwUserCount);
    }
    if (pwszOwner)
    {
        VMAFD_SAFE_FREE_MEMORY(pwszOwner);
    }
    return dwError;

error:
    goto cleanup;

}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsCloseCertStore(
        JNIEnv  *env,
        jclass  clazz,
        jobject jpStore)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;

    if (jpStore == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = JniGetPointer(env, jpStore, (PVOID*)&pStore);
    BAIL_ON_ERROR(dwError);

    dwError = VecsCloseCertStore(pStore);
    BAIL_ON_ERROR(dwError);

    pStore = NULL;
    dwError = JniSetPointer(env, jpStore, (PVOID)pStore);
    BAIL_ON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VecsDeleteCertStoreHW(
        JNIEnv  *env,
        jclass  clazz,
        jobject jpServer,
        jstring jStoreName
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PCWSTR pwszStoreName = NULL;

    if (jpServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = JniGetPointer(env, jpServer, (PVOID*)&pServer);
    BAIL_ON_ERROR(dwError);

    if (jStoreName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    } else
    {
        pwszStoreName = (*env)->GetStringChars(env, jStoreName, NULL);
    }

    dwError = VecsDeleteCertStoreHW(
            pServer,
            pwszStoreName
            );
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pwszStoreName)
    {
        (*env)->ReleaseStringChars(env, jStoreName, pwszStoreName);
    }

    return dwError;

error:
    goto cleanup;
}

JNIEXPORT jint JNICALL
Java_com_vmware_identity_vecs_VecsAdapter_VmAfdCloseServer(
        JNIEnv  *env,
        jclass  clazz,
        jobject jpServer
        )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;

    if (jpServer == NULL) {
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
    if (jPointerClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidPointer = (*env)->GetFieldID(env, jPointerClass, "pointer", "J");
    if (fidPointer == NULL) {
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

    if (jPointerClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }
    fidPointer = (*env)->GetFieldID(env, jPointerClass, "pointer", "J");
    if (fidPointer == NULL) {
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
JniSetString(
    JNIEnv  *env,
    jobject jObj,
    PCWSTR  pwszValue,
    PCSTR   pszPropName
    )
{
    DWORD dwError = 0;
    jfieldID fid = NULL;
    jstring jStr = NULL;
    size_t size = 0;

    jclass jObjClass = (*env)->GetObjectClass(env, jObj);

    if (jObjClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }
    fid = (*env)->GetFieldID(env, jObjClass, pszPropName, "Ljava/lang/String;");
    if (fid == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    if (pwszValue)
    {
        dwError = VmAfdGetStringLengthW(pwszValue, &size);
        BAIL_ON_ERROR(dwError);
        jStr = (*env)->NewString(env, pwszValue, size);
    }
    else
    {
        jStr = NULL;
    }
    (*env)->SetObjectField(env, jObj, fid, jStr);

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

    if (jListClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    midPointer = (*env)->GetMethodID(env, jListClass, "add", "(Ljava/lang/Object;)Z");
    if (midPointer == NULL) {
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
JniAddEntriesToList(
        JNIEnv *env,
        jobject jList,
        PVECS_CERT_ENTRY_W pwEntries,
        DWORD dwCount
        )
{
    DWORD dwError = 0;
    jmethodID mid = NULL;
    jfieldID fidEntryType = NULL;
    jfieldID fidDate = NULL;
    jfieldID fidAlias = NULL;
    jfieldID fidCertificate = NULL;
    jfieldID fidKey = NULL;
    jstring  jStr = NULL;
    jclass jEntryClass = NULL;
    jobject jEntry = NULL;
    jboolean jbSuccess = JNI_FALSE;
    DWORD dwIndex = 0;
    size_t size = 0;

    jclass jListClass = (*env)->GetObjectClass(env, jList);

    if (jListClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    mid = (*env)->GetMethodID(env, jListClass, "add", "(Ljava/lang/Object;)Z");
    if (mid == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    jEntryClass = (*env)->FindClass(env, "com/vmware/identity/vecs/VecsEntryNative");
    if (jEntryClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidEntryType = (*env)->GetFieldID(env, jEntryClass, "entryType", "I");
    if (fidEntryType == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidDate = (*env)->GetFieldID(env, jEntryClass, "date", "J");
    if (fidDate == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidAlias = (*env)->GetFieldID(env, jEntryClass, "alias", "Ljava/lang/String;");
    if (fidAlias == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidCertificate = (*env)->GetFieldID(env, jEntryClass, "certificate", "Ljava/lang/String;");
    if (fidCertificate == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidKey = (*env)->GetFieldID(env, jEntryClass, "key", "Ljava/lang/String;");
    if (fidKey == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }


    dwIndex = 0;
    for (; dwIndex < dwCount; dwIndex++)
    {
		PVECS_CERT_ENTRY_W pwEntry = &pwEntries[dwIndex];
        jEntry = (*env)->AllocObject(env, jEntryClass);
        if (jEntry == NULL)
        {
            dwError = ERROR_BAD_UNIT;
            BAIL_ON_ERROR(dwError);
        }

        (*env)->SetIntField(env, jEntry, fidEntryType, (jint)(pwEntry->entryType));

        (*env)->SetLongField(env, jEntry, fidDate, (jint)(pwEntry->dwDate));

        dwError = VmAfdGetStringLengthW(pwEntry->pwszAlias, &size);
        BAIL_ON_ERROR(dwError);
        jStr = (*env)->NewString(env, pwEntry->pwszAlias, size);
        (*env)->SetObjectField(env, jEntry, fidAlias, jStr);

        if (pwEntry->pwszCertificate)
        {
            dwError = VmAfdGetStringLengthW(pwEntry->pwszCertificate, &size);
            BAIL_ON_ERROR(dwError);
            jStr = (*env)->NewString(env, pwEntry->pwszCertificate, size);
        }
        else
        {
            jStr = NULL;
        }
        (*env)->SetObjectField(env, jEntry, fidCertificate, jStr);

        if (pwEntry->pwszKey)
        {
            dwError = VmAfdGetStringLengthW(pwEntry->pwszKey, &size);
            BAIL_ON_ERROR(dwError);
            jStr = (*env)->NewString(env, pwEntry->pwszKey, size);
        }
        else
        {
            jStr = NULL;
        }
        (*env)->SetObjectField(env, jEntry, fidKey, jStr);


        jbSuccess = (*env)->CallBooleanMethod(env, jList, mid, jEntry);
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
JniAddPermissionsToList(
        JNIEnv *env,
        jobject jList,
        PVECS_STORE_PERMISSION_W pPermissions,
        DWORD dwCount
        )
{
    DWORD dwError = 0;
    jmethodID mid = NULL;
    jfieldID fidUserName = NULL;
    jfieldID fidAccessMask = NULL;
    jstring  jStr = NULL;
    jclass jPermissionClass = NULL;
    jobject jPermission = NULL;
    jboolean jbSuccess = JNI_FALSE;
    DWORD dwIndex = 0;
    size_t size = 0;

    jclass jListClass = (*env)->GetObjectClass(env, jList);

    if (jListClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    mid = (*env)->GetMethodID(env, jListClass, "add", "(Ljava/lang/Object;)Z");
    if (mid == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    jPermissionClass = (*env)->FindClass(env, "com/vmware/identity/vecs/VecsPermissionNative");
    if (jPermissionClass == NULL)
    {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidUserName = (*env)->GetFieldID(env, jPermissionClass, "userName", "Ljava/lang/String;");
    if (fidUserName == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidAccessMask = (*env)->GetFieldID(env, jPermissionClass, "accessMask", "I");
    if (fidAccessMask == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    dwIndex = 0;
    for (; dwIndex < dwCount; dwIndex++)
    {
        PVECS_STORE_PERMISSION_W pStorePermission = &pPermissions[dwIndex];
        jPermission = (*env)->AllocObject(env, jPermissionClass);
        if (jPermission == NULL)
        {
            dwError = ERROR_BAD_UNIT;
            BAIL_ON_ERROR(dwError);
        }

        dwError = VmAfdGetStringLengthW(pStorePermission->pszUserName, &size);
        BAIL_ON_ERROR(dwError);
        jStr = (*env)->NewString(env, pStorePermission->pszUserName, size);
        (*env)->SetObjectField(env, jPermission, fidUserName, jStr);

        (*env)->SetIntField(env, jPermission, fidAccessMask, (jint)(pStorePermission->dwAccessMask));

        jbSuccess = (*env)->CallBooleanMethod(env, jList, mid, jPermission);
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
JniPopulateJavaVecsEntryNative(
        JNIEnv *env,
        jobject jEntry,
        PVECS_CERT_ENTRY_W pwEntry
        )
{
    DWORD dwError = 0;
    jfieldID fidEntryType = NULL;
    jfieldID fidDate = NULL;
    jfieldID fidAlias = NULL;
    jfieldID fidCertificate = NULL;
    jfieldID fidKey = NULL;
    jstring jStr = NULL;
    size_t size = 0;

    jclass jEntryClass = (*env)->GetObjectClass(env, jEntry);
    if (jEntryClass == NULL) {
        dwError = ERROR_INVALID_DATATYPE;
        BAIL_ON_ERROR(dwError);
    }

    fidEntryType = (*env)->GetFieldID(env, jEntryClass, "entryType", "I");
    if (fidEntryType == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidDate = (*env)->GetFieldID(env, jEntryClass, "date", "J");
    if (fidDate == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidAlias = (*env)->GetFieldID(env, jEntryClass, "alias", "Ljava/lang/String;");
    if (fidAlias == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidCertificate = (*env)->GetFieldID(env, jEntryClass, "certificate", "Ljava/lang/String;");
    if (fidCertificate == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }

    fidKey = (*env)->GetFieldID(env, jEntryClass, "key", "Ljava/lang/String;");
    if (fidKey == NULL) {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_ERROR(dwError);
    }


    (*env)->SetIntField(env, jEntry, fidEntryType, (jint)(pwEntry->entryType));

    (*env)->SetLongField(env, jEntry, fidDate, (jint)(pwEntry->dwDate));

    dwError = VmAfdGetStringLengthW(pwEntry->pwszAlias, &size);
    BAIL_ON_ERROR(dwError);
    jStr = (*env)->NewString(env, pwEntry->pwszAlias, size);
    (*env)->SetObjectField(env, jEntry, fidAlias, jStr);

    if (pwEntry->pwszCertificate)
    {
        dwError = VmAfdGetStringLengthW(pwEntry->pwszCertificate, &size);
        BAIL_ON_ERROR(dwError);
        jStr = (*env)->NewString(env, pwEntry->pwszCertificate, size);
    }
    else
    {
        jStr = NULL;
    }
    (*env)->SetObjectField(env, jEntry, fidCertificate, jStr);

    if (pwEntry->pwszKey)
    {
        dwError = VmAfdGetStringLengthW(pwEntry->pwszKey, &size);
        BAIL_ON_ERROR(dwError);
        jStr = (*env)->NewString(env, pwEntry->pwszKey, size);
    }
    else
    {
        jStr = NULL;
    }
    (*env)->SetObjectField(env, jEntry, fidKey, jStr);

cleanup:
    return dwError;

error:
    goto cleanup;
}
