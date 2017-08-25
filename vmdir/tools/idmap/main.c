/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

static
uint64_t dbmaxmapsize()
{
    DWORD   dwError = 0;
    HANDLE  hConn = NULL;
    HKEY    hKey = NULL;

    DWORD   val = 0;
    DWORD   valsize = sizeof(val);

    uint64_t    maxmapsize = 21474836480;

    dwError = RegOpenServer(&hConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegOpenKeyExA(hConn, NULL, HKEY_THIS_MACHINE, 0, KEY_READ, &hKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegGetValueA(
            hConn,
            hKey,
            "Services\\vmdir\\Parameters",
            "MaximumDbSizeMb",
            RRF_RT_REG_DWORD,
            NULL,
            (PVOID)&val,
            &valsize);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (maxmapsize < (uint64_t)(val)*1024*1024)
    {
        maxmapsize = (uint64_t)(val)*1024*1024;
    }

error:
    if (hConn)
    {
        if (hKey)
        {
            (VOID)RegCloseKey(hConn, hKey);
        }
        RegCloseServer(hConn);
    }
    return maxmapsize;
}

int dump()
{
    DWORD   dwError = 0;
    PSTR    pszErrMsg = NULL;

    MDB_env*        mdbEnv = NULL;
    uint64_t        maxmapsize = 0;
    unsigned int    envFlags = 0;
    mdb_mode_t      oflags = 0;

    MDB_dbi     mdbDBi = 0;
    MDB_txn*    pTxn = NULL;
    MDB_cursor* pCursor = NULL;
    MDB_val     key = {0};
    MDB_val     val = {0};

    unsigned int    cursorFlags = 0;
    PCSTR           pszKey = "1VmdirAttrIDToNameTb";
    PSTR            pszVal = NULL;

    PSTR*   ppszArr = NULL;
    DWORD   dwSize = 2048;
    DWORD   dwCnt = 0;
    DWORD   i = 0;

    SHA_CTX shaCtx = {0};
    char    sha1Digest[SHA_DIGEST_LENGTH] = {0};
    char    mdString[SHA_DIGEST_LENGTH*2+1] = {0};

#ifdef MDB_NOTLS
    envFlags = MDB_NOTLS; // Required for versions of mdb which have this flag
#endif

#ifndef _WIN32
    oflags = O_RDWR;
#else
    oflags = GENERIC_READ|GENERIC_WRITE;
#endif

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s starting", __FUNCTION__);

    // open db env
    dwError = mdb_env_create (&mdbEnv);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_env_set_maxreaders(mdbEnv, 1000);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_set_maxreaders failed (1000)");

    maxmapsize = dbmaxmapsize();
    dwError = mdb_env_set_mapsize(mdbEnv, maxmapsize);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_set_mapsize failed (%"PRIu64")", maxmapsize);

    dwError = mdb_env_set_maxdbs(mdbEnv, 100);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_set_maxdbs failed (100)");

    dwError = mdb_env_open(mdbEnv, VMDIR_DB_DIR, envFlags, oflags);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_open failed (%s)", VMDIR_DB_DIR);

    // open db
    dwError = mdb_txn_begin(mdbEnv, NULL, 0, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_open(pTxn, "MDB_GNERIC_DUPKEY_DB", MDB_DUPSORT, &mdbDBi);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_open failed");

    dwError = mdb_txn_commit(pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);
    pTxn = NULL;

    // iterate ID map
    dwError = mdb_txn_begin(mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_cursor_open(pTxn, mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_cursor_open failed");

    memset(&key, 0, sizeof(key));
    key.mv_size = VmDirStringLenA(pszKey);
    key.mv_data = (PVOID)pszKey;
    cursorFlags = MDB_SET;

    dwError = VmDirAllocateMemory(sizeof(PSTR)*dwSize, (PVOID)&ppszArr);
    BAIL_ON_VMDIR_ERROR(dwError);

    while ((dwError = mdb_cursor_get(pCursor, &key, &val, cursorFlags)) == 0)
    {
        dwError = VmDirAllocateAndCopyMemory(val.mv_data, val.mv_size, (PVOID*)&pszVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (dwCnt + 1 == dwSize)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    ppszArr,
                    (PVOID*)&ppszArr,
                    dwSize * sizeof(PSTR),
                    dwSize * sizeof(PSTR) * 2);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "VmDirReallocateMemoryWithInit failed (%d)", dwSize * 2);
            dwSize *= 2;
        }
        ppszArr[dwCnt++] = pszVal;

        printf("%s\n", pszVal);
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s", pszVal);
        pszVal = NULL;

        cursorFlags = MDB_NEXT;
    }

    // sort and compute chksum
    qsort(ppszArr, dwCnt, sizeof(PSTR), VmDirQsortCaseExactCompareString);

    SHA1_Init(&shaCtx);
    for (i = 0; i < dwCnt; i++)
    {
        SHA1_Update(&shaCtx, ppszArr[i], VmDirStringLenA(ppszArr[i]));
    }
    SHA1_Final(sha1Digest, &shaCtx);

    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(&mdString[i*2], "%02x", (unsigned int)sha1Digest[i]);
    }

    mdb_cursor_close(pCursor);
    pCursor = NULL;

    dwError = mdb_txn_commit(pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);
    pTxn = NULL;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s succeeded (SHA1 digest = %020s)", __FUNCTION__, mdString);

cleanup:
    mdb_close(mdbEnv, mdbDBi);
    mdb_env_close(mdbEnv);
    VmDirFreeStrArray(ppszArr);
    VMDIR_SAFE_FREE_MEMORY(pszVal);
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed [%d] %s\n", __FUNCTION__, dwError, VDIR_SAFE_STRING(pszErrMsg));
    mdb_cursor_close(pCursor);
    mdb_txn_abort(pTxn);
    goto cleanup;
}

int analyze(char* filename)
{
    DWORD   dwError = 0;
    PSTR    pszErrMsg = NULL;

    FILE*   fp = NULL;
    ssize_t read = 0;
    PSTR    line = NULL;
    size_t  len = 0;

    PSTR        pszId = NULL;
    PSTR        pszCur = NULL;
    PSTR        pszNew = NULL;
    PSTR        pszTok = NULL;
    PSTR        pszSav = NULL;
    PLW_HASHMAP     pMap = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    BOOLEAN     bFirst = TRUE;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s starting (%s)", __FUNCTION__, filename);

    // open input file
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        dwError = errno;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "fopen failed");
    }

    dwError = LwRtlCreateHashMap(
            &pMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    // build in-memory id map from file
    while ((read = getline(&line, &len, fp)) != -1)
    {
        line[read-1] = '\0';
        pszId = VmDirStringTokA(line, "=", &pszNew);

        dwError = VmDirAllocateStringA(pszId, &pszId);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (LwRtlHashMapFindKey(pMap, (PVOID*)&pszCur, pszId) == 0)
        {
            dwError = VmDirAllocateStringPrintf(&pszNew, "%s|%s", pszCur, pszNew);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirAllocateStringA(pszNew, &pszNew);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = LwRtlHashMapInsert(pMap, pszId, pszNew, &pair);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pair.pKey);
        VMDIR_SAFE_FREE_MEMORY(pair.pValue);
        VMDIR_SAFE_FREE_MEMORY(line);
    }

    // print all collisions
    while (LwRtlHashMapIterate(pMap, &iter, &pair))
    {
        pszId = (PSTR)pair.pKey;
        pszCur = (PSTR)pair.pValue;

        if (VmDirStringStrA(pszCur, "|"))
        {
            if (bFirst)
            {
                printf("Collisions\n");
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Collisions");
                bFirst = FALSE;
            }

            printf("----------\n");
            printf("%s:\n", pszId);
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "----------");
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s:", pszId);
            for (pszTok = VmDirStringTokA(pszCur, "|", &pszSav); pszTok; pszTok = VmDirStringTokA(NULL, "|", &pszSav))
            {
                printf("  %s\n", pszTok);
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "  %s", pszTok);
            }
        }
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s succeeded", __FUNCTION__);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);
    if (fp)
    {
        fclose(fp);
    }
    if (pMap)
    {
        LwRtlHashMapClear(pMap, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pMap);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed [%d] %s\n", __FUNCTION__, dwError, VDIR_SAFE_STRING(pszErrMsg));
    goto cleanup;
}

int repair(char* filename)
{
    DWORD   dwError = 0;
    PSTR    pszErrMsg = NULL;

    FILE*   fp = NULL;
    ssize_t read = 0;
    PSTR    line = NULL;
    size_t  len = 0;

    MDB_env*        mdbEnv = NULL;
    uint64_t        maxmapsize = 0;
    unsigned int    envFlags = 0;
    mdb_mode_t      oflags = 0;

    MDB_dbi     mdbDBi = 0;
    MDB_txn*    pTxn = NULL;
    MDB_val     key = {0};
    MDB_val     val = {0};

    PSTR*   ppszArr = NULL;
    DWORD   dwSize = 2048;
    DWORD   dwCnt = 0;
    DWORD   i = 0;

    SHA_CTX shaCtx = {0};
    char    sha1Digest[SHA_DIGEST_LENGTH] = {0};
    char    mdString[SHA_DIGEST_LENGTH*2+1] = {0};

    PCSTR           pszKey = "1VmdirAttrIDToNameTb";

#ifdef MDB_NOTLS
    envFlags = MDB_NOTLS; // Required for versions of mdb which have this flag
#endif

#ifndef _WIN32
    oflags = O_RDWR;
#else
    oflags = GENERIC_READ|GENERIC_WRITE;
#endif

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s starting (%s)", __FUNCTION__, filename);

    // open input file - sort and compute chksum
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        dwError = errno;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "fopen failed");
    }

    dwError = VmDirAllocateMemory(sizeof(PSTR)*dwSize, (PVOID)&ppszArr);
    BAIL_ON_VMDIR_ERROR(dwError);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (dwCnt + 1 == dwSize)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    ppszArr,
                    (PVOID*)&ppszArr,
                    dwSize * sizeof(PSTR),
                    dwSize * sizeof(PSTR) * 2);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "VmDirReallocateMemoryWithInit failed (%d)", dwSize * 2);
            dwSize *= 2;
        }
        line[read-1] = '\0';
        ppszArr[dwCnt++] = line;
        line = NULL;
    }
    qsort(ppszArr, dwCnt, sizeof(PSTR), VmDirQsortCaseExactCompareString);

    SHA1_Init(&shaCtx);
    for (i = 0; i < dwCnt; i++)
    {
        SHA1_Update(&shaCtx, ppszArr[i], VmDirStringLenA(ppszArr[i]));
    }
    SHA1_Final(sha1Digest, &shaCtx);

    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(&mdString[i*2], "%02x", (unsigned int)sha1Digest[i]);
    }

    // open db env
    dwError = mdb_env_create (&mdbEnv);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_env_set_maxreaders(mdbEnv, 1000);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_set_maxreaders failed (1000)");

    maxmapsize = dbmaxmapsize();
    dwError = mdb_env_set_mapsize(mdbEnv, maxmapsize);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_set_mapsize failed (%"PRIu64")", maxmapsize);

    dwError = mdb_env_set_maxdbs(mdbEnv, 100);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_set_maxdbs failed (100)");

    dwError = mdb_env_open(mdbEnv, VMDIR_DB_DIR, envFlags, oflags);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_env_open failed (%s)", VMDIR_DB_DIR);

    // open db
    dwError = mdb_txn_begin(mdbEnv, NULL, 0, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_open(pTxn, "MDB_GNERIC_DUPKEY_DB", MDB_DUPSORT, &mdbDBi);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_open failed");

    dwError = mdb_txn_commit(pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);
    pTxn = NULL;

    memset(&key, 0, sizeof(key));
    key.mv_size = VmDirStringLenA(pszKey);
    key.mv_data = (PVOID)pszKey;

    // delete ID map
    dwError = mdb_txn_begin(mdbEnv, NULL, 0, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_del(pTxn, mdbDBi, &key, NULL);
    dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_del failed");

    dwError = mdb_txn_commit(pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);
    pTxn = NULL;

    // write ID map from file
    dwError = mdb_txn_begin(mdbEnv, NULL, 0, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < dwCnt; i++)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s", ppszArr[i]);

        val.mv_data = (PVOID)ppszArr[i];
        val.mv_size = VmDirStringLenA(ppszArr[i]);

        dwError = mdb_put(pTxn, mdbDBi, &key, &val, 0);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "mdb_put failed");
    }

    dwError = mdb_txn_commit(pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);
    pTxn = NULL;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s succeeded (SHA1 digest = %020s)", __FUNCTION__, mdString);

cleanup:
    mdb_close(mdbEnv, mdbDBi);
    mdb_env_close(mdbEnv);
    VmDirFreeStrArray(ppszArr);
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);
    if (fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed [%d] %s\n", __FUNCTION__, dwError, VDIR_SAFE_STRING(pszErrMsg));
    mdb_txn_abort(pTxn);
    goto cleanup;
}

int main(int argc, char* argv[])
{
    DWORD   dwError = 0;
    PSTR    pszErrMsg = NULL;
    CHAR    pszPath[MAX_PATH];

    dwError = VmDirGetVmDirLogPath(pszPath, "idmap.log");
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "VmDirGetVmDirLogPath failed");

    dwError = VmDirLogInitialize(pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "VmDirLogInitialize failed");

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s starting", __FUNCTION__);

    if (argc == 2 && VmDirStringCompareA("dump", argv[1], TRUE) == 0)
    {
        dwError = dump();
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (argc == 3 && VmDirStringCompareA("analyze", argv[1], TRUE) == 0)
    {
        dwError = analyze(argv[2]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (argc == 3 && VmDirStringCompareA("repair", argv[1], TRUE) == 0)
    {
        dwError = repair(argv[2]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        printf( "Usage:\n"
                "\t./idmap dump\n"
                "\t./idmap analyze <filename>\n"
                "\t./idmap repair <filename>\n");

        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszErrMsg, "Invalid argument");
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s succeeded", __FUNCTION__);

cleanup:
    VmDirLogTerminate();
    return dwError;

error:
    printf("[%d] %s\n", dwError, VDIR_SAFE_STRING(pszErrMsg));
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed [%d] %s", __FUNCTION__, dwError, VDIR_SAFE_STRING(pszErrMsg));
    goto cleanup;
}
