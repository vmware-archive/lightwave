/*
 * Copyright © 2014 VMware, Inc.  All Rights Reserved.
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

/* This is the gssapi_unix.h prologue. */

#include <stdint.h>
/* End of gssapi_krb5.h prologue. */
/* -*- mode: c; indent-tabs-mode: nil -*- */
/*
 * Copyright 1993 by OpenVision Technologies, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Module: gssapi_unix.h
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     GSSAPI SRP public header file
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */


#ifndef _GSSAPI_SRP_H_
#define _GSSAPI_SRP_H_

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
#include <krb5.h>

/* C++ friendlyness */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Reserved static storage for GSS_oids.  See rfc 1964 for more details. */

/* 2.1.1. Kerberos Principal Name Form: */
GSS_DLLIMP extern const gss_OID_desc * const GSS_KRB5_NT_PRINCIPAL_NAME;
/* This name form shall be represented by the Object Identifier {iso(1)
 * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
 * krb5(2) krb5_name(1)}.  The recommended symbolic name for this type
 * is "GSS_KRB5_NT_PRINCIPAL_NAME". */

/* 2.1.2. Host-Based Service Name Form */
#define GSS_KRB5_NT_HOSTBASED_SERVICE_NAME GSS_C_NT_HOSTBASED_SERVICE
/* This name form shall be represented by the Object Identifier {iso(1)
 * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
 * generic(1) service_name(4)}.  The previously recommended symbolic
 * name for this type is "GSS_KRB5_NT_HOSTBASED_SERVICE_NAME".  The
 * currently preferred symbolic name for this type is
 * "GSS_C_NT_HOSTBASED_SERVICE". */

/* 2.2.1. User Name Form */
#define GSS_KRB5_NT_USER_NAME GSS_C_NT_USER_NAME
/* This name form shall be represented by the Object Identifier {iso(1)
 * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
 * generic(1) user_name(1)}.  The recommended symbolic name for this
 * type is "GSS_KRB5_NT_USER_NAME". */

/* 2.2.2. Machine UID Form */
#define GSS_KRB5_NT_MACHINE_UID_NAME GSS_C_NT_MACHINE_UID_NAME
/* This name form shall be represented by the Object Identifier {iso(1)
 * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
 * generic(1) machine_uid_name(2)}.  The recommended symbolic name for
 * this type is "GSS_KRB5_NT_MACHINE_UID_NAME". */

/* 2.2.3. String UID Form */
#define GSS_KRB5_NT_STRING_UID_NAME GSS_C_NT_STRING_UID_NAME
/* This name form shall be represented by the Object Identifier {iso(1)
 * member-body(2) United States(840) mit(113554) infosys(1) gssapi(2)
 * generic(1) string_uid_name(3)}.  The recommended symbolic name for
 * this type is "GSS_KRB5_NT_STRING_UID_NAME". */


/* SRP Mechs */
extern const gss_OID_desc * const gss_mech_srp_oid;
extern const gss_OID_desc * const gss_mech_gssapi_srp_oid;
extern const gss_OID_desc * const gss_nt_srp_name_oid;
extern const gss_OID_desc * const gss_srp_password_oid;

/* SRP Mech sets */
extern const gss_OID_set_desc * const gss_mech_set_srp;

/* "Made up" SRP mech OID */
#define GSS_SRP_MECH_OID_ST               (gss_mech_srp_oid->elements)
#define GSS_SRP_MECH_OID_LEN_ST           (gss_mech_srp_oid->length)

/* Officially allocated GSSAPI_SRP mech OID */
#define GSSAPI_SRP_MECH_OID_ST            (gss_mech_gssapi_srp_oid->elements)
#define GSSAPI_SRP_MECH_OID_LEN_ST        (gss_mech_gssapi_srp_oid->length)

#define GSS_SRP_NT_GENERAL_NAME_ST        gss_nt_srp_name_oid
#define GSS_SRP_NT_GENERAL_NAME_LEN_ST    10

/* "Made up" password OID; stolen from Likewise NTLM */
#define GSS_CRED_OPT_PW_ST              (gss_srp_password_oid->elements)
#define GSS_CRED_OPT_PW_LEN_ST          (gss_srp_password_oid->length)

/* Officially allocated GSSAPI_SRP set cred option OID */
#define GSSAPI_SRP_CRED_OPT_PW_ST         (gss_srp_cred_opt_pw_oid->elements)
#define GSSAPI_SRP_CRED_OPT_PW_LEN_ST     (gss_srp_cred_opt_pw_oid->length)

/* UNIX Mechs */
extern const gss_OID_desc * const gss_mech_unix_oid;
extern const gss_OID_desc * const gss_mech_gssapi_unix_oid;
extern const gss_OID_desc * const gss_nt_unix_name_oid;

/* UNIX Mech sets */
extern const gss_OID_set_desc * const gss_mech_set_unix;

/* Officially allocated GSSAPI_UNIX mech OID */
#define GSSAPI_UNIX_MECH_OID_ST            (gss_mech_gssapi_unix_oid->elements)
#define GSSAPI_UNIX_MECH_OID_LEN_ST        (gss_mech_gssapi_unix_oid->length)

/* Officially allocated GSSAPI_UNIX set cred option OID */
#define GSSAPI_UNIX_CRED_OPT_PW_ST         (gss_unix_cred_opt_pw_oid->elements)
#define GSSAPI_UNIX_CRED_OPT_PW_LEN_ST     (gss_unix_cred_opt_pw_oid->length)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GSSAPI_SRP_H_ */
