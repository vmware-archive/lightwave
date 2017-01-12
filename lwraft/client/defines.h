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


#define VMDIR_MAX_STRING_LEN    256

#define GROUP_ADMINISTRATORS "cn=Administrators,cn=BuiltIn"
#define GROUP_USERS "cn=Users,cn=BuiltIn"
#define GROUP_ACTASUSERS "cn=ActAsUsers"
#define GROUP_CMADMINISTRATORS "cn=ComponentManager.Administrators"
#define GROUP_SOLUTIONUSERS "cn=SolutionUsers"

#define ATTR_VMW_MACHINE_GUID "vmwMachineGUID"

#define VMDIR_MAX_PASSWORD_RETRIES 128

#define VMDIR_MAX_CONFIG_VALUE_LENGTH 2048

#define VMDIR_MAX_SCHEMACHECK_ATTR_COUNT 4

#define SLEEP_INTERVAL_IN_SECS  10

#define MAX_REPL_STATE_USN_SEARCH   64

#define VMDIR_LOCALHOST         "localhost"

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

#define VMDIR_ALIGN_BYTES(marker) \
            ((marker) % sizeof(size_t) ? \
                    sizeof(size_t) - ((marker) % sizeof(size_t)) : 0)

#define VMDIR_RPC_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) \
    { \
        unsigned32 rpcStatus = RPC_S_OK; \
        rpc_sm_client_free(pMemory, &rpcStatus); \
        (pMemory) = NULL; \
    }

#define VMDIR_RPC_TRY DCETHREAD_TRY
#define VMDIR_RPC_CATCH DCETHREAD_CATCH_ALL(THIS_CATCH)
#define VMDIR_RPC_ENDTRY DCETHREAD_ENDTRY
#define VMDIR_RPC_GETERROR_CODE(dwError) \
    dwError = VmDirDCEGetErrorCode(THIS_CATCH);

#define VMDIR_RPC_PROTECT_LEVEL_NONE        rpc_c_protect_level_none
#define VMDIR_RPC_PROTECT_LEVEL_PKT_PRIVACY rpc_c_protect_level_pkt_privacy

#define VMDIR_RPC_AUTHN_NONE                rpc_c_authn_none
#define VMDIR_RPC_AUTHN_GSS_NEGOTIATE       rpc_c_authn_gss_negotiate

#define VMDIR_RPC_AUTHZN_NONE               rpc_c_authz_none
#define VMDIR_RPC_AUTHZN_NAME               rpc_c_authz_name

#else

#define VMDIR_RPC_TRY RpcTryExcept
#define VMDIR_RPC_CATCH RpcExcept(1)
#define VMDIR_RPC_ENDTRY RpcEndExcept
#define VMDIR_RPC_GETERROR_CODE(dwError) \
    dwError = RpcExceptionCode();

#define VMDIR_RPC_PROTECT_LEVEL_NONE        RPC_C_AUTHN_LEVEL_NONE
#define VMDIR_RPC_PROTECT_LEVEL_PKT_PRIVACY RPC_C_AUTHN_LEVEL_PKT_PRIVACY

#define VMDIR_RPC_AUTHN_NONE                RPC_C_AUTHN_NONE
#define VMDIR_RPC_AUTHN_GSS_NEGOTIATE       RPC_C_AUTHN_GSS_NEGOTIATE

#define VMDIR_RPC_AUTHZN_NONE               RPC_C_AUTHZ_NONE
#define VMDIR_RPC_AUTHZN_NAME               RPC_C_AUTHZ_NAME

/*
MSDN: gethostname
The maximum length, in bytes, of the string returned in the buffer
pointed to by the name parameter is dependent on the namespace provider,
but this string must be 256 bytes or less.
So if a buffer of 256 bytes is passed in the name parameter
and the namelen parameter is set to 256,
the buffer size will always be adequate.
*/
#define HOST_NAME_MAX 256

#endif

#define VMDIR_CLOSE_BINDING_HANDLE(handle) \
{ \
    if ((handle) != NULL) \
    { \
        VmDirFreeBindingHandle(handle); \
        (handle) = NULL; \
    } \
}

#define VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER { "ldap", "vmca", "host", "http" }
#define VMDIR_CLIENT_SERVICE_PRINCIPAL_INITIALIZER { "host", "http" }

// Error handling
// vmdir ldap error space using macros defined in ldap.h
#define IS_VMDIR_LDAP_ERROR_SPACE(n)    \
    ( (LDAP_ATTR_ERROR(n))           || \
      (LDAP_NAME_ERROR(n))           || \
      (LDAP_SECURITY_ERROR(n))       || \
      (LDAP_SERVICE_ERROR(n))        || \
      (LDAP_UPDATE_ERROR((n)))       || \
      (LDAP_RANGE((n),0x01,0x0e))    || \
      (LDAP_RANGE((n),0x71,0x7b))    || \
      (LDAP_E_ERROR((n)))            || \
      (LDAP_RANGE((n),0x4100,0x4121))|| \
      ((n) == LDAP_OTHER)               \
    )

// vmdir rpc error space rpc_s_mod(0x16c9a000)~rpc_s_fault_codeset_conv_error(0x16c9a16e)
#define IS_VMDIR_RPC_ERROR_SPACE(n) \
    (VMDIR_RANGE(n, rpc_s_mod, rpc_s_fault_codeset_conv_error))

// WARNING: vmdir errors MUST stay in sync with definitions in vmdirerros.h
#define VMDIR_ERROR_TABLE_INITIALIZER \
{ \
    {VMDIR_ERROR_NO_SUCH_DITSTRUCTURERULES, \
        "No such ditstructure rules"}, \
    {VMDIR_ERROR_PASSWORD_EXPIRED, \
        "Password expired"}, \
    {VMDIR_ERROR_KERBEROS_ERROR, \
        "Kerberos error"}, \
    {VMDIR_ERROR_INVALID_CONFIGURATION, \
        "Invalid configuration"}, \
    {VMDIR_ERROR_ACCESS_DENIED, \
        "Access denied"}, \
    {VMDIR_ERROR_OPERATION_NOT_PERMITTED, \
        "Operation not permitted"}, \
    {VMDIR_ERROR_INVALID_RESULT, \
        "Invalid result"}, \
    {VMDIR_ERROR_UNDEFINED_TYPE, \
        "Undefined type"}, \
    {VMDIR_ERROR_OBJECTCLASS_VIOLATION, \
        "Objectclass violation"}, \
    {VMDIR_ERROR_ENTRY_ALREADY_EXIST, \
        "Entry already exist"}, \
    {VMDIR_ERROR_UNAVAILABLE, \
        "Unavailable"}, \
    {VMDIR_ERROR_VDCREPADMIN_TOO_FEW_REPLICATION_PARTNERS, \
        "Vdcrepadmin too few replication partners"}, \
    {VMDIR_ERROR_USER_NO_CREDENTIAL, \
        "User no credential"}, \
    {VMDIR_ERROR_INVALID_REALM, \
        "Invalid realm"}, \
    {VMDIR_ERROR_VDCSPLIT, \
        "Vdcsplit"}, \
    {VMDIR_ERROR_NO_USN, \
        "No USN"}, \
    {VMDIR_ERROR_INVALID_DITSTRUCTURERULES, \
        "Invalid ditstructure rules"}, \
    {VMDIR_ERROR_GENERIC, \
        "Generic error"}, \
    {VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED, \
        "Auth method not supported"}, \
    {VMDIR_ERROR_VDCMERGE, \
        "Vdcmerge error"}, \
    {VMDIR_ERROR_INVALID_DN, \
        "Invalid DN"}, \
    {VMDIR_ERROR_INVALID_DITCONTENTRULES, \
        "Invalid dit content rules"}, \
    {VMDIR_ERROR_INVALID_ATTRIBUTETYPES, \
        "Invalid attribute types"}, \
    {VMDIR_ERROR_PASSWORD_HASH, \
        "Password hash"}, \
    {VMDIR_ERROR_RID_LIMIT_EXCEEDED, \
        "Rid limit exceeded"}, \
    {VMDIR_ERROR_NO_CRED_CACHE_FOUND, \
        "No cred cache found"}, \
    {VMDIR_ERROR_INVALID_SCHEMA, \
        "Invalid schema"}, \
    {VMDIR_ERROR_NO_SUCH_DITCONTENTRULES, \
        "No such dit content rules"}, \
    {VMDIR_ERROR_PASSWORD_POLICY_VIOLATION, \
        "Password policy violation"}, \
    {VMDIR_ERROR_NO_SUCH_NAMEFORMS, \
        "No such name forms"}, \
    {VMDIR_ERROR_ENTRY_NOT_FOUND, \
        "Entry not found"}, \
    {VMDIR_ERROR_INVALID_ENTRY, \
        "Invalid entry"}, \
    {VMDIR_ERROR_PARENT_NOT_FOUND, \
        "Parent not found"}, \
    {VMDIR_ERROR_ACCOUNT_DISABLED, \
        "Account disabled"}, \
    {VMDIR_ERROR_TOKEN_IN_USE, \
        "Token in use"}, \
    {VMDIR_ERROR_INVALID_REQUEST, \
        "Invalid request"}, \
    {VMDIR_ERROR_NO_SUCH_OBJECTCLASS, \
        "No such objectclass"}, \
    {VMDIR_ERROR_VDCREPADMIN_GENERAL, \
        "Vdcrepadmin general"}, \
    {VMDIR_ERROR_NO_SECURITY_DESCRIPTOR, \
        "No security descriptor"}, \
    {VMDIR_ERROR_PASSWORD_TOO_LONG, \
        "Password too long"}, \
    {VMDIR_ERROR_INVALID_SYNTAX, \
        "Invalid syntax"}, \
    {VMDIR_ERROR_UNWILLING_TO_PERFORM, \
        "Unwilling to perform"}, \
    {VMDIR_ERROR_SIZELIMIT_EXCEEDED, \
        "Sizelimit exceeded"}, \
    {VMDIR_ERROR_KERBEROS_REALM_OFFLINE, \
        "Kerberos realm offline"}, \
    {VMDIR_ERROR_SSL, \
        "SSL error"}, \
    {VMDIR_ERROR_NO_SUCH_SYNTAX, \
        "No such syntax"}, \
    {VMDIR_ERROR_DOMAIN_NOT_FOUND, \
        "Domain not found"}, \
    {VMDIR_ERROR_NAMING_VIOLATION, \
        "Naming violation"}, \
    {VMDIR_ERROR_NO_CRED_CACHE_NAME, \
        "No cred cache name"}, \
    {VMDIR_ERROR_NO_OBJECT_SID_GEN, \
        "No object sid gen"}, \
    {VMDIR_ERROR_NO_SSL_CTX, \
        "No SSL ctx"}, \
    {VMDIR_ERROR_TIMELIMIT_EXCEEDED, \
        "Timelimit exceeded"}, \
    {VMDIR_ERROR_NO_SUCH_ATTRIBUTE, \
        "No such attribute"}, \
    {VMDIR_ERROR_INVALID_OBJECTCLASSES, \
        "Invalid objectclasses"}, \
    {VMDIR_ERROR_SSL_CERT_FILE_NOT_FOUND, \
        "Ssl cert file not found"}, \
    {VMDIR_ERROR_INSUFFICIENT_ACCESS, \
        "Insufficient access"}, \
    {VMDIR_ERROR_NOT_FOUND, \
        "Not found"}, \
    {VMDIR_ERROR_CANNOT_LOAD_LIBRARY, \
        "Cannot load library"}, \
    {VMDIR_ERROR_STRUCTURE_VIOLATION, \
        "Structure violation"}, \
    {VMDIR_ERROR_BUSY, \
        "Vmdir busy"}, \
    {VMDIR_ERROR_BASE, \
        "Vmdir base"}, \
    {VMDIR_ERROR_NO_MYSELF, \
        "No myself"}, \
    {VMDIR_ERROR_BACKEND_ERROR, \
        "Backend error"}, \
    {VMDIR_ERROR_BACKEND_ENTRY_EXISTS, \
        "Backend entry exists"}, \
    {VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION, \
        "Data constraint violation"}, \
    {VMDIR_ERROR_BACKEND_CONSTRAINT, \
        "Backend constraint"}, \
    {VMDIR_ERROR_NO_OBJECTSID_ATTR, \
        "No object sid attr"}, \
    {VMDIR_ERROR_NO_MEMORY, \
        "No memory"}, \
    {VMDIR_ERROR_BACKEND_OPERATIONS, \
        "Backend operations"}, \
    {VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND, \
        "Backend entry notfound"}, \
    {VMDIR_ERROR_NO_SCHEMA, \
        "No schema"}, \
    {VMDIR_ERROR_IO, \
        "IO error"}, \
    {VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF, \
        "Not allowed on nonleaf"}, \
    {VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY, \
        "No such file or directory"}, \
    {VMDIR_ERROR_BACKEND_MAX_RETRY, \
        "Backend max retry"}, \
    {VMDIR_ERROR_LOCK_DEADLOCK, \
        "Lock deadlock"}, \
    {VMDIR_ERROR_USER_INVALID_CREDENTIAL, \
        "User invalid credential"}, \
    {VMDIR_ERROR_BAD_ATTRIBUTE_DATA, \
        "Bad attribute data"}, \
    {VMDIR_ERROR_INVALID_PARAMETER, \
        "Invalid parameter"}, \
    {VMDIR_ERROR_ACCOUNT_LOCKED, \
        "Account locked"}, \
    {VMDIR_ERROR_SASL_BIND_IN_PROGRESS, \
        "Sasl bind in progress"}, \
    {VMDIR_ERROR_SERVER_DOWN, \
        "Server down"}, \
    {VMDIR_ERROR_USER_LOCKOUT, \
        "User lockout"}, \
    {VMDIR_ERROR_INVALID_POLICY_DEFINITION, \
        "Invalid policy definition"}, \
    {VMDIR_ERROR_BACKEND_ATTR_META_DATA_NOTFOUND, \
        "Backend attr meta data notfound"}, \
    {VMDIR_ERROR_INVALID_NAMEFORMS, \
        "Invalid nameforms"}, \
    {VMDIR_ERROR_ORG_ID_GEN_FAILED, \
        "Org id gen failed"}, \
    {VMDIR_ERROR_CANNOT_CONNECT_VMDIR, \
        "Cannot connect vmdir"}, \
    {VMDIR_ERROR_SRP, \
        "SRP error"}, \
    {VMDIR_ERROR_BACKEND_DEADLOCK, \
        "Backend deadlock"}, \
    {VMDIR_ERROR_TYPE_OR_VALUE_EXISTS, \
        "Type or value exists"}, \
    {VMDIR_ERROR_VDCPROMO, \
        "Vdcpromo error"}, \
    {VMDIR_ERROR_BACKEND_PARENT_NOTFOUND, \
        "Backend parent notfound"}, \
};

#define VMDIR_RPC_ERROR_TABLE_INITIALIZER \
{\
{rpc_s_mod, \
    "(rpc_s_mod) "}, \
{rpc_s_op_rng_error, \
    "(rpc_s_op_rng_error) Operation out of range"}, \
{rpc_s_cant_create_socket, \
    "(rpc_s_cant_create_socket) Cannot create socket"}, \
{rpc_s_cant_bind_socket, \
    "(rpc_s_cant_bind_socket) Cannot bind socket"}, \
{rpc_s_not_in_call, \
    "(rpc_s_not_in_call) "}, \
{rpc_s_no_port, \
    "(rpc_s_no_port) "}, \
{rpc_s_wrong_boot_time, \
    "(rpc_s_wrong_boot_time) Wrong boot time"}, \
{rpc_s_too_many_sockets, \
    "(rpc_s_too_many_sockets) "}, \
{rpc_s_illegal_register, \
    "(rpc_s_illegal_register) "}, \
{rpc_s_cant_recv, \
    "(rpc_s_cant_recv) "}, \
{rpc_s_bad_pkt, \
    "(rpc_s_bad_pkt) "}, \
{rpc_s_unbound_handle, \
    "(rpc_s_unbound_handle) "}, \
{rpc_s_addr_in_use, \
    "(rpc_s_addr_in_use) "}, \
{rpc_s_in_args_too_big, \
    "(rpc_s_in_args_too_big) Input arguments too big"}, \
{rpc_s_string_too_long, \
    "(rpc_s_string_too_long) "}, \
{rpc_s_too_many_objects, \
    "(rpc_s_too_many_objects) "}, \
{rpc_s_binding_has_no_auth, \
    "(rpc_s_binding_has_no_auth) Not authenticated"}, \
{rpc_s_unknown_authn_service, \
    "(rpc_s_unknown_authn_service) Unknown authentication service"}, \
{rpc_s_no_memory, \
    "(rpc_s_no_memory) No memory for RPC runtime"}, \
{rpc_s_cant_nmalloc, \
    "(rpc_s_cant_nmalloc) "}, \
{rpc_s_call_faulted, \
    "(rpc_s_call_faulted) Call faulted"}, \
{rpc_s_call_failed, \
    "(rpc_s_call_failed) "}, \
{rpc_s_comm_failure, \
    "(rpc_s_comm_failure) Communications failure"}, \
{rpc_s_rpcd_comm_failure, \
    "(rpc_s_rpcd_comm_failure) RPC daemon communications failure"}, \
{rpc_s_illegal_family_rebind, \
    "(rpc_s_illegal_family_rebind) "}, \
{rpc_s_invalid_handle, \
    "(rpc_s_invalid_handle) "}, \
{rpc_s_coding_error, \
    "(rpc_s_coding_error) RPC coding error"}, \
{rpc_s_object_not_found, \
    "(rpc_s_object_not_found) Object not found"}, \
{rpc_s_cthread_not_found, \
    "(rpc_s_cthread_not_found) Call thread not found"}, \
{rpc_s_invalid_binding, \
    "(rpc_s_invalid_binding) Invalid binding"}, \
{rpc_s_already_registered, \
    "(rpc_s_already_registered) Already registered"}, \
{rpc_s_endpoint_not_found, \
    "(rpc_s_endpoint_not_found) Endpoint not found"}, \
{rpc_s_invalid_rpc_protseq, \
    "(rpc_s_invalid_rpc_protseq) Invalid RPC protocol sequence"}, \
{rpc_s_desc_not_registered, \
    "(rpc_s_desc_not_registered) Descriptor not registered"}, \
{rpc_s_already_listening, \
    "(rpc_s_already_listening) Already listening"}, \
{rpc_s_no_protseqs, \
    "(rpc_s_no_protseqs) No protocol sequences"}, \
{rpc_s_no_protseqs_registered, \
    "(rpc_s_no_protseqs_registered) No protocol sequences registered"}, \
{rpc_s_no_bindings, \
    "(rpc_s_no_bindings) No bindings"}, \
{rpc_s_max_descs_exceeded, \
    "(rpc_s_max_descs_exceeded) Max descriptors exceeded"}, \
{rpc_s_no_interfaces, \
    "(rpc_s_no_interfaces) No interfaces"}, \
{rpc_s_invalid_timeout, \
    "(rpc_s_invalid_timeout) Invalid timeout"}, \
{rpc_s_cant_inq_socket, \
    "(rpc_s_cant_inq_socket) Cannot inquire socket"}, \
{rpc_s_invalid_naf_id, \
    "(rpc_s_invalid_naf_id) Invalid network address family ID"}, \
{rpc_s_inval_net_addr, \
    "(rpc_s_inval_net_addr) Invalid network address"}, \
{rpc_s_unknown_if, \
    "(rpc_s_unknown_if) Unknown interface"}, \
{rpc_s_unsupported_type, \
    "(rpc_s_unsupported_type) Unsupported type"}, \
{rpc_s_invalid_call_opt, \
    "(rpc_s_invalid_call_opt) Invalid call option"}, \
{rpc_s_no_fault, \
    "(rpc_s_no_fault) No fault"}, \
{rpc_s_cancel_timeout, \
    "(rpc_s_cancel_timeout) Cancel timeout"}, \
{rpc_s_call_cancelled, \
    "(rpc_s_call_cancelled) Call canceled"}, \
{rpc_s_invalid_call_handle, \
    "(rpc_s_invalid_call_handle) "}, \
{rpc_s_cannot_alloc_assoc, \
    "(rpc_s_cannot_alloc_assoc) "}, \
{rpc_s_cannot_connect, \
    "(rpc_s_cannot_connect) Cannot connect"}, \
{rpc_s_connection_aborted, \
    "(rpc_s_connection_aborted) Connection aborted"}, \
{rpc_s_connection_closed, \
    "(rpc_s_connection_closed) Connection closed"}, \
{rpc_s_cannot_accept, \
    "(rpc_s_cannot_accept) Cannot accept"}, \
{rpc_s_assoc_grp_not_found, \
    "(rpc_s_assoc_grp_not_found) Association group not found"}, \
{rpc_s_stub_interface_error, \
    "(rpc_s_stub_interface_error) "}, \
{rpc_s_invalid_object, \
    "(rpc_s_invalid_object) Invalid object"}, \
{rpc_s_invalid_type, \
    "(rpc_s_invalid_type) "}, \
{rpc_s_invalid_if_opnum, \
    "(rpc_s_invalid_if_opnum) "}, \
{rpc_s_different_server_instance, \
    "(rpc_s_different_server_instance) "}, \
{rpc_s_protocol_error, \
    "(rpc_s_protocol_error) Protocol error"}, \
{rpc_s_cant_recvmsg, \
    "(rpc_s_cant_recvmsg) "}, \
{rpc_s_invalid_string_binding, \
    "(rpc_s_invalid_string_binding) Invalid string binding"}, \
{rpc_s_connect_timed_out, \
    "(rpc_s_connect_timed_out) Connection request timed out"}, \
{rpc_s_connect_rejected, \
    "(rpc_s_connect_rejected) Connection request rejected"}, \
{rpc_s_network_unreachable, \
    "(rpc_s_network_unreachable) Network is unreachable"}, \
{rpc_s_connect_no_resources, \
    "(rpc_s_connect_no_resources) Not enough resources for connection at local or remote host"}, \
{rpc_s_rem_network_shutdown, \
    "(rpc_s_rem_network_shutdown) Network is down"}, \
{rpc_s_too_many_rem_connects, \
    "(rpc_s_too_many_rem_connects) Too many connections at remote host"}, \
{rpc_s_no_rem_endpoint, \
    "(rpc_s_no_rem_endpoint) Endpoint does not exist at remote host"}, \
{rpc_s_rem_host_down, \
    "(rpc_s_rem_host_down) Remote host is down"}, \
{rpc_s_host_unreachable, \
    "(rpc_s_host_unreachable) Remote host is unreachable"}, \
{rpc_s_access_control_info_inv, \
    "(rpc_s_access_control_info_inv) Access control information invalid at remote host"}, \
{rpc_s_loc_connect_aborted, \
    "(rpc_s_loc_connect_aborted) Connection aborted by local host"}, \
{rpc_s_connect_closed_by_rem, \
    "(rpc_s_connect_closed_by_rem) Connection closed by remote host"}, \
{rpc_s_rem_host_crashed, \
    "(rpc_s_rem_host_crashed) Remote host crashed"}, \
{rpc_s_invalid_endpoint_format, \
    "(rpc_s_invalid_endpoint_format) Invalid endpoint format for remote host"}, \
{rpc_s_unknown_status_code, \
    "(rpc_s_unknown_status_code) "}, \
{rpc_s_unknown_mgr_type, \
    "(rpc_s_unknown_mgr_type) Unknown or unsupported manager type"}, \
{rpc_s_assoc_creation_failed, \
    "(rpc_s_assoc_creation_failed) "}, \
{rpc_s_assoc_grp_max_exceeded, \
    "(rpc_s_assoc_grp_max_exceeded) Association group maximum exceeded"}, \
{rpc_s_assoc_grp_alloc_failed, \
    "(rpc_s_assoc_grp_alloc_failed) "}, \
{rpc_s_sm_invalid_state, \
    "(rpc_s_sm_invalid_state) Invalid state machine state"}, \
{rpc_s_assoc_req_rejected, \
    "(rpc_s_assoc_req_rejected) "}, \
{rpc_s_assoc_shutdown, \
    "(rpc_s_assoc_shutdown) Association shutdown"}, \
{rpc_s_tsyntaxes_unsupported, \
    "(rpc_s_tsyntaxes_unsupported) Transfer syntaxes unsupported"}, \
{rpc_s_context_id_not_found, \
    "(rpc_s_context_id_not_found) Context id not found"}, \
{rpc_s_cant_listen_socket, \
    "(rpc_s_cant_listen_socket) "}, \
{rpc_s_no_addrs, \
    "(rpc_s_no_addrs) No addresses"}, \
{rpc_s_cant_getpeername, \
    "(rpc_s_cant_getpeername) Cannot get peer name"}, \
{rpc_s_cant_get_if_id, \
    "(rpc_s_cant_get_if_id) Cannot get interface ID"}, \
{rpc_s_protseq_not_supported, \
    "(rpc_s_protseq_not_supported) Protocol sequence not supported"}, \
{rpc_s_call_orphaned, \
    "(rpc_s_call_orphaned) Call orphaned"}, \
{rpc_s_who_are_you_failed, \
    "(rpc_s_who_are_you_failed) Who are you failed"}, \
{rpc_s_unknown_reject, \
    "(rpc_s_unknown_reject) Unknown reject"}, \
{rpc_s_type_already_registered, \
    "(rpc_s_type_already_registered) Type already registered"}, \
{rpc_s_stop_listening_disabled, \
    "(rpc_s_stop_listening_disabled) "}, \
{rpc_s_invalid_arg, \
    "(rpc_s_invalid_arg) Invalid argument"}, \
{rpc_s_not_supported, \
    "(rpc_s_not_supported) Not supported"}, \
{rpc_s_wrong_kind_of_binding, \
    "(rpc_s_wrong_kind_of_binding) Wrong kind of binding"}, \
{rpc_s_authn_authz_mismatch, \
    "(rpc_s_authn_authz_mismatch) Authentication authorization mismatch"}, \
{rpc_s_call_queued, \
    "(rpc_s_call_queued) Call queued"}, \
{rpc_s_cannot_set_nodelay, \
    "(rpc_s_cannot_set_nodelay) Cannot set nodelay"}, \
{rpc_s_not_rpc_tower, \
    "(rpc_s_not_rpc_tower) Not an RPC tower"}, \
{rpc_s_invalid_rpc_protid, \
    "(rpc_s_invalid_rpc_protid) "}, \
{rpc_s_invalid_rpc_floor, \
    "(rpc_s_invalid_rpc_floor) "}, \
{rpc_s_call_timeout, \
    "(rpc_s_call_timeout) Call timed out"}, \
{rpc_s_mgmt_op_disallowed, \
    "(rpc_s_mgmt_op_disallowed) Management operation disallowed"}, \
{rpc_s_manager_not_entered, \
    "(rpc_s_manager_not_entered) Manager not entered"}, \
{rpc_s_calls_too_large_for_wk_ep, \
    "(rpc_s_calls_too_large_for_wk_ep) Calls too large for well known endpoint"}, \
{rpc_s_server_too_busy, \
    "(rpc_s_server_too_busy) Server too busy"}, \
{rpc_s_prot_version_mismatch, \
    "(rpc_s_prot_version_mismatch) "}, \
{rpc_s_rpc_prot_version_mismatch, \
    "(rpc_s_rpc_prot_version_mismatch) RPC protocol version mismatch"}, \
{rpc_s_ss_no_import_cursor, \
    "(rpc_s_ss_no_import_cursor) No import cursor"}, \
{rpc_s_fault_addr_error, \
    "(rpc_s_fault_addr_error) Fault address error"}, \
{rpc_s_fault_context_mismatch, \
    "(rpc_s_fault_context_mismatch) Fault context mismatch"}, \
{rpc_s_fault_fp_div_by_zero, \
    "(rpc_s_fault_fp_div_by_zero) Fault floating point divide by zero"}, \
{rpc_s_fault_fp_error, \
    "(rpc_s_fault_fp_error) Fault floating point error"}, \
{rpc_s_fault_fp_overflow, \
    "(rpc_s_fault_fp_overflow) Fault floating point overflow"}, \
{rpc_s_fault_fp_underflow, \
    "(rpc_s_fault_fp_underflow) Fault floating point underflow"}, \
{rpc_s_fault_ill_inst, \
    "(rpc_s_fault_ill_inst) Fault illegal instruction"}, \
{rpc_s_fault_int_div_by_zero, \
    "(rpc_s_fault_int_div_by_zero) Fault integer divide by zero"}, \
{rpc_s_fault_int_overflow, \
    "(rpc_s_fault_int_overflow) Fault integer overflow"}, \
{rpc_s_fault_invalid_bound, \
    "(rpc_s_fault_invalid_bound) Fault invalid bound"}, \
{rpc_s_fault_invalid_tag, \
    "(rpc_s_fault_invalid_tag) Fault invalid tag"}, \
{rpc_s_fault_pipe_closed, \
    "(rpc_s_fault_pipe_closed) Fault pipe closed"}, \
{rpc_s_fault_pipe_comm_error, \
    "(rpc_s_fault_pipe_comm_error) Fault pipe communication error"}, \
{rpc_s_fault_pipe_discipline, \
    "(rpc_s_fault_pipe_discipline) Fault pipe discipline"}, \
{rpc_s_fault_pipe_empty, \
    "(rpc_s_fault_pipe_empty) Fault pipe empty"}, \
{rpc_s_fault_pipe_memory, \
    "(rpc_s_fault_pipe_memory) Fault pipe memory"}, \
{rpc_s_fault_pipe_order, \
    "(rpc_s_fault_pipe_order) Fault pipe order"}, \
{rpc_s_fault_remote_comm_failure, \
    "(rpc_s_fault_remote_comm_failure) Fault remote communication failure"}, \
{rpc_s_fault_remote_no_memory, \
    "(rpc_s_fault_remote_no_memory) Fault remote no memory"}, \
{rpc_s_fault_unspec, \
    "(rpc_s_fault_unspec) Fault unspecified"}, \
{uuid_s_bad_version, \
    "(uuid_s_bad_version) Bad UUID version"}, \
{uuid_s_socket_failure, \
    "(uuid_s_socket_failure) Socket failure"}, \
{uuid_s_getconf_failure, \
    "(uuid_s_getconf_failure) get_configuration failure"}, \
{uuid_s_no_address, \
    "(uuid_s_no_address) No IEEE 802 hardware address"}, \
{uuid_s_overrun, \
    "(uuid_s_overrun) "}, \
{uuid_s_internal_error, \
    "(uuid_s_internal_error) Internal error"}, \
{uuid_s_coding_error, \
    "(uuid_s_coding_error) UUID coding error"}, \
{uuid_s_invalid_string_uuid, \
    "(uuid_s_invalid_string_uuid) Invalid string UUID"}, \
{uuid_s_no_memory, \
    "(uuid_s_no_memory) "}, \
{rpc_s_no_more_entries, \
    "(rpc_s_no_more_entries) "}, \
{rpc_s_unknown_ns_error, \
    "(rpc_s_unknown_ns_error) "}, \
{rpc_s_name_service_unavailable, \
    "(rpc_s_name_service_unavailable) Name service unavailable"}, \
{rpc_s_incomplete_name, \
    "(rpc_s_incomplete_name) Incomplete name"}, \
{rpc_s_group_not_found, \
    "(rpc_s_group_not_found) "}, \
{rpc_s_invalid_name_syntax, \
    "(rpc_s_invalid_name_syntax) Invalid name syntax"}, \
{rpc_s_no_more_members, \
    "(rpc_s_no_more_members) No more members"}, \
{rpc_s_no_more_interfaces, \
    "(rpc_s_no_more_interfaces) "}, \
{rpc_s_invalid_name_service, \
    "(rpc_s_invalid_name_service) "}, \
{rpc_s_no_name_mapping, \
    "(rpc_s_no_name_mapping) "}, \
{rpc_s_profile_not_found, \
    "(rpc_s_profile_not_found) "}, \
{rpc_s_not_found, \
    "(rpc_s_not_found) Not found"}, \
{rpc_s_no_updates, \
    "(rpc_s_no_updates) "}, \
{rpc_s_update_failed, \
    "(rpc_s_update_failed) "}, \
{rpc_s_no_match_exported, \
    "(rpc_s_no_match_exported) "}, \
{rpc_s_entry_not_found, \
    "(rpc_s_entry_not_found) Entry not found"}, \
{rpc_s_invalid_inquiry_context, \
    "(rpc_s_invalid_inquiry_context) Invalid inquiry context"}, \
{rpc_s_interface_not_found, \
    "(rpc_s_interface_not_found) Interface not found"}, \
{rpc_s_group_member_not_found, \
    "(rpc_s_group_member_not_found) Group member not found"}, \
{rpc_s_entry_already_exists, \
    "(rpc_s_entry_already_exists) Entry already exists"}, \
{rpc_s_nsinit_failure, \
    "(rpc_s_nsinit_failure) "}, \
{rpc_s_unsupported_name_syntax, \
    "(rpc_s_unsupported_name_syntax) Unsupported name syntax"}, \
{rpc_s_no_more_elements, \
    "(rpc_s_no_more_elements) No more profile elements"}, \
{rpc_s_no_ns_permission, \
    "(rpc_s_no_ns_permission) No permission for name service operation"}, \
{rpc_s_invalid_inquiry_type, \
    "(rpc_s_invalid_inquiry_type) Invalid inquiry type"}, \
{rpc_s_profile_element_not_found, \
    "(rpc_s_profile_element_not_found) Profile element not found"}, \
{rpc_s_profile_element_replaced, \
    "(rpc_s_profile_element_replaced) Profile element replaced"}, \
{rpc_s_import_already_done, \
    "(rpc_s_import_already_done) "}, \
{rpc_s_database_busy, \
    "(rpc_s_database_busy) "}, \
{rpc_s_invalid_import_context, \
    "(rpc_s_invalid_import_context) "}, \
{rpc_s_uuid_set_not_found, \
    "(rpc_s_uuid_set_not_found) "}, \
{rpc_s_uuid_member_not_found, \
    "(rpc_s_uuid_member_not_found) "}, \
{rpc_s_no_interfaces_exported, \
    "(rpc_s_no_interfaces_exported) No interfaces exported"}, \
{rpc_s_tower_set_not_found, \
    "(rpc_s_tower_set_not_found) "}, \
{rpc_s_tower_member_not_found, \
    "(rpc_s_tower_member_not_found) "}, \
{rpc_s_obj_uuid_not_found, \
    "(rpc_s_obj_uuid_not_found) "}, \
{rpc_s_no_more_bindings, \
    "(rpc_s_no_more_bindings) No more bindings"}, \
{rpc_s_invalid_priority, \
    "(rpc_s_invalid_priority) Invalid priority"}, \
{rpc_s_not_rpc_entry, \
    "(rpc_s_not_rpc_entry) Not an RPC entry"}, \
{rpc_s_invalid_lookup_context, \
    "(rpc_s_invalid_lookup_context) "}, \
{rpc_s_binding_vector_full, \
    "(rpc_s_binding_vector_full) Binding vector full"}, \
{rpc_s_cycle_detected, \
    "(rpc_s_cycle_detected) Cycle detected"}, \
{rpc_s_nothing_to_export, \
    "(rpc_s_nothing_to_export) Nothing to export"}, \
{rpc_s_nothing_to_unexport, \
    "(rpc_s_nothing_to_unexport) Nothing to unexport"}, \
{rpc_s_invalid_vers_option, \
    "(rpc_s_invalid_vers_option) Invalid interface version option"}, \
{rpc_s_no_rpc_data, \
    "(rpc_s_no_rpc_data) "}, \
{rpc_s_mbr_picked, \
    "(rpc_s_mbr_picked) Member picked"}, \
{rpc_s_not_all_objs_unexported, \
    "(rpc_s_not_all_objs_unexported) Not all objects unexported"}, \
{rpc_s_no_entry_name, \
    "(rpc_s_no_entry_name) No entry name"}, \
{rpc_s_priority_group_done, \
    "(rpc_s_priority_group_done) Priority group done"}, \
{rpc_s_partial_results, \
    "(rpc_s_partial_results) Partial results"}, \
{rpc_s_no_env_setup, \
    "(rpc_s_no_env_setup) Name service environment variable not defined"}, \
{twr_s_unknown_sa, \
    "(twr_s_unknown_sa) Unknown sockaddr"}, \
{twr_s_unknown_tower, \
    "(twr_s_unknown_tower) Unknown tower"}, \
{twr_s_not_implemented, \
    "(twr_s_not_implemented) Not implemented"}, \
{rpc_s_max_calls_too_small, \
    "(rpc_s_max_calls_too_small) Max calls (threads) too small"}, \
{rpc_s_cthread_create_failed, \
    "(rpc_s_cthread_create_failed) cthread create failed"}, \
{rpc_s_cthread_pool_exists, \
    "(rpc_s_cthread_pool_exists) cthread pool exists"}, \
{rpc_s_cthread_no_such_pool, \
    "(rpc_s_cthread_no_such_pool) No such cthread pool"}, \
{rpc_s_cthread_invoke_disabled, \
    "(rpc_s_cthread_invoke_disabled) cthread invoke disabled"}, \
{ept_s_cant_perform_op, \
    "(ept_s_cant_perform_op) Cannot perform endpoint map operation"}, \
{ept_s_no_memory, \
    "(ept_s_no_memory) No memory for endpoint map service"}, \
{ept_s_database_invalid, \
    "(ept_s_database_invalid) Invalid endpoint database"}, \
{ept_s_cant_create, \
    "(ept_s_cant_create) Cannot create endpoint database"}, \
{ept_s_cant_access, \
    "(ept_s_cant_access) Cannot access endpoint database"}, \
{ept_s_database_already_open, \
    "(ept_s_database_already_open) Endpoint database already open by another process"}, \
{ept_s_invalid_entry, \
    "(ept_s_invalid_entry) Invalid endpoint entry"}, \
{ept_s_update_failed, \
    "(ept_s_update_failed) Cannot update endpoint database"}, \
{ept_s_invalid_context, \
    "(ept_s_invalid_context) Invalid endpoint map or lookup context"}, \
{ept_s_not_registered, \
    "(ept_s_not_registered) Not registered in endpoint map"}, \
{ept_s_server_unavailable, \
    "(ept_s_server_unavailable) "}, \
{rpc_s_underspecified_name, \
    "(rpc_s_underspecified_name) Name is underspecified"}, \
{rpc_s_invalid_ns_handle, \
    "(rpc_s_invalid_ns_handle) Invalid name service handle"}, \
{rpc_s_unknown_error, \
    "(rpc_s_unknown_error) Unidentified communications error in client stub"}, \
{rpc_s_ss_char_trans_open_fail, \
    "(rpc_s_ss_char_trans_open_fail) Could not open file containing ASCII/EBCDIC translation tables"}, \
{rpc_s_ss_char_trans_short_file, \
    "(rpc_s_ss_char_trans_short_file) File containing ASCII/EBCDIC translation tables is damaged"}, \
{rpc_s_ss_context_damaged, \
    "(rpc_s_ss_context_damaged) A client side context handle has been incorrectly modified"}, \
{rpc_s_ss_in_null_context, \
    "(rpc_s_ss_in_null_context) Null value of [in] context handle or all [in,out] context handles"}, \
{rpc_s_socket_failure, \
    "(rpc_s_socket_failure) Persistent errors on socket"}, \
{rpc_s_unsupported_protect_level, \
    "(rpc_s_unsupported_protect_level) Requested protection level is not supported"}, \
{rpc_s_invalid_checksum, \
    "(rpc_s_invalid_checksum) Received packet has an invalid checksum"}, \
{rpc_s_invalid_credentials, \
    "(rpc_s_invalid_credentials) Credentials invalid"}, \
{rpc_s_credentials_too_large, \
    "(rpc_s_credentials_too_large) Credentials too large for packet"}, \
{rpc_s_call_id_not_found, \
    "(rpc_s_call_id_not_found) Call ID in packet unknown"}, \
{rpc_s_key_id_not_found, \
    "(rpc_s_key_id_not_found) Key ID in packet unknown"}, \
{rpc_s_auth_bad_integrity, \
    "(rpc_s_auth_bad_integrity) Decrypt integrity check failed"}, \
{rpc_s_auth_tkt_expired, \
    "(rpc_s_auth_tkt_expired) Authentication ticket expired"}, \
{rpc_s_auth_tkt_nyv, \
    "(rpc_s_auth_tkt_nyv) Authentication ticket not yet valid"}, \
{rpc_s_auth_repeat, \
    "(rpc_s_auth_repeat) Authentication request is a replay"}, \
{rpc_s_auth_not_us, \
    "(rpc_s_auth_not_us) Authentication ticket is not for destination"}, \
{rpc_s_auth_badmatch, \
    "(rpc_s_auth_badmatch) Authentication ticket/authenticator do not match"}, \
{rpc_s_auth_skew, \
    "(rpc_s_auth_skew) Clock skew too great to authenticate"}, \
{rpc_s_auth_badaddr, \
    "(rpc_s_auth_badaddr) Incorrect network address in authentication request"}, \
{rpc_s_auth_badversion, \
    "(rpc_s_auth_badversion) Authentication protocol version mismatch"}, \
{rpc_s_auth_msg_type, \
    "(rpc_s_auth_msg_type) Invalid authentication message type"}, \
{rpc_s_auth_modified, \
    "(rpc_s_auth_modified) Authentication message stream modified"}, \
{rpc_s_auth_badorder, \
    "(rpc_s_auth_badorder) Authentication message out of order"}, \
{rpc_s_auth_badkeyver, \
    "(rpc_s_auth_badkeyver) Authentication key version not available"}, \
{rpc_s_auth_nokey, \
    "(rpc_s_auth_nokey) Authentication service key not available"}, \
{rpc_s_auth_mut_fail, \
    "(rpc_s_auth_mut_fail) Mutual authentication failed"}, \
{rpc_s_auth_baddirection, \
    "(rpc_s_auth_baddirection) Incorrect authentication message direction"}, \
{rpc_s_auth_method, \
    "(rpc_s_auth_method) Alternative authentication method required"}, \
{rpc_s_auth_badseq, \
    "(rpc_s_auth_badseq) Incorrect sequence number in authentication message"}, \
{rpc_s_auth_inapp_cksum, \
    "(rpc_s_auth_inapp_cksum) Inappropriate authentication checksum type"}, \
{rpc_s_auth_field_toolong, \
    "(rpc_s_auth_field_toolong) Authentication field too long for implementation"}, \
{rpc_s_invalid_crc, \
    "(rpc_s_invalid_crc) Received packet has an invalid CRC"}, \
{rpc_s_binding_incomplete, \
    "(rpc_s_binding_incomplete) Binding incomplete (no object ID and no endpoint)"}, \
{rpc_s_key_func_not_allowed, \
    "(rpc_s_key_func_not_allowed) Key function not allowed when default authentication service specified"}, \
{rpc_s_unknown_stub_rtl_if_vers, \
    "(rpc_s_unknown_stub_rtl_if_vers) Interface's stub/runtime version is unknown"}, \
{rpc_s_unknown_ifspec_vers, \
    "(rpc_s_unknown_ifspec_vers) Interface's version is unknown"}, \
{rpc_s_proto_unsupp_by_auth, \
    "(rpc_s_proto_unsupp_by_auth) RPC protocol not supported by this authentication protocol"}, \
{rpc_s_authn_challenge_malformed, \
    "(rpc_s_authn_challenge_malformed) Authentication challenge malformed"}, \
{rpc_s_protect_level_mismatch, \
    "(rpc_s_protect_level_mismatch) Protection level changed unexpectedly"}, \
{rpc_s_no_mepv, \
    "(rpc_s_no_mepv) No manager EPV available"}, \
{rpc_s_stub_protocol_error, \
    "(rpc_s_stub_protocol_error) Stub or runtime protocol error"}, \
{rpc_s_class_version_mismatch, \
    "(rpc_s_class_version_mismatch) RPC class version mismatch"}, \
{rpc_s_helper_not_running, \
    "(rpc_s_helper_not_running) Helper process not running"}, \
{rpc_s_helper_short_read, \
    "(rpc_s_helper_short_read) Short read from kernel helper"}, \
{rpc_s_helper_catatonic, \
    "(rpc_s_helper_catatonic) Helper process catatonic"}, \
{rpc_s_helper_aborted, \
    "(rpc_s_helper_aborted) Helper process aborted"}, \
{rpc_s_not_in_kernel, \
    "(rpc_s_not_in_kernel) Feature not supported in kernel"}, \
{rpc_s_helper_wrong_user, \
    "(rpc_s_helper_wrong_user) Attempting to use credentials belonging to another user"}, \
{rpc_s_helper_overflow, \
    "(rpc_s_helper_overflow) Helper process too busy"}, \
{rpc_s_dg_need_way_auth, \
    "(rpc_s_dg_need_way_auth) DG protocol needs reauthentication."}, \
{rpc_s_unsupported_auth_subtype, \
    "(rpc_s_unsupported_auth_subtype) Receiver cannot support authentication subtype"}, \
{rpc_s_wrong_pickle_type, \
    "(rpc_s_wrong_pickle_type) Wrong type of pickle passed to unpickling routine"}, \
{rpc_s_not_listening, \
    "(rpc_s_not_listening) Listener thread is not running"}, \
{rpc_s_ss_bad_buffer, \
    "(rpc_s_ss_bad_buffer) Buffer not usable by IDL Encoding Services"}, \
{rpc_s_ss_bad_es_action, \
    "(rpc_s_ss_bad_es_action) Action cannot be performed by IDL Encoding Services"}, \
{rpc_s_ss_wrong_es_version, \
    "(rpc_s_ss_wrong_es_version) Wrong version of IDL Encoding Services"}, \
{rpc_s_fault_user_defined, \
    "(rpc_s_fault_user_defined) User defined exception received"}, \
{rpc_s_ss_incompatible_codesets, \
    "(rpc_s_ss_incompatible_codesets) Conversion between codesets not possible"}, \
{rpc_s_tx_not_in_transaction, \
    "(rpc_s_tx_not_in_transaction) Transaction not started before operation"}, \
{rpc_s_tx_open_failed, \
    "(rpc_s_tx_open_failed) Transaction open failed at server"}, \
{rpc_s_partial_credentials, \
    "(rpc_s_partial_credentials) Credentials have been fragmented"}, \
{rpc_s_ss_invalid_codeset_tag, \
    "(rpc_s_ss_invalid_codeset_tag) I18N tags structure is not valid"}, \
{rpc_s_mgmt_bad_type, \
    "(rpc_s_mgmt_bad_type) Unsupported attribute type was given to NSI"}, \
{rpc_s_ss_invalid_char_input, \
    "(rpc_s_ss_invalid_char_input) Invalid character input for conversion"}, \
{rpc_s_ss_short_conv_buffer, \
    "(rpc_s_ss_short_conv_buffer) No room to place the converted characters"}, \
{rpc_s_ss_iconv_error, \
    "(rpc_s_ss_iconv_error) iconv failed other than conversion operation"}, \
{rpc_s_ss_no_compat_codeset, \
    "(rpc_s_ss_no_compat_codeset) No compatible code set found"}, \
{rpc_s_ss_no_compat_charsets, \
    "(rpc_s_ss_no_compat_charsets) Character sets are not compatible"}, \
{dce_cs_c_ok, \
    "(dce_cs_c_ok) Code set registry access succeeded."}, \
{dce_cs_c_unknown, \
    "(dce_cs_c_unknown) Value not found in the code set registry"}, \
{dce_cs_c_notfound, \
    "(dce_cs_c_notfound) No local code set name exists in the code set registry"}, \
{dce_cs_c_cannot_open_file, \
    "(dce_cs_c_cannot_open_file) Cannot open the code set registry file"}, \
{dce_cs_c_cannot_read_file, \
    "(dce_cs_c_cannot_read_file) Cannot read the code set registry file"}, \
{dce_cs_c_cannot_allocate_memory, \
    "(dce_cs_c_cannot_allocate_memory) Cannot allocate memory for code set info"}, \
{rpc_s_ss_cleanup_failed, \
    "(rpc_s_ss_cleanup_failed) Cleanup failed within an evaluation routine."}, \
{rpc_svc_desc_general, \
    "(rpc_svc_desc_general) "}, \
{rpc_svc_desc_mutex, \
    "(rpc_svc_desc_mutex) "}, \
{rpc_svc_desc_xmit, \
    "(rpc_svc_desc_xmit) "}, \
{rpc_svc_desc_recv, \
    "(rpc_svc_desc_recv) "}, \
{rpc_svc_desc_dg_state, \
    "(rpc_svc_desc_dg_state) "}, \
{rpc_svc_desc_cancel, \
    "(rpc_svc_desc_cancel) "}, \
{rpc_svc_desc_orphan, \
    "(rpc_svc_desc_orphan) "}, \
{rpc_svc_desc_cn_state, \
    "(rpc_svc_desc_cn_state) "}, \
{rpc_svc_desc_cn_pkt, \
    "(rpc_svc_desc_cn_pkt) "}, \
{rpc_svc_desc_pkt_quotas, \
    "(rpc_svc_desc_pkt_quotas) "}, \
{rpc_svc_desc_auth, \
    "(rpc_svc_desc_auth) "}, \
{rpc_svc_desc_source, \
    "(rpc_svc_desc_source) "}, \
{rpc_svc_desc_stats, \
    "(rpc_svc_desc_stats) "}, \
{rpc_svc_desc_mem, \
    "(rpc_svc_desc_mem) "}, \
{rpc_svc_desc_mem_type, \
    "(rpc_svc_desc_mem_type) "}, \
{rpc_svc_desc_dg_pktlog, \
    "(rpc_svc_desc_dg_pktlog) "}, \
{rpc_svc_desc_thread_id, \
    "(rpc_svc_desc_thread_id) "}, \
{rpc_svc_desc_timestamp, \
    "(rpc_svc_desc_timestamp) "}, \
{rpc_svc_desc_cn_errors, \
    "(rpc_svc_desc_cn_errors) "}, \
{rpc_svc_desc_conv_thread, \
    "(rpc_svc_desc_conv_thread) "}, \
{rpc_svc_desc_pid, \
    "(rpc_svc_desc_pid) "}, \
{rpc_svc_desc_atfork, \
    "(rpc_svc_desc_atfork) "}, \
{rpc_svc_desc_cma_thread, \
    "(rpc_svc_desc_cma_thread) "}, \
{rpc_svc_desc_inherit, \
    "(rpc_svc_desc_inherit) "}, \
{rpc_svc_desc_dg_sockets, \
    "(rpc_svc_desc_dg_sockets) "}, \
{rpc_svc_desc_timer, \
    "(rpc_svc_desc_timer) "}, \
{rpc_svc_desc_threads, \
    "(rpc_svc_desc_threads) "}, \
{rpc_svc_desc_server_call, \
    "(rpc_svc_desc_server_call) "}, \
{rpc_svc_desc_nsi, \
    "(rpc_svc_desc_nsi) "}, \
{rpc_svc_desc_dg_pkt, \
    "(rpc_svc_desc_dg_pkt) "}, \
{rpc_m_cn_ill_state_trans_sa, \
    "(rpc_m_cn_ill_state_trans_sa) Illegal state transition detected in CN server association state machine"}, \
{rpc_m_cn_ill_state_trans_ca, \
    "(rpc_m_cn_ill_state_trans_ca) Illegal state transition detected in CN client association state machine"}, \
{rpc_m_cn_ill_state_trans_sg, \
    "(rpc_m_cn_ill_state_trans_sg) Illegal state transition detected in CN server association group state machine"}, \
{rpc_m_cn_ill_state_trans_cg, \
    "(rpc_m_cn_ill_state_trans_cg) Illegal state transition detected in CN client association group state machine"}, \
{rpc_m_cn_ill_state_trans_sr, \
    "(rpc_m_cn_ill_state_trans_sr) Illegal state transition detected in CN server call state machine"}, \
{rpc_m_cn_ill_state_trans_cr, \
    "(rpc_m_cn_ill_state_trans_cr) Illegal state transition detected in CN client call state machine"}, \
{rpc_m_bad_pkt_type, \
    "(rpc_m_bad_pkt_type) Illegal or unknown packet type"}, \
{rpc_m_prot_mismatch, \
    "(rpc_m_prot_mismatch) (receive_packet) Protocol version mismatch"}, \
{rpc_m_frag_toobig, \
    "(rpc_m_frag_toobig) (receive_packet) frag_length in header > fragbuf data size"}, \
{rpc_m_unsupp_stub_rtl_if, \
    "(rpc_m_unsupp_stub_rtl_if) Unsupported stub/RTL IF version"}, \
{rpc_m_unhandled_callstate, \
    "(rpc_m_unhandled_callstate) Unhandled call state"}, \
{rpc_m_call_failed, \
    "(rpc_m_call_failed) Call failed"}, \
{rpc_m_call_failed_no_status, \
    "(rpc_m_call_failed_no_status) Call failed"}, \
{rpc_m_call_failed_errno, \
    "(rpc_m_call_failed_errno) Call failed"}, \
{rpc_m_call_failed_s, \
    "(rpc_m_call_failed_s) Call on server failed"}, \
{rpc_m_call_failed_c, \
    "(rpc_m_call_failed_c) Call on client failed"}, \
{rpc_m_errmsg_toobig, \
    "(rpc_m_errmsg_toobig) Error message will not fit in packet"}, \
{rpc_m_invalid_srchattr, \
    "(rpc_m_invalid_srchattr) Unexpected search attribute seen"}, \
{rpc_m_nts_not_found, \
    "(rpc_m_nts_not_found) Negotiated transfer syntax not found in presentation context element"}, \
{rpc_m_invalid_accbytcnt, \
    "(rpc_m_invalid_accbytcnt) Inconsistency in ACC_BYTCNT field"}, \
{rpc_m_pre_v2_ifspec, \
    "(rpc_m_pre_v2_ifspec) Pre-v2 interface spec"}, \
{rpc_m_unk_ifspec, \
    "(rpc_m_unk_ifspec) Unknown interface spec version"}, \
{rpc_m_recvbuf_toosmall, \
    "(rpc_m_recvbuf_toosmall) Socket's maximum receive buffering is less than NCA Connection Protocol minimum requirement"}, \
{rpc_m_unalign_authtrl, \
    "(rpc_m_unalign_authtrl) Unaligned RPC_CN_PKT_AUTH_TRL"}, \
{rpc_m_unexpected_exc, \
    "(rpc_m_unexpected_exc) Unexpected exception was raised"}, \
{rpc_m_no_stub_data, \
    "(rpc_m_no_stub_data) No stub data to send"}, \
{rpc_m_eventlist_full, \
    "(rpc_m_eventlist_full) Event list full"}, \
{rpc_m_unk_sock_type, \
    "(rpc_m_unk_sock_type) Unknown socket type"}, \
{rpc_m_unimp_call, \
    "(rpc_m_unimp_call) Call not implemented"}, \
{rpc_m_invalid_seqnum, \
    "(rpc_m_invalid_seqnum) Invalid call sequence number"}, \
{rpc_m_cant_create_uuid, \
    "(rpc_m_cant_create_uuid) Can't create UUID"}, \
{rpc_m_pre_v2_ss, \
    "(rpc_m_pre_v2_ss) Can't handle pre-v2 server stubs"}, \
{rpc_m_dgpkt_pool_corrupt, \
    "(rpc_m_dgpkt_pool_corrupt) DG packet free pool is corrupted"}, \
{rpc_m_dgpkt_bad_free, \
    "(rpc_m_dgpkt_bad_free) Attempt to free already-freed DG packet"}, \
{rpc_m_lookaside_corrupt, \
    "(rpc_m_lookaside_corrupt) Lookaside list is corrupted"}, \
{rpc_m_alloc_fail, \
    "(rpc_m_alloc_fail) Memory allocation failed"}, \
{rpc_m_realloc_fail, \
    "(rpc_m_realloc_fail) Memory reallocation failed"}, \
{rpc_m_cant_open_file, \
    "(rpc_m_cant_open_file) Can't open file"}, \
{rpc_m_cant_read_addr, \
    "(rpc_m_cant_read_addr) "}, \
{rpc_svc_desc_libidl, \
    "(rpc_svc_desc_libidl) "}, \
{rpc_m_ctxrundown_nomem, \
    "(rpc_m_ctxrundown_nomem) Out of memory while trying to run down contexts of client"}, \
{rpc_m_ctxrundown_exc, \
    "(rpc_m_ctxrundown_exc) Exception in routine, running down context of client"}, \
{rpc_s_fault_codeset_conv_error, \
    "(rpc_s_fault_codeset_conv_error) Fault codeset conversion error"}, \
};

#define VMDIR_DFL_UNKNOWN "UNKNOWN"
#define VMDIR_DFL_5_5 "5.5"
#define VMDIR_DFL_6_0 "6.0"
#define VMDIR_DFL_6_5 "6.5"
#define VMDIR_DFL_6_6 "6.6"
#define VMDIR_DFL_DEFAULT 1

#define VMDIR_DFL_VERSION_INITIALIZER   \
{                                       \
    { 1, VMDIR_DFL_5_5 },               \
    { 1, VMDIR_DFL_6_0 },               \
    { 2, VMDIR_DFL_6_5 },               \
    { 3, VMDIR_DFL_6_6 }                \
}
