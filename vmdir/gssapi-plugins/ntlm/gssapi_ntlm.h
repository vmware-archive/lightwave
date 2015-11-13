/* This is the gssapi_ntlm.h prologue. */

#include <stdint.h>
/* End of gssapi_krb5.h prologue. */
/* -*- mode: c; indent-tabs-mode: nil -*- */
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
 *
 * Module: gssapi_ntlm.h
 * Abstract:
 *     VMware GSSAPI NTLM Authentication Plugin
 *     GSSAPI NTLM public header file
 *
 * Author: Jonathan Brown (brownj@vmware.com)
 */


#ifndef _GSSAPI_NTLM_H_
#define _GSSAPI_NTLM_H_

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

/* NTLM Mechs */
extern const gss_OID_desc * const gss_mech_ntlm_oid;
extern const gss_OID_desc * const gss_nt_ntlm_name_oid;
extern const gss_OID_desc * const gss_ntlm_password_oid;

/* NTLM Mech sets */
extern const gss_OID_set_desc * const gss_mech_set_ntlm;


#if 1 /* debug; remove me */
void ntlm_print_hex(const unsigned char *buf, int buf_len, const char *msg);
#endif

#define GSS_NTLM_MECH_OID               (gss_mech_ntlm_oid->elements)
#define GSS_NTLM_MECH_OID_LEN           (gss_mech_ntlm_oid->length)

#define GSS_NTLM_NT_GENERAL_NAME        gss_nt_ntlm_name_oid
#define GSS_NTLM_NT_GENERAL_NAME_LEN    10

#define GSS_NTLM_PASSWORD_OID           (gss_ntlm_password_oid->elements)
#define GSS_NTLM_PASSWORD_LEN           (gss_ntlm_password_oid->length)

#if 0
#define gss_ntlm_nt_principal           gss_nt_ntlm_principal
#define gss_ntlm_nt_service_name        gss_nt_service_name
#define gss_ntlm_nt_user_name           gss_nt_user_name
#define gss_ntlm_nt_machine_uid_name    gss_nt_machine_uid_name
#define gss_ntlm_nt_string_uid_name     gss_nt_string_uid_name
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GSSAPI_NTLM_H_ */
