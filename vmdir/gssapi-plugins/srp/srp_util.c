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



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "gssapiP_srp.h"
#include "gssapi_srp.h"
#include "gssapi_alloc.h"

static char *g_debug_printf;

#if 1 /* Debug logging */

#ifndef VMDIR_LOG_MASK_ALL
#define VMDIR_LOG_MASK_ALL (-1)
#endif

unsigned long
VmDirLogInitialize(
   const char *pszLogFileName,
   int          bUseSysLog,
   const char *pszSyslogName,
   unsigned int  iLogLevel,
   unsigned long iInitLogMask
   );


void
VmDirLog(
   unsigned long level,
   const char*  fmt,
   ...);
#endif

#ifdef _WIN32
#if 1 /* debugging SRP logging */
#define OUTPUT_DEBUG_LOG(str) VmDirLog(VMDIR_LOG_MASK_ALL, "%s", (str))
#else
#define OUTPUT_DEBUG_LOG(str) OutputDebugStringA((char *) str)
#endif

#else
#if 1 /* debugging SRP logging */
#define OUTPUT_DEBUG_LOG(str) VmDirLog(VMDIR_LOG_MASK_ALL, "%s", (str))
#else
#include <syslog.h>
#define OUTPUT_DEBUG_LOG(str) syslog(LOG_DEBUG, "%s", str)
#endif
#endif


static char *srp_getenv_debug(void)
{
    if (!g_debug_printf)
    {
        g_debug_printf = getenv("GSSAPI_SRP_DEBUG");
        if (!g_debug_printf)
        {
            return NULL;
        }
#ifdef _WIN32
        VmDirLogInitialize(
           g_debug_printf,
           0,    // bUseSysLog
           NULL, // pszSyslogName
           0,
           VMDIR_LOG_MASK_ALL);
#else
        VmDirLogInitialize(
           NULL, // pszLogFileName
           1,    // bUseSysLog
           NULL, // pszSyslogName
           0,
           VMDIR_LOG_MASK_ALL);
#endif
    }
    return g_debug_printf;
}

int srp_debug_printf(char *fmt, ...)
{
    va_list print_args;
    int ret_len = 0;
    char *ptr = NULL;
    char debug_string[4*1024] = {0};

    if (!srp_getenv_debug())
    {
        return 0;
    }
    strcpy(debug_string, "  MMM ");
    va_start(print_args,fmt);
    ret_len = vsnprintf(debug_string+6,
                        sizeof(debug_string)-7, // 6 for MMM prefix, 1 for nul terminator
                        fmt,
                        print_args);

    ptr = strrchr(debug_string, '\n');
    if (ptr)
    {
        strcpy(ptr, " MMM\n");
    }
    else
    {
        ptr = debug_string + strlen(debug_string);
        strcpy(ptr, " MMM\n");
    }
    if (ret_len > 0)
    {
        OUTPUT_DEBUG_LOG(debug_string);
    }
    va_end(print_args);
    return ret_len;
}


char *srp_bin_to_hex_str(const unsigned char *buf, int buf_len)
{
    char *hexstr = NULL;
    unsigned int hex_hi = 0;
    unsigned int hex_lo = 0;
    static char hexchars[] = "0123456789abcdef";
    int i = 0;
    int j = 0;

    hexstr = calloc(buf_len*2+1, sizeof(char));
    if (buf)
    {
        for (i=0; i<buf_len; i++)
        {
            hex_hi = (0xf0 & buf[i]) >> 4;
            hex_lo = (0x0f & buf[i]);
            hexstr[j] = hexchars[hex_hi];
            hexstr[j+1] = hexchars[hex_lo];
            j += 2;
        }
        hexstr[j] = '\0';
    }
    return hexstr;
}

OM_uint32
srp_gss_duplicate_oid(
     OM_uint32 *minor_status,
     gss_OID   input_oid,
     gss_OID   *output_oid)
{
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    gss_buffer_desc oid_str = {0};
    gss_OID ret_oid = NULL;

    maj = gss_oid_to_str(&min, input_oid, &oid_str);
    if (maj)
    {
        goto error;
    }

    maj = gss_str_to_oid(&min, &oid_str, &ret_oid);
    if (maj)
    {
        goto error;
    }

    *output_oid = ret_oid;
    ret_oid = NULL;

error:
    if (maj)
    {
        *minor_status = min;
    }

    if (oid_str.value)
    {
        gss_release_buffer(&min, &oid_str);
    }
    return maj;
}


void srp_print_hex(const unsigned char *buf, int buf_len, const char *msg)
{
    char *hexstr = NULL;

    if (!srp_getenv_debug())
    {
        return;
    }

    srp_debug_printf("len = %d %s ", buf_len, msg?msg:"");
    hexstr = srp_bin_to_hex_str(buf, buf_len);
    if (hexstr)
    {
        srp_debug_printf("hex = %s\n", hexstr);
        free(hexstr);
        OUTPUT_DEBUG_LOG("\n");
    }
}


/*
 * tag for APPLICATION 0, Sequence[constructed, definite length]
 * length of remainder of token
 * tag of OBJECT IDENTIFIER
 * length of mechanism OID
 * encoding of mechanism OID
 * <the rest of the token>
 *
 * Numerically, this looks like :
 *
 * 0x60
 * <length> - could be multiple bytes
 * 0x06
 * <length> - assume only one byte, hence OID length < 127
 * <mech OID bytes>
 *
 */
OM_uint32
srp_asn1_encode_mech_oid_token(
    OM_uint32 *ret_minor,
    gss_OID mech_oid,
    gss_buffer_t output_token)
{
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    OM_uint32 asn1_mech_prefix_len = 4;
    gss_buffer_desc asn1_oid = {0};
    unsigned char *ptr = NULL;
    int i = 0;

    /* ASN.1 encoded SRP OID value */
    asn1_oid.length = mech_oid->length + asn1_mech_prefix_len;
    asn1_oid.value = gssalloc_malloc(asn1_oid.length);
    if (!asn1_oid.value)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }

    /* ASN.1 encode OID, State and length delimited display name string */
    memset(asn1_oid.value, 0, sizeof(asn1_oid.length));

    ptr = (unsigned char *) asn1_oid.value;
    i = 0;

    /* tag for APPLICATION 0, Sequence[constructed, definite length] */
    ptr[i++] = 0x60;

    /* length of remainder of token: OID tag(1) + OID len(1) */
    ptr[i++] = mech_oid->length + 2;

    /* ASN.1 Object Identifier tag */
    ptr[i++] = 0x06;

    /* Only works if value is < 127 bytes; GSS-SRP mech oid is much <127 */
    ptr[i++] = mech_oid->length;

    /* Copy the actual pre-encoded ASN.1 GSS-OID into the asn1_oid buffer */
    memcpy(&ptr[i], mech_oid->elements, mech_oid->length);

    *output_token = asn1_oid;
error:
    if (major)
    {
        *ret_minor = minor;
    }

    return major;
}
