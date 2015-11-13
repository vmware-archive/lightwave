/*
 * Copyright (C) 2014, VMware Inc. All rights reserved.
 *
 * Module: srp_util.h
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     Shared Utility Functions header file
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#ifndef _SRP_UTIL_H
#define _SRP_UTIL_H

#include "gssapiP_unix.h"
#include "gssapi_unix.h"


char *
srp_bin_to_hex_str(
    const unsigned char *buf,
    int buf_len);

OM_uint32
srp_gss_duplicate_oid(
     OM_uint32 *minor_status,
     gss_OID   input_oid,
     gss_OID   *output_oid);


void 
srp_print_hex(
    const unsigned char *buf,
    int buf_len,
    const char *msg);


OM_uint32
srp_asn1_encode_mech_oid_token(
    OM_uint32 *ret_minor,
    gss_OID mech_oid,
    gss_buffer_t output_token);

int srp_debug_printf(char *fmt, ...);

#endif
