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



/*
 * Module Name: Directory Schema
 *
 * Filename: syntax.c
 *
 * Abstract:
 *
 * Schema syntax support
 *
 */

#include "includes.h"

static
int
syntaxPOIDCmp(
    const void *p1,
    const void *p2
    );

static
BOOLEAN
syntaxGetNextInteger(
    int         iStart,
    int         iLen,
    PVDIR_BERVALUE     pBerv,
    int*        piResult
    );

static
BOOLEAN
syntaxBinary(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxBoolean(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxInteger(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxNumericString(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxIA5String(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxDirectoryString(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxGeneralizedTime(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxPrintableString(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxNotImplemented(
    PVDIR_BERVALUE pBerv
    );

static
BOOLEAN
syntaxUTF8(
    PSTR    pStr,
    size_t   dwLen
    );

/*
 * We only support static list of syntax definitions.
 */
DWORD
VdirSyntaxLoad(
    VOID
    )
{
    DWORD    dwError = 0;

    static VDIR_SYNTAX_DESC gSyntaxTbl[] = VDIR_SYNTAX_INIT_TABLE_INITIALIZER;

    if (gVdirSyntaxGlobals.pSyntax)
    {
        return 0;
    }

    gVdirSyntaxGlobals.pSyntax = &gSyntaxTbl[0];
    gVdirSyntaxGlobals.usSize = sizeof(gSyntaxTbl)/sizeof(gSyntaxTbl[0]);

    qsort(gVdirSyntaxGlobals.pSyntax,
            gVdirSyntaxGlobals.usSize,
            sizeof(VDIR_SYNTAX_DESC),
            syntaxPOIDCmp);

#ifdef LDAP_DEBUG
{
    DWORD dwCnt = 0;
    VmDirLog( LDAP_DEBUG_TRACE, "Supported ldapSyntaxes");
    for (dwCnt=0; dwCnt<gVdirSyntaxGlobals.usSize; dwCnt++)
    {
        VmDirLog( LDAP_DEBUG_TRACE, "[%3d][%28s](%s)\n", dwCnt,
                gVdirSyntaxGlobals.pSyntax[dwCnt].pszName,
                gVdirSyntaxGlobals.pSyntax[dwCnt].pszOid);
    }
}
#endif


    return dwError;
}

DWORD
VdirSyntaxLookupByOid(
    PCSTR               pszOid,
    PVDIR_SYNTAX_DESC*  ppSyntax
    )
{
    DWORD   dwError = 0;
    PVDIR_SYNTAX_DESC pSyntax = NULL;
    VDIR_SYNTAX_DESC  key = {0};

    if (IsNullOrEmptyString(pszOid) || !ppSyntax)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.pszOid = (PSTR)pszOid;

    pSyntax = (PVDIR_SYNTAX_DESC) bsearch(
            &key,
            gVdirSyntaxGlobals.pSyntax,
            gVdirSyntaxGlobals.usSize,
            sizeof(VDIR_SYNTAX_DESC),
            syntaxPOIDCmp);

    dwError = pSyntax ? 0 : VMDIR_ERROR_NO_SUCH_SYNTAX;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSyntax = pSyntax;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

/*
 * Get syntax definitions
 *
 * (*pppszOutStr) is an array of PSTR ends with a NULL
 */
DWORD
VdirSyntaxGetDefinition(
    PSTR**    pppszOutStr,
    USHORT*   pusSize
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;
    USHORT usSize = 0;
    PSTR*  ppszBuf = NULL;

    if (!pppszOutStr || !pusSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * (gVdirSyntaxGlobals.usSize + 1),
            (PVOID*)&ppszBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < gVdirSyntaxGlobals.usSize; dwCnt++)
    {
        if (gVdirSyntaxGlobals.pSyntax[dwCnt].bPublic)
        {
            char pszTmp[256] = {0};

            dwError = VmDirStringNPrintFA(
                    pszTmp, 256, 255, "( %s DESC '%s' )",
                    gVdirSyntaxGlobals.pSyntax[dwCnt].pszOid,
                    gVdirSyntaxGlobals.pSyntax[dwCnt].pszName
            );
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(
                    pszTmp,
                    &ppszBuf[usSize++]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *pppszOutStr = ppszBuf;
    *pusSize = usSize;

cleanup:
    return dwError;

error:
    VmDirFreeStrArray(ppszBuf);
    goto cleanup;
}

static
int
syntaxPOIDCmp(
    const void *p1,
    const void *p2
    )
{
    PVDIR_SYNTAX_DESC pSyntaxDesc1 = (PVDIR_SYNTAX_DESC) p1;
    PVDIR_SYNTAX_DESC pSyntaxDesc2 = (PVDIR_SYNTAX_DESC) p2;

    if ((pSyntaxDesc1 == NULL || pSyntaxDesc1->pszOid == NULL) &&
        (pSyntaxDesc2 == NULL || pSyntaxDesc2->pszOid == NULL))
    {
        return 0;
    }

    if (pSyntaxDesc1 == NULL || pSyntaxDesc1->pszOid == NULL)
    {
        return -1;
    }

    if (pSyntaxDesc2 == NULL || pSyntaxDesc2->pszOid == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pSyntaxDesc1->pszOid, pSyntaxDesc2->pszOid, TRUE);
}

static
BOOLEAN
syntaxGetNextInteger(
    int        iStart,
    int        iLen,
    PVDIR_BERVALUE    pBerv,
    int*       piResult
    )
{
#define BUFFER_SIZE 32
    char    pBuf[BUFFER_SIZE] = {0};
    char*    pEnd = NULL;
    int      iValue = 0;
    int64_t  i64Val = 0;

    if (iStart + iLen > pBerv->lberbv.bv_len)
    {
        return FALSE;
    }

    VmDirStringNCpyA(pBuf, BUFFER_SIZE, pBerv->lberbv.bv_val+iStart, iLen);

    i64Val = VmDirStringToLA(pBuf, &pEnd, 10);

    if ( ( i64Val > INT_MAX ) || (i64Val < INT_MIN) )
    {
        return FALSE;
    }

    iValue = (int)i64Val;
    if (*pEnd != '\0')
    {
        return FALSE;
    }

    *piResult = iValue;

    return TRUE;

#undef BUFFER_SIZE
}

static
BOOLEAN
syntaxBinary(
    PVDIR_BERVALUE pBerv
    )
{
    return TRUE;
}

static
BOOLEAN
syntaxBoolean(
    PVDIR_BERVALUE pBerv
    )
{
    static const char pszBoolTrue[] =  "TRUE";
    static const char pszBoolFalse[] = "FALSE";

    if (!pBerv || !pBerv->lberbv.bv_val)
    {
        return FALSE;
    }

    //TODO, make sure true/false is not allowed
    if (VmDirStringCompareA(pszBoolTrue,  pBerv->lberbv.bv_val, FALSE) == 0 ||
        VmDirStringCompareA(pszBoolFalse, pBerv->lberbv.bv_val, FALSE) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

static
BOOLEAN
syntaxInteger(
    PVDIR_BERVALUE pBerv
    )
{
    DWORD    dwCnt = 0;

    if( VDIR_BERBV_ISNULL(pBerv) || VDIR_BERBV_ISEMPTY( pBerv ) )
    {
        return FALSE;
    }

    if ( pBerv->lberbv.bv_val[0] == '-'|| pBerv->lberbv.bv_val[0] == '+')
    {
        if ((pBerv->lberbv.bv_len > 2  && pBerv->lberbv.bv_val[1] == '0')  ||
            (pBerv->lberbv.bv_len == 1))
        {
            return FALSE;
        }

    }

    for( dwCnt = 1; dwCnt < pBerv->lberbv.bv_len; dwCnt++)
    {
        if( !ASCII_DIGIT(pBerv->lberbv.bv_val[dwCnt]) )
        {
            return FALSE;
        }
    }

    return TRUE;
}

static
BOOLEAN
syntaxNumericString(
    PVDIR_BERVALUE pBerv
    )
{
    DWORD dwCnt = 0;

    if( VDIR_BERBV_ISNULL(pBerv) || VDIR_BERBV_ISEMPTY( pBerv ) )
    {
        return FALSE;
    }

    for (dwCnt = 0; dwCnt < pBerv->lberbv.bv_len; dwCnt++)
    {
        if (! VDIR_NUMERIC_CHAR(pBerv->lberbv.bv_val[dwCnt]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static
BOOLEAN
syntaxIA5String(
    PVDIR_BERVALUE pBerv
    )
{
    DWORD dwCnt = 0;

    // IA5 string can be empty
    for (dwCnt = 0; dwCnt < pBerv->lberbv.bv_len; dwCnt++)
    {
        if (! VDIR_IA5_CHAR(pBerv->lberbv.bv_val[dwCnt]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static
BOOLEAN
syntaxDirectoryString(
    PVDIR_BERVALUE pBerv
    )
{
    if( VDIR_BERBV_ISNULL(pBerv) || VDIR_BERBV_ISEMPTY( pBerv ) )
    {
        return FALSE;
    }

    return syntaxUTF8(pBerv->lberbv.bv_val, pBerv->lberbv.bv_len);
}

/*
 * GeneralizedTime YYYYmmddHH[MM[SS]][(./,)d...](Z|(+/-)HH[MM])
 * A year will be a leap year if it is divisible by 4 but not by 100.
 * If a year is divisible by 4 and by 100, it is not a leap year unless it is also divisible by 400.
 */
static
BOOLEAN
syntaxGeneralizedTime(
    PVDIR_BERVALUE pBerv
    )
{
    int iCnt = 0;
    int segments[9] ={0};
    int iZFlag = 0;
    int iZoneFlag = 0;
    int    iDotFlag = 0;

    if( VDIR_BERBV_ISNULL(pBerv) || VDIR_BERBV_ISEMPTY( pBerv ) || pBerv->lberbv.bv_len < 11)
    {
        return FALSE;
    }

    if (! syntaxGetNextInteger(0, 4, pBerv, &segments[0])  ||
        ! syntaxGetNextInteger(4, 2, pBerv, &segments[1])  ||
        ! syntaxGetNextInteger(6, 2, pBerv, &segments[2])  ||
        ! syntaxGetNextInteger(8, 2, pBerv, &segments[3]))
    {
        return FALSE;
    }

    for (iCnt = 10; iCnt < pBerv->lberbv.bv_len; iCnt++)
    {
        if (iDotFlag == 0 && iZFlag == 0 && iZoneFlag == 0)
        {
            if (iCnt == 12 &&
                !syntaxGetNextInteger(10, 2, pBerv, &segments[4]))
            {
                return FALSE;
            }
            if (iCnt == 14 &&
                !syntaxGetNextInteger(12, 2, pBerv, &segments[5]))
            {
                return FALSE;
            }
        }

        if (pBerv->lberbv.bv_val[iCnt] == ',' || pBerv->lberbv.bv_val[iCnt] == '.')
        {
            iDotFlag = iCnt;
        }
        if (pBerv->lberbv.bv_val[iCnt] == '+' || pBerv->lberbv.bv_val[iCnt] == '-')
        {
            iZoneFlag = iCnt;
        }
        if (pBerv->lberbv.bv_val[iCnt] == 'Z')
        {
            iZFlag = iCnt;
        }
    }

    if (iDotFlag > 0)
    {
        int iNext = iZFlag > iZoneFlag ? iZFlag : iZoneFlag;

        if (iNext <= iDotFlag)
        {
            return FALSE;
        }
        if (!syntaxGetNextInteger(iDotFlag+1, iNext-iDotFlag-1, pBerv, &segments[6]))
        {
            return FALSE;
        }
    }

    if (iZoneFlag)
    {
        if (!syntaxGetNextInteger(iZoneFlag+1, 2, pBerv, &segments[7]))
        {
            return FALSE;
        }

        if (pBerv->lberbv.bv_len >= iZoneFlag+5 &&
            !syntaxGetNextInteger(iZoneFlag+3, 2, pBerv, &segments[8]))
        {
            return FALSE;
        }
    }

    // length check
    if (iZFlag > 0 && iZFlag != pBerv->lberbv.bv_len -1)
    {
        return FALSE;
    }
    else if(iZoneFlag > 0 &&
            (pBerv->lberbv.bv_len != iZoneFlag+2 || pBerv->lberbv.bv_len != iZoneFlag+4))
    {
        return FALSE;
    }
    else

    //TODO,  more checking..... leap year allowed month/day...etc.
    // year check
    // month check
    // day check
    // hour check
    // min check

    return TRUE;
}

// NOT compliance....
BOOLEAN
syntaxOID(
    PVDIR_BERVALUE pBerv
    )
{
    DWORD dwCnt = 0;

    // disable till can handle alpha
    return TRUE;

    for (dwCnt=0; dwCnt < pBerv->lberbv.bv_len; dwCnt++)
    {
        if ((pBerv->lberbv.bv_val[dwCnt]>= '0' && pBerv->lberbv.bv_val[dwCnt]<='9') ||
            pBerv->lberbv.bv_val[dwCnt] == '.')
        {
            continue;
        }

        return FALSE;
    }

    return TRUE;
}

static
BOOLEAN
syntaxPrintableString(
    PVDIR_BERVALUE pBerv
    )
{
    DWORD dwCnt = 0;

    if( VDIR_BERBV_ISNULL(pBerv) || VDIR_BERBV_ISEMPTY( pBerv ) )
    {
        return FALSE;
    }

    for (dwCnt = 0; dwCnt < pBerv->lberbv.bv_len; dwCnt++)
    {
        if (! VDIR_PRINTABLE_CHAR(pBerv->lberbv.bv_val[dwCnt]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static
BOOLEAN
syntaxNotImplemented(
    PVDIR_BERVALUE pBerv
    )
{
    return TRUE;
}

/*
 * Simple byte header check for UTF8
 */
static
BOOLEAN
syntaxUTF8(
    PSTR    pStr,
    size_t  dwLen
    )
{
    DWORD    dwUCEncodeLen = 1;
    DWORD    dwThisLen = 0;
    const unsigned char* pNow = NULL;
    const unsigned char* pEnd = (unsigned char*)pStr + dwLen;

    for (pNow = (unsigned char*)pStr;
         pNow < pEnd;
         pNow = pNow + dwThisLen)
    {
        unsigned char uc = *pNow;

        if ((uc & 0x80) == 0)
        { // 0x0xxxxxxx for single byte
            dwThisLen = 1;
            continue;
        }

        if ((uc & 0xc0) != 0xc0)
        { // 0x11xxxxxx must be true for multi bytes
            return FALSE;
        }

        uc = uc << 1;
        while ((uc & 0x80) == 0x80)  // Max len is 6
        {
            dwUCEncodeLen++;
            uc = uc << 1;
        }

        if (dwUCEncodeLen > 6 || pEnd - pNow < dwUCEncodeLen)
        {
            return FALSE;
        }

        dwThisLen = dwUCEncodeLen;

        // Very basic check - all subsequent byte must start with 0x10xxxxxx
        while (dwUCEncodeLen > 1)
        {
            if (((pNow[dwUCEncodeLen-1]) & 0xc0) != 0x80)
            {
                return FALSE;
            }
            dwUCEncodeLen--;
        }

    }

    return TRUE;
}
