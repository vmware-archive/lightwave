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

DWORD
VMCADCEGetErrorCode(
    dcethread_exc* pDceException
)
{
    DWORD dwError = 0;
    dwError = dcethread_exc_getstatus (pDceException);
//FixME  AnuE - find right mapping/move to common gobuild component
	return dwError;
}

DWORD
VMCAGetDceRpcShortErrorString(DWORD dwRpcError, PSTR* szErrorMessage)
{
    PSTR s = "rpc_unknown";
    PSTR szError = NULL;
    DWORD dwError = 0;
    switch (dwRpcError)
    {
    case rpc_s_mod                       : s = "rpc_s_mod"; break;
    case rpc_s_op_rng_error              : s = "rpc_s_op_rng_error"; break;
    case rpc_s_cant_create_socket        : s = "rpc_s_cant_create_socket"; break;
    case rpc_s_cant_bind_socket          : s = "rpc_s_cant_bind_socket"; break;
    case rpc_s_not_in_call               : s = "rpc_s_not_in_call"; break;
    case rpc_s_no_port                   : s = "rpc_s_no_port"; break;
    case rpc_s_wrong_boot_time           : s = "rpc_s_wrong_boot_time"; break;
    case rpc_s_too_many_sockets          : s = "rpc_s_too_many_sockets"; break;
    case rpc_s_illegal_register          : s = "rpc_s_illegal_register"; break;
    case rpc_s_cant_recv                 : s = "rpc_s_cant_recv"; break;
    case rpc_s_bad_pkt                   : s = "rpc_s_bad_pkt"; break;
    case rpc_s_unbound_handle            : s = "rpc_s_unbound_handle"; break;
    case rpc_s_addr_in_use               : s = "rpc_s_addr_in_use"; break;
    case rpc_s_in_args_too_big           : s = "rpc_s_in_args_too_big"; break;
    case rpc_s_string_too_long           : s = "rpc_s_string_too_long"; break;
    case rpc_s_too_many_objects          : s = "rpc_s_too_many_objects"; break;
    case rpc_s_binding_has_no_auth       : s = "rpc_s_binding_has_no_auth"; break;
    case rpc_s_unknown_authn_service     : s = "rpc_s_unknown_authn_service"; break;
    case rpc_s_no_memory                 : s = "rpc_s_no_memory"; break;
    case rpc_s_cant_nmalloc              : s = "rpc_s_cant_nmalloc"; break;
    case rpc_s_call_faulted              : s = "rpc_s_call_faulted"; break;
    case rpc_s_call_failed               : s = "rpc_s_call_failed"; break;
    case rpc_s_comm_failure              : s = "rpc_s_comm_failure"; break;
    case rpc_s_rpcd_comm_failure         : s = "rpc_s_rpcd_comm_failure"; break;
    case rpc_s_illegal_family_rebind     : s = "rpc_s_illegal_family_rebind"; break;
    case rpc_s_invalid_handle            : s = "rpc_s_invalid_handle"; break;
    case rpc_s_coding_error              : s = "rpc_s_coding_error"; break;
    case rpc_s_object_not_found          : s = "rpc_s_object_not_found"; break;
    case rpc_s_cthread_not_found         : s = "rpc_s_cthread_not_found"; break;
    case rpc_s_invalid_binding           : s = "rpc_s_invalid_binding"; break;
    case rpc_s_already_registered        : s = "rpc_s_already_registered"; break;
    case rpc_s_endpoint_not_found        : s = "rpc_s_endpoint_not_found"; break;
    case rpc_s_invalid_rpc_protseq       : s = "rpc_s_invalid_rpc_protseq"; break;
    case rpc_s_desc_not_registered       : s = "rpc_s_desc_not_registered"; break;
    case rpc_s_already_listening         : s = "rpc_s_already_listening"; break;
    case rpc_s_no_protseqs               : s = "rpc_s_no_protseqs"; break;
    case rpc_s_no_protseqs_registered    : s = "rpc_s_no_protseqs_registered"; break;
    case rpc_s_no_bindings               : s = "rpc_s_no_bindings"; break;
    case rpc_s_max_descs_exceeded        : s = "rpc_s_max_descs_exceeded"; break;
    case rpc_s_no_interfaces             : s = "rpc_s_no_interfaces"; break;
    case rpc_s_invalid_timeout           : s = "rpc_s_invalid_timeout"; break;
    case rpc_s_cant_inq_socket           : s = "rpc_s_cant_inq_socket"; break;
    case rpc_s_invalid_naf_id            : s = "rpc_s_invalid_naf_id"; break;
    case rpc_s_inval_net_addr            : s = "rpc_s_inval_net_addr"; break;
    case rpc_s_unknown_if                : s = "rpc_s_unknown_if"; break;
    case rpc_s_unsupported_type          : s = "rpc_s_unsupported_type"; break;
    case rpc_s_invalid_call_opt          : s = "rpc_s_invalid_call_opt"; break;
    case rpc_s_no_fault                  : s = "rpc_s_no_fault"; break;
    case rpc_s_cancel_timeout            : s = "rpc_s_cancel_timeout"; break;
    case rpc_s_call_cancelled            : s = "rpc_s_call_cancelled"; break;
    case rpc_s_invalid_call_handle       : s = "rpc_s_invalid_call_handle"; break;
    case rpc_s_cannot_alloc_assoc        : s = "rpc_s_cannot_alloc_assoc"; break;
    case rpc_s_cannot_connect            : s = "rpc_s_cannot_connect"; break;
    case rpc_s_connection_aborted        : s = "rpc_s_connection_aborted"; break;
    case rpc_s_connection_closed         : s = "rpc_s_connection_closed"; break;
    case rpc_s_cannot_accept             : s = "rpc_s_cannot_accept"; break;
    case rpc_s_assoc_grp_not_found       : s = "rpc_s_assoc_grp_not_found"; break;
    case rpc_s_stub_interface_error      : s = "rpc_s_stub_interface_error"; break;
    case rpc_s_invalid_object            : s = "rpc_s_invalid_object"; break;
    case rpc_s_invalid_type              : s = "rpc_s_invalid_type"; break;
    case rpc_s_invalid_if_opnum          : s = "rpc_s_invalid_if_opnum"; break;
    case rpc_s_different_server_instance : s = "rpc_s_different_server_instance"; break;
    case rpc_s_protocol_error            : s = "rpc_s_protocol_error"; break;
    case rpc_s_cant_recvmsg              : s = "rpc_s_cant_recvmsg"; break;
    case rpc_s_invalid_string_binding    : s = "rpc_s_invalid_string_binding"; break;
    case rpc_s_connect_timed_out         : s = "rpc_s_connect_timed_out"; break;
    case rpc_s_connect_rejected          : s = "rpc_s_connect_rejected"; break;
    case rpc_s_network_unreachable       : s = "rpc_s_network_unreachable"; break;
    case rpc_s_connect_no_resources      : s = "rpc_s_connect_no_resources"; break;
    case rpc_s_rem_network_shutdown      : s = "rpc_s_rem_network_shutdown"; break;
    case rpc_s_too_many_rem_connects     : s = "rpc_s_too_many_rem_connects"; break;
    case rpc_s_no_rem_endpoint           : s = "rpc_s_no_rem_endpoint"; break;
    case rpc_s_rem_host_down             : s = "rpc_s_rem_host_down"; break;
    case rpc_s_host_unreachable          : s = "rpc_s_host_unreachable"; break;
    case rpc_s_access_control_info_inv   : s = "rpc_s_access_control_info_inv"; break;
    case rpc_s_loc_connect_aborted       : s = "rpc_s_loc_connect_aborted"; break;
    case rpc_s_connect_closed_by_rem     : s = "rpc_s_connect_closed_by_rem"; break;
    case rpc_s_rem_host_crashed          : s = "rpc_s_rem_host_crashed"; break;
    case rpc_s_invalid_endpoint_format   : s = "rpc_s_invalid_endpoint_format"; break;
    case rpc_s_unknown_status_code       : s = "rpc_s_unknown_status_code"; break;
    case rpc_s_unknown_mgr_type          : s = "rpc_s_unknown_mgr_type"; break;
    case rpc_s_assoc_creation_failed     : s = "rpc_s_assoc_creation_failed"; break;
    case rpc_s_assoc_grp_max_exceeded    : s = "rpc_s_assoc_grp_max_exceeded"; break;
    case rpc_s_assoc_grp_alloc_failed    : s = "rpc_s_assoc_grp_alloc_failed"; break;
    case rpc_s_sm_invalid_state          : s = "rpc_s_sm_invalid_state"; break;
    case rpc_s_assoc_req_rejected        : s = "rpc_s_assoc_req_rejected"; break;
    case rpc_s_assoc_shutdown            : s = "rpc_s_assoc_shutdown"; break;
    case rpc_s_tsyntaxes_unsupported     : s = "rpc_s_tsyntaxes_unsupported"; break;
    case rpc_s_context_id_not_found      : s = "rpc_s_context_id_not_found"; break;
    case rpc_s_cant_listen_socket        : s = "rpc_s_cant_listen_socket"; break;
    case rpc_s_no_addrs                  : s = "rpc_s_no_addrs"; break;
    case rpc_s_cant_getpeername          : s = "rpc_s_cant_getpeername"; break;
    case rpc_s_cant_get_if_id            : s = "rpc_s_cant_get_if_id"; break;
    case rpc_s_protseq_not_supported     : s = "rpc_s_protseq_not_supported"; break;
    case rpc_s_call_orphaned             : s = "rpc_s_call_orphaned"; break;
    case rpc_s_who_are_you_failed        : s = "rpc_s_who_are_you_failed"; break;
    case rpc_s_unknown_reject            : s = "rpc_s_unknown_reject"; break;
    case rpc_s_type_already_registered   : s = "rpc_s_type_already_registered"; break;
    case rpc_s_stop_listening_disabled   : s = "rpc_s_stop_listening_disabled"; break;
    case rpc_s_invalid_arg               : s = "rpc_s_invalid_arg"; break;
    case rpc_s_not_supported             : s = "rpc_s_not_supported"; break;
    case rpc_s_wrong_kind_of_binding     : s = "rpc_s_wrong_kind_of_binding"; break;
    case rpc_s_authn_authz_mismatch      : s = "rpc_s_authn_authz_mismatch"; break;
    case rpc_s_call_queued               : s = "rpc_s_call_queued"; break;
    case rpc_s_cannot_set_nodelay        : s = "rpc_s_cannot_set_nodelay"; break;
    case rpc_s_not_rpc_tower             : s = "rpc_s_not_rpc_tower"; break;
    case rpc_s_invalid_rpc_protid        : s = "rpc_s_invalid_rpc_protid"; break;
    case rpc_s_invalid_rpc_floor         : s = "rpc_s_invalid_rpc_floor"; break;
    case rpc_s_call_timeout              : s = "rpc_s_call_timeout"; break;
    case rpc_s_mgmt_op_disallowed        : s = "rpc_s_mgmt_op_disallowed"; break;
    case rpc_s_manager_not_entered       : s = "rpc_s_manager_not_entered"; break;
    case rpc_s_calls_too_large_for_wk_ep : s = "rpc_s_calls_too_large_for_wk_ep"; break;
    case rpc_s_server_too_busy           : s = "rpc_s_server_too_busy"; break;
    case rpc_s_prot_version_mismatch     : s = "rpc_s_prot_version_mismatch"; break;
    case rpc_s_rpc_prot_version_mismatch : s = "rpc_s_rpc_prot_version_mismatch"; break;
    case rpc_s_ss_no_import_cursor       : s = "rpc_s_ss_no_import_cursor"; break;
    case rpc_s_fault_addr_error          : s = "rpc_s_fault_addr_error"; break;
    case rpc_s_fault_context_mismatch    : s = "rpc_s_fault_context_mismatch"; break;
    case rpc_s_fault_fp_div_by_zero      : s = "rpc_s_fault_fp_div_by_zero"; break;
    case rpc_s_fault_fp_error            : s = "rpc_s_fault_fp_error"; break;
    case rpc_s_fault_fp_overflow         : s = "rpc_s_fault_fp_overflow"; break;
    case rpc_s_fault_fp_underflow        : s = "rpc_s_fault_fp_underflow"; break;
    case rpc_s_fault_ill_inst            : s = "rpc_s_fault_ill_inst"; break;
    case rpc_s_fault_int_div_by_zero     : s = "rpc_s_fault_int_div_by_zero"; break;
    case rpc_s_fault_int_overflow        : s = "rpc_s_fault_int_overflow"; break;
    case rpc_s_fault_invalid_bound       : s = "rpc_s_fault_invalid_bound"; break;
    case rpc_s_fault_invalid_tag         : s = "rpc_s_fault_invalid_tag"; break;
    case rpc_s_fault_pipe_closed         : s = "rpc_s_fault_pipe_closed"; break;
    case rpc_s_fault_pipe_comm_error     : s = "rpc_s_fault_pipe_comm_error"; break;
    case rpc_s_fault_pipe_discipline     : s = "rpc_s_fault_pipe_discipline"; break;
    case rpc_s_fault_pipe_empty          : s = "rpc_s_fault_pipe_empty"; break;
    case rpc_s_fault_pipe_memory         : s = "rpc_s_fault_pipe_memory"; break;
    case rpc_s_fault_pipe_order          : s = "rpc_s_fault_pipe_order"; break;
    case rpc_s_fault_remote_comm_failure : s = "rpc_s_fault_remote_comm_failure"; break;
    case rpc_s_fault_remote_no_memory    : s = "rpc_s_fault_remote_no_memory"; break;
    case rpc_s_fault_unspec              : s = "rpc_s_fault_unspec"; break;
    case uuid_s_bad_version              : s = "uuid_s_bad_version"; break;
    case uuid_s_socket_failure           : s = "uuid_s_socket_failure"; break;
    case uuid_s_getconf_failure          : s = "uuid_s_getconf_failure"; break;
    case uuid_s_no_address               : s = "uuid_s_no_address"; break;
    case uuid_s_overrun                  : s = "uuid_s_overrun"; break;
    case uuid_s_internal_error           : s = "uuid_s_internal_error"; break;
    case uuid_s_coding_error             : s = "uuid_s_coding_error"; break;
    case uuid_s_invalid_string_uuid      : s = "uuid_s_invalid_string_uuid"; break;
    case uuid_s_no_memory                : s = "uuid_s_no_memory"; break;
    case rpc_s_no_more_entries           : s = "rpc_s_no_more_entries"; break;
    case rpc_s_unknown_ns_error          : s = "rpc_s_unknown_ns_error"; break;
    case rpc_s_name_service_unavailable  : s = "rpc_s_name_service_unavailable"; break;
    case rpc_s_incomplete_name           : s = "rpc_s_incomplete_name"; break;
    case rpc_s_group_not_found           : s = "rpc_s_group_not_found"; break;
    case rpc_s_invalid_name_syntax       : s = "rpc_s_invalid_name_syntax"; break;
    case rpc_s_no_more_members           : s = "rpc_s_no_more_members"; break;
    case rpc_s_no_more_interfaces        : s = "rpc_s_no_more_interfaces"; break;
    case rpc_s_invalid_name_service      : s = "rpc_s_invalid_name_service"; break;
    case rpc_s_no_name_mapping           : s = "rpc_s_no_name_mapping"; break;
    case rpc_s_profile_not_found         : s = "rpc_s_profile_not_found"; break;
    case rpc_s_not_found                 : s = "rpc_s_not_found"; break;
    case rpc_s_no_updates                : s = "rpc_s_no_updates"; break;
    case rpc_s_update_failed             : s = "rpc_s_update_failed"; break;
    case rpc_s_no_match_exported         : s = "rpc_s_no_match_exported"; break;
    case rpc_s_entry_not_found           : s = "rpc_s_entry_not_found"; break;
    case rpc_s_invalid_inquiry_context   : s = "rpc_s_invalid_inquiry_context"; break;
    case rpc_s_interface_not_found       : s = "rpc_s_interface_not_found"; break;
    case rpc_s_group_member_not_found    : s = "rpc_s_group_member_not_found"; break;
    case rpc_s_entry_already_exists      : s = "rpc_s_entry_already_exists"; break;
    case rpc_s_nsinit_failure            : s = "rpc_s_nsinit_failure"; break;
    case rpc_s_unsupported_name_syntax   : s = "rpc_s_unsupported_name_syntax"; break;
    case rpc_s_no_more_elements          : s = "rpc_s_no_more_elements"; break;
    case rpc_s_no_ns_permission          : s = "rpc_s_no_ns_permission"; break;
    case rpc_s_invalid_inquiry_type      : s = "rpc_s_invalid_inquiry_type"; break;
    case rpc_s_profile_element_not_found : s = "rpc_s_profile_element_not_found"; break;
    case rpc_s_profile_element_replaced  : s = "rpc_s_profile_element_replaced"; break;
    case rpc_s_import_already_done       : s = "rpc_s_import_already_done"; break;
    case rpc_s_database_busy             : s = "rpc_s_database_busy"; break;
    case rpc_s_invalid_import_context    : s = "rpc_s_invalid_import_context"; break;
    case rpc_s_uuid_set_not_found        : s = "rpc_s_uuid_set_not_found"; break;
    case rpc_s_uuid_member_not_found     : s = "rpc_s_uuid_member_not_found"; break;
    case rpc_s_no_interfaces_exported    : s = "rpc_s_no_interfaces_exported"; break;
    case rpc_s_tower_set_not_found       : s = "rpc_s_tower_set_not_found"; break;
    case rpc_s_tower_member_not_found    : s = "rpc_s_tower_member_not_found"; break;
    case rpc_s_obj_uuid_not_found        : s = "rpc_s_obj_uuid_not_found"; break;
    case rpc_s_no_more_bindings          : s = "rpc_s_no_more_bindings"; break;
    case rpc_s_invalid_priority          : s = "rpc_s_invalid_priority"; break;
    case rpc_s_not_rpc_entry             : s = "rpc_s_not_rpc_entry"; break;
    case rpc_s_invalid_lookup_context    : s = "rpc_s_invalid_lookup_context"; break;
    case rpc_s_binding_vector_full       : s = "rpc_s_binding_vector_full"; break;
    case rpc_s_cycle_detected            : s = "rpc_s_cycle_detected"; break;
    case rpc_s_nothing_to_export         : s = "rpc_s_nothing_to_export"; break;
    case rpc_s_nothing_to_unexport       : s = "rpc_s_nothing_to_unexport"; break;
    case rpc_s_invalid_vers_option       : s = "rpc_s_invalid_vers_option"; break;
    case rpc_s_no_rpc_data               : s = "rpc_s_no_rpc_data"; break;
    case rpc_s_mbr_picked                : s = "rpc_s_mbr_picked"; break;
    case rpc_s_not_all_objs_unexported   : s = "rpc_s_not_all_objs_unexported"; break;
    case rpc_s_no_entry_name             : s = "rpc_s_no_entry_name"; break;
    case rpc_s_priority_group_done       : s = "rpc_s_priority_group_done"; break;
    case rpc_s_partial_results           : s = "rpc_s_partial_results"; break;
    case rpc_s_no_env_setup              : s = "rpc_s_no_env_setup"; break;
    case twr_s_unknown_sa                : s = "twr_s_unknown_sa"; break;
    case twr_s_unknown_tower             : s = "twr_s_unknown_tower"; break;
    case twr_s_not_implemented           : s = "twr_s_not_implemented"; break;
    case rpc_s_max_calls_too_small       : s = "rpc_s_max_calls_too_small"; break;
    case rpc_s_cthread_create_failed     : s = "rpc_s_cthread_create_failed"; break;
    case rpc_s_cthread_pool_exists       : s = "rpc_s_cthread_pool_exists"; break;
    case rpc_s_cthread_no_such_pool      : s = "rpc_s_cthread_no_such_pool"; break;
    case rpc_s_cthread_invoke_disabled   : s = "rpc_s_cthread_invoke_disabled"; break;
    case ept_s_cant_perform_op           : s = "ept_s_cant_perform_op"; break;
    case ept_s_no_memory                 : s = "ept_s_no_memory"; break;
    case ept_s_database_invalid          : s = "ept_s_database_invalid"; break;
    case ept_s_cant_create               : s = "ept_s_cant_create"; break;
    case ept_s_cant_access               : s = "ept_s_cant_access"; break;
    case ept_s_database_already_open     : s = "ept_s_database_already_open"; break;
    case ept_s_invalid_entry             : s = "ept_s_invalid_entry"; break;
    case ept_s_update_failed             : s = "ept_s_update_failed"; break;
    case ept_s_invalid_context           : s = "ept_s_invalid_context"; break;
    case ept_s_not_registered            : s = "ept_s_not_registered"; break;
    case ept_s_server_unavailable        : s = "ept_s_server_unavailable"; break;
    case rpc_s_underspecified_name       : s = "rpc_s_underspecified_name"; break;
    case rpc_s_invalid_ns_handle         : s = "rpc_s_invalid_ns_handle"; break;
    case rpc_s_unknown_error             : s = "rpc_s_unknown_error"; break;
    case rpc_s_ss_char_trans_open_fail   : s = "rpc_s_ss_char_trans_open_fail"; break;
    case rpc_s_ss_char_trans_short_file  : s = "rpc_s_ss_char_trans_short_file"; break;
    case rpc_s_ss_context_damaged        : s = "rpc_s_ss_context_damaged"; break;
    case rpc_s_ss_in_null_context        : s = "rpc_s_ss_in_null_context"; break;
    case rpc_s_socket_failure            : s = "rpc_s_socket_failure"; break;
    case rpc_s_unsupported_protect_level : s = "rpc_s_unsupported_protect_level"; break;
    case rpc_s_invalid_checksum          : s = "rpc_s_invalid_checksum"; break;
    case rpc_s_invalid_credentials       : s = "rpc_s_invalid_credentials"; break;
    case rpc_s_credentials_too_large     : s = "rpc_s_credentials_too_large"; break;
    case rpc_s_call_id_not_found         : s = "rpc_s_call_id_not_found"; break;
    case rpc_s_key_id_not_found          : s = "rpc_s_key_id_not_found"; break;
    case rpc_s_auth_bad_integrity        : s = "rpc_s_auth_bad_integrity"; break;
    case rpc_s_auth_tkt_expired          : s = "rpc_s_auth_tkt_expired"; break;
    case rpc_s_auth_tkt_nyv              : s = "rpc_s_auth_tkt_nyv"; break;
    case rpc_s_auth_repeat               : s = "rpc_s_auth_repeat"; break;
    case rpc_s_auth_not_us               : s = "rpc_s_auth_not_us"; break;
    case rpc_s_auth_badmatch             : s = "rpc_s_auth_badmatch"; break;
    case rpc_s_auth_skew                 : s = "rpc_s_auth_skew"; break;
    case rpc_s_auth_badaddr              : s = "rpc_s_auth_badaddr"; break;
    case rpc_s_auth_badversion           : s = "rpc_s_auth_badversion"; break;
    case rpc_s_auth_msg_type             : s = "rpc_s_auth_msg_type"; break;
    case rpc_s_auth_modified             : s = "rpc_s_auth_modified"; break;
    case rpc_s_auth_badorder             : s = "rpc_s_auth_badorder"; break;
    case rpc_s_auth_badkeyver            : s = "rpc_s_auth_badkeyver"; break;
    case rpc_s_auth_nokey                : s = "rpc_s_auth_nokey"; break;
    case rpc_s_auth_mut_fail             : s = "rpc_s_auth_mut_fail"; break;
    case rpc_s_auth_baddirection         : s = "rpc_s_auth_baddirection"; break;
    case rpc_s_auth_method               : s = "rpc_s_auth_method"; break;
    case rpc_s_auth_badseq               : s = "rpc_s_auth_badseq"; break;
    case rpc_s_auth_inapp_cksum          : s = "rpc_s_auth_inapp_cksum"; break;
    case rpc_s_auth_field_toolong        : s = "rpc_s_auth_field_toolong"; break;
    case rpc_s_invalid_crc               : s = "rpc_s_invalid_crc"; break;
    case rpc_s_binding_incomplete        : s = "rpc_s_binding_incomplete"; break;
    case rpc_s_key_func_not_allowed      : s = "rpc_s_key_func_not_allowed"; break;
    case rpc_s_unknown_stub_rtl_if_vers  : s = "rpc_s_unknown_stub_rtl_if_vers"; break;
    case rpc_s_unknown_ifspec_vers       : s = "rpc_s_unknown_ifspec_vers"; break;
    case rpc_s_proto_unsupp_by_auth      : s = "rpc_s_proto_unsupp_by_auth"; break;
    case rpc_s_authn_challenge_malformed : s = "rpc_s_authn_challenge_malformed"; break;
    case rpc_s_protect_level_mismatch    : s = "rpc_s_protect_level_mismatch"; break;
    case rpc_s_no_mepv                   : s = "rpc_s_no_mepv"; break;
    case rpc_s_stub_protocol_error       : s = "rpc_s_stub_protocol_error"; break;
    case rpc_s_class_version_mismatch    : s = "rpc_s_class_version_mismatch"; break;
    case rpc_s_helper_not_running        : s = "rpc_s_helper_not_running"; break;
    case rpc_s_helper_short_read         : s = "rpc_s_helper_short_read"; break;
    case rpc_s_helper_catatonic          : s = "rpc_s_helper_catatonic"; break;
    case rpc_s_helper_aborted            : s = "rpc_s_helper_aborted"; break;
    case rpc_s_not_in_kernel             : s = "rpc_s_not_in_kernel"; break;
    case rpc_s_helper_wrong_user         : s = "rpc_s_helper_wrong_user"; break;
    case rpc_s_helper_overflow           : s = "rpc_s_helper_overflow"; break;
    case rpc_s_dg_need_way_auth          : s = "rpc_s_dg_need_way_auth"; break;
    case rpc_s_unsupported_auth_subtype  : s = "rpc_s_unsupported_auth_subtype"; break;
    case rpc_s_wrong_pickle_type         : s = "rpc_s_wrong_pickle_type"; break;
    case rpc_s_not_listening             : s = "rpc_s_not_listening"; break;
    case rpc_s_ss_bad_buffer             : s = "rpc_s_ss_bad_buffer"; break;
    case rpc_s_ss_bad_es_action          : s = "rpc_s_ss_bad_es_action"; break;
    case rpc_s_ss_wrong_es_version       : s = "rpc_s_ss_wrong_es_version"; break;
    case rpc_s_fault_user_defined        : s = "rpc_s_fault_user_defined"; break;
    case rpc_s_ss_incompatible_codesets  : s = "rpc_s_ss_incompatible_codesets"; break;
    case rpc_s_tx_not_in_transaction     : s = "rpc_s_tx_not_in_transaction"; break;
    case rpc_s_tx_open_failed            : s = "rpc_s_tx_open_failed"; break;
    case rpc_s_partial_credentials       : s = "rpc_s_partial_credentials"; break;
    case rpc_s_ss_invalid_codeset_tag    : s = "rpc_s_ss_invalid_codeset_tag"; break;
    case rpc_s_mgmt_bad_type             : s = "rpc_s_mgmt_bad_type"; break;
    case rpc_s_ss_invalid_char_input     : s = "rpc_s_ss_invalid_char_input"; break;
    case rpc_s_ss_short_conv_buffer      : s = "rpc_s_ss_short_conv_buffer"; break;
    case rpc_s_ss_iconv_error            : s = "rpc_s_ss_iconv_error"; break;
    case rpc_s_ss_no_compat_codeset      : s = "rpc_s_ss_no_compat_codeset"; break;
    case rpc_s_ss_no_compat_charsets     : s = "rpc_s_ss_no_compat_charsets"; break;
    case dce_cs_c_ok                     : s = "dce_cs_c_ok"; break;
    case dce_cs_c_unknown                : s = "dce_cs_c_unknown"; break;
    case dce_cs_c_notfound               : s = "dce_cs_c_notfound"; break;
    case dce_cs_c_cannot_open_file       : s = "dce_cs_c_cannot_open_file"; break;
    case dce_cs_c_cannot_read_file       : s = "dce_cs_c_cannot_read_file"; break;
    case dce_cs_c_cannot_allocate_memory : s = "dce_cs_c_cannot_allocate_memory"; break;
    case rpc_s_ss_cleanup_failed         : s = "rpc_s_ss_cleanup_failed"; break;
    case rpc_svc_desc_general            : s = "rpc_svc_desc_general"; break;
    case rpc_svc_desc_mutex              : s = "rpc_svc_desc_mutex"; break;
    case rpc_svc_desc_xmit               : s = "rpc_svc_desc_xmit"; break;
    case rpc_svc_desc_recv               : s = "rpc_svc_desc_recv"; break;
    case rpc_svc_desc_dg_state           : s = "rpc_svc_desc_dg_state"; break;
    case rpc_svc_desc_cancel             : s = "rpc_svc_desc_cancel"; break;
    case rpc_svc_desc_orphan             : s = "rpc_svc_desc_orphan"; break;
    case rpc_svc_desc_cn_state           : s = "rpc_svc_desc_cn_state"; break;
    case rpc_svc_desc_cn_pkt             : s = "rpc_svc_desc_cn_pkt"; break;
    case rpc_svc_desc_pkt_quotas         : s = "rpc_svc_desc_pkt_quotas"; break;
    case rpc_svc_desc_auth               : s = "rpc_svc_desc_auth"; break;
    case rpc_svc_desc_source             : s = "rpc_svc_desc_source"; break;
    case rpc_svc_desc_stats              : s = "rpc_svc_desc_stats"; break;
    case rpc_svc_desc_mem                : s = "rpc_svc_desc_mem"; break;
    case rpc_svc_desc_mem_type           : s = "rpc_svc_desc_mem_type"; break;
    case rpc_svc_desc_dg_pktlog          : s = "rpc_svc_desc_dg_pktlog"; break;
    case rpc_svc_desc_thread_id          : s = "rpc_svc_desc_thread_id"; break;
    case rpc_svc_desc_timestamp          : s = "rpc_svc_desc_timestamp"; break;
    case rpc_svc_desc_cn_errors          : s = "rpc_svc_desc_cn_errors"; break;
    case rpc_svc_desc_conv_thread        : s = "rpc_svc_desc_conv_thread"; break;
    case rpc_svc_desc_pid                : s = "rpc_svc_desc_pid"; break;
    case rpc_svc_desc_atfork             : s = "rpc_svc_desc_atfork"; break;
    case rpc_svc_desc_cma_thread         : s = "rpc_svc_desc_cma_thread"; break;
    case rpc_svc_desc_inherit            : s = "rpc_svc_desc_inherit"; break;
    case rpc_svc_desc_dg_sockets         : s = "rpc_svc_desc_dg_sockets"; break;
    case rpc_svc_desc_timer              : s = "rpc_svc_desc_timer"; break;
    case rpc_svc_desc_threads            : s = "rpc_svc_desc_threads"; break;
    case rpc_svc_desc_server_call        : s = "rpc_svc_desc_server_call"; break;
    case rpc_svc_desc_nsi                : s = "rpc_svc_desc_nsi"; break;
    case rpc_svc_desc_dg_pkt             : s = "rpc_svc_desc_dg_pkt"; break;
    case rpc_m_cn_ill_state_trans_sa     : s = "rpc_m_cn_ill_state_trans_sa"; break;
    case rpc_m_cn_ill_state_trans_ca     : s = "rpc_m_cn_ill_state_trans_ca"; break;
    case rpc_m_cn_ill_state_trans_sg     : s = "rpc_m_cn_ill_state_trans_sg"; break;
    case rpc_m_cn_ill_state_trans_cg     : s = "rpc_m_cn_ill_state_trans_cg"; break;
    case rpc_m_cn_ill_state_trans_sr     : s = "rpc_m_cn_ill_state_trans_sr"; break;
    case rpc_m_cn_ill_state_trans_cr     : s = "rpc_m_cn_ill_state_trans_cr"; break;
    case rpc_m_bad_pkt_type              : s = "rpc_m_bad_pkt_type"; break;
    case rpc_m_prot_mismatch             : s = "rpc_m_prot_mismatch"; break;
    case rpc_m_frag_toobig               : s = "rpc_m_frag_toobig"; break;
    case rpc_m_unsupp_stub_rtl_if        : s = "rpc_m_unsupp_stub_rtl_if"; break;
    case rpc_m_unhandled_callstate       : s = "rpc_m_unhandled_callstate"; break;
    case rpc_m_call_failed               : s = "rpc_m_call_failed"; break;
    case rpc_m_call_failed_no_status     : s = "rpc_m_call_failed_no_status"; break;
    case rpc_m_call_failed_errno         : s = "rpc_m_call_failed_errno"; break;
    case rpc_m_call_failed_s             : s = "rpc_m_call_failed_s"; break;
    case rpc_m_call_failed_c             : s = "rpc_m_call_failed_c"; break;
    case rpc_m_errmsg_toobig             : s = "rpc_m_errmsg_toobig"; break;
    case rpc_m_invalid_srchattr          : s = "rpc_m_invalid_srchattr"; break;
    case rpc_m_nts_not_found             : s = "rpc_m_nts_not_found"; break;
    case rpc_m_invalid_accbytcnt         : s = "rpc_m_invalid_accbytcnt"; break;
    case rpc_m_pre_v2_ifspec             : s = "rpc_m_pre_v2_ifspec"; break;
    case rpc_m_unk_ifspec                : s = "rpc_m_unk_ifspec"; break;
    case rpc_m_recvbuf_toosmall          : s = "rpc_m_recvbuf_toosmall"; break;
    case rpc_m_unalign_authtrl           : s = "rpc_m_unalign_authtrl"; break;
    case rpc_m_unexpected_exc            : s = "rpc_m_unexpected_exc"; break;
    case rpc_m_no_stub_data              : s = "rpc_m_no_stub_data"; break;
    case rpc_m_eventlist_full            : s = "rpc_m_eventlist_full"; break;
    case rpc_m_unk_sock_type             : s = "rpc_m_unk_sock_type"; break;
    case rpc_m_unimp_call                : s = "rpc_m_unimp_call"; break;
    case rpc_m_invalid_seqnum            : s = "rpc_m_invalid_seqnum"; break;
    case rpc_m_cant_create_uuid          : s = "rpc_m_cant_create_uuid"; break;
    case rpc_m_pre_v2_ss                 : s = "rpc_m_pre_v2_ss"; break;
    case rpc_m_dgpkt_pool_corrupt        : s = "rpc_m_dgpkt_pool_corrupt"; break;
    case rpc_m_dgpkt_bad_free            : s = "rpc_m_dgpkt_bad_free"; break;
    case rpc_m_lookaside_corrupt         : s = "rpc_m_lookaside_corrupt"; break;
    case rpc_m_alloc_fail                : s = "rpc_m_alloc_fail"; break;
    case rpc_m_realloc_fail              : s = "rpc_m_realloc_fail"; break;
    case rpc_m_cant_open_file            : s = "rpc_m_cant_open_file"; break;
    case rpc_m_cant_read_addr            : s = "rpc_m_cant_read_addr"; break;
    case rpc_svc_desc_libidl             : s = "rpc_svc_desc_libidl"; break;
    case rpc_m_ctxrundown_nomem          : s = "rpc_m_ctxrundown_nomem"; break;
    case rpc_m_ctxrundown_exc            : s = "rpc_m_ctxrundown_exc"; break;
    case rpc_s_fault_codeset_conv_error  : s = "rpc_s_fault_codeset_conv_error"; break;
    }
    dwError = VMCAAllocateStringA(s, &szError);
    BAIL_ON_VMCA_ERROR(dwError);

    *szErrorMessage = szError;
cleanup:
    return dwError;

error:
    VMCA_SAFE_FREE_STRINGA(szError);
    goto cleanup;
}


DWORD
VMCAGetDceRpcErrorString(DWORD dwRpcError, PSTR* szErrorMessage)
{
    PSTR s = "unknown rpc error occured, error = %s (0x%08x).";
    PSTR szError = NULL;
    PSTR szShortError = NULL;
    DWORD dwError = 0;
    switch (dwRpcError)
    {
    case rpc_s_mod                       :
    case rpc_s_op_rng_error              :
    case rpc_s_cant_create_socket        :
    case rpc_s_cant_bind_socket          :
    case rpc_s_not_in_call               :
    case rpc_s_no_port                   :
    case rpc_s_wrong_boot_time           :
    case rpc_s_too_many_sockets          :
    case rpc_s_illegal_register          :
    case rpc_s_cant_recv                 :
    case rpc_s_bad_pkt                   :
    case rpc_s_unbound_handle            :
    case rpc_s_addr_in_use               :
    case rpc_s_in_args_too_big           :
    case rpc_s_string_too_long           :
    case rpc_s_too_many_objects          :
    case rpc_s_binding_has_no_auth       :
    case rpc_s_unknown_authn_service     :
    case rpc_s_no_memory                 :
    case rpc_s_cant_nmalloc              :
    case rpc_s_call_faulted              :
    case rpc_s_call_failed               :
    case rpc_s_comm_failure              :
    case rpc_s_rpcd_comm_failure         :
    case rpc_s_illegal_family_rebind     :
    case rpc_s_invalid_handle            :
    case rpc_s_coding_error              :
    case rpc_s_object_not_found          :
    case rpc_s_cthread_not_found         :
    case rpc_s_invalid_binding           :
    case rpc_s_already_registered        :
    case rpc_s_endpoint_not_found        :
    case rpc_s_invalid_rpc_protseq       :
    case rpc_s_desc_not_registered       :
    case rpc_s_already_listening         :
    case rpc_s_no_protseqs               :
    case rpc_s_no_protseqs_registered    :
    case rpc_s_no_bindings               :
    case rpc_s_max_descs_exceeded        :
    case rpc_s_no_interfaces             :
    case rpc_s_invalid_timeout           :
    case rpc_s_cant_inq_socket           :
    case rpc_s_invalid_naf_id            :
    case rpc_s_inval_net_addr            :
    case rpc_s_unknown_if                :
    case rpc_s_unsupported_type          :
    case rpc_s_invalid_call_opt          :
    case rpc_s_no_fault                  :
    case rpc_s_cancel_timeout            :
    case rpc_s_call_cancelled            :
    case rpc_s_invalid_call_handle       :
    case rpc_s_cannot_alloc_assoc        :
    case rpc_s_cannot_connect            :
    case rpc_s_connection_aborted        :
    case rpc_s_connection_closed         :
    case rpc_s_cannot_accept             :
    case rpc_s_assoc_grp_not_found       :
    case rpc_s_stub_interface_error      :
    case rpc_s_invalid_object            :
    case rpc_s_invalid_type              :
    case rpc_s_invalid_if_opnum          :
    case rpc_s_different_server_instance :
    case rpc_s_protocol_error            :
    case rpc_s_cant_recvmsg              :
    case rpc_s_invalid_string_binding    :
    case rpc_s_connect_timed_out         :
    case rpc_s_connect_rejected          :
    case rpc_s_network_unreachable       :
    case rpc_s_connect_no_resources      :
    case rpc_s_rem_network_shutdown      :
    case rpc_s_too_many_rem_connects     :
    case rpc_s_no_rem_endpoint           :
    case rpc_s_rem_host_down             :
    case rpc_s_host_unreachable          :
    case rpc_s_access_control_info_inv   :
    case rpc_s_loc_connect_aborted       :
    case rpc_s_connect_closed_by_rem     :
    case rpc_s_rem_host_crashed          :
    case rpc_s_invalid_endpoint_format   :
    case rpc_s_unknown_status_code       :
    case rpc_s_unknown_mgr_type          :
    case rpc_s_assoc_creation_failed     :
    case rpc_s_assoc_grp_max_exceeded    :
    case rpc_s_assoc_grp_alloc_failed    :
    case rpc_s_sm_invalid_state          :
    case rpc_s_assoc_req_rejected        :
    case rpc_s_assoc_shutdown            :
    case rpc_s_tsyntaxes_unsupported     :
    case rpc_s_context_id_not_found      :
    case rpc_s_cant_listen_socket        :
    case rpc_s_no_addrs                  :
    case rpc_s_cant_getpeername          :
    case rpc_s_cant_get_if_id            :
    case rpc_s_protseq_not_supported     :
    case rpc_s_call_orphaned             :
    case rpc_s_who_are_you_failed        :
    case rpc_s_unknown_reject            :
    case rpc_s_type_already_registered   :
    case rpc_s_stop_listening_disabled   :
    case rpc_s_invalid_arg               :
    case rpc_s_not_supported             :
    case rpc_s_wrong_kind_of_binding     :
    case rpc_s_authn_authz_mismatch      :
    case rpc_s_call_queued               :
    case rpc_s_cannot_set_nodelay        :
    case rpc_s_not_rpc_tower             :
    case rpc_s_invalid_rpc_protid        :
    case rpc_s_invalid_rpc_floor         :
    case rpc_s_call_timeout              :
    case rpc_s_mgmt_op_disallowed        :
    case rpc_s_manager_not_entered       :
    case rpc_s_calls_too_large_for_wk_ep :
    case rpc_s_server_too_busy           :
    case rpc_s_prot_version_mismatch     :
    case rpc_s_rpc_prot_version_mismatch :
    case rpc_s_ss_no_import_cursor       :
    case rpc_s_fault_addr_error          :
    case rpc_s_fault_context_mismatch    :
    case rpc_s_fault_fp_div_by_zero      :
    case rpc_s_fault_fp_error            :
    case rpc_s_fault_fp_overflow         :
    case rpc_s_fault_fp_underflow        :
    case rpc_s_fault_ill_inst            :
    case rpc_s_fault_int_div_by_zero     :
    case rpc_s_fault_int_overflow        :
    case rpc_s_fault_invalid_bound       :
    case rpc_s_fault_invalid_tag         :
    case rpc_s_fault_pipe_closed         :
    case rpc_s_fault_pipe_comm_error     :
    case rpc_s_fault_pipe_discipline     :
    case rpc_s_fault_pipe_empty          :
    case rpc_s_fault_pipe_memory         :
    case rpc_s_fault_pipe_order          :
    case rpc_s_fault_remote_comm_failure :
    case rpc_s_fault_remote_no_memory    :
    case rpc_s_fault_unspec              :
    case uuid_s_bad_version              :
    case uuid_s_socket_failure           :
    case uuid_s_getconf_failure          :
    case uuid_s_no_address               :
    case uuid_s_overrun                  :
    case uuid_s_internal_error           :
    case uuid_s_coding_error             :
    case uuid_s_invalid_string_uuid      :
    case uuid_s_no_memory                :
    case rpc_s_no_more_entries           :
    case rpc_s_unknown_ns_error          :
    case rpc_s_name_service_unavailable  :
    case rpc_s_incomplete_name           :
    case rpc_s_group_not_found           :
    case rpc_s_invalid_name_syntax       :
    case rpc_s_no_more_members           :
    case rpc_s_no_more_interfaces        :
    case rpc_s_invalid_name_service      :
    case rpc_s_no_name_mapping           :
    case rpc_s_profile_not_found         :
    case rpc_s_not_found                 :
    case rpc_s_no_updates                :
    case rpc_s_update_failed             :
    case rpc_s_no_match_exported         :
    case rpc_s_entry_not_found           :
    case rpc_s_invalid_inquiry_context   :
    case rpc_s_interface_not_found       :
    case rpc_s_group_member_not_found    :
    case rpc_s_entry_already_exists      :
    case rpc_s_nsinit_failure            :
    case rpc_s_unsupported_name_syntax   :
    case rpc_s_no_more_elements          :
    case rpc_s_no_ns_permission          :
    case rpc_s_invalid_inquiry_type      :
    case rpc_s_profile_element_not_found :
    case rpc_s_profile_element_replaced  :
    case rpc_s_import_already_done       :
    case rpc_s_database_busy             :
    case rpc_s_invalid_import_context    :
    case rpc_s_uuid_set_not_found        :
    case rpc_s_uuid_member_not_found     :
    case rpc_s_no_interfaces_exported    :
    case rpc_s_tower_set_not_found       :
    case rpc_s_tower_member_not_found    :
    case rpc_s_obj_uuid_not_found        :
    case rpc_s_no_more_bindings          :
    case rpc_s_invalid_priority          :
    case rpc_s_not_rpc_entry             :
    case rpc_s_invalid_lookup_context    :
    case rpc_s_binding_vector_full       :
    case rpc_s_cycle_detected            :
    case rpc_s_nothing_to_export         :
    case rpc_s_nothing_to_unexport       :
    case rpc_s_invalid_vers_option       :
    case rpc_s_no_rpc_data               :
    case rpc_s_mbr_picked                :
    case rpc_s_not_all_objs_unexported   :
    case rpc_s_no_entry_name             :
    case rpc_s_priority_group_done       :
    case rpc_s_partial_results           :
    case rpc_s_no_env_setup              :
    case twr_s_unknown_sa                :
    case twr_s_unknown_tower             :
    case twr_s_not_implemented           :
    case rpc_s_max_calls_too_small       :
    case rpc_s_cthread_create_failed     :
    case rpc_s_cthread_pool_exists       :
    case rpc_s_cthread_no_such_pool      :
    case rpc_s_cthread_invoke_disabled   :
    case ept_s_cant_perform_op           :
    case ept_s_no_memory                 :
    case ept_s_database_invalid          :
    case ept_s_cant_create               :
    case ept_s_cant_access               :
    case ept_s_database_already_open     :
    case ept_s_invalid_entry             :
    case ept_s_update_failed             :
    case ept_s_invalid_context           :
    case ept_s_not_registered            :
    case ept_s_server_unavailable        :
    case rpc_s_underspecified_name       :
    case rpc_s_invalid_ns_handle         :
    case rpc_s_unknown_error             :
    case rpc_s_ss_char_trans_open_fail   :
    case rpc_s_ss_char_trans_short_file  :
    case rpc_s_ss_context_damaged        :
    case rpc_s_ss_in_null_context        :
    case rpc_s_socket_failure            :
    case rpc_s_unsupported_protect_level :
    case rpc_s_invalid_checksum          :
    case rpc_s_invalid_credentials       :
    case rpc_s_credentials_too_large     :
    case rpc_s_call_id_not_found         :
    case rpc_s_key_id_not_found          :
        {
            s = "Failed to connect to the remote host, reason = %s (0x%08x).";
            break;
        }
    case rpc_s_auth_bad_integrity        :
    case rpc_s_auth_tkt_expired          :
    case rpc_s_auth_tkt_nyv              :
    case rpc_s_auth_repeat               :
    case rpc_s_auth_not_us               :
    case rpc_s_auth_badmatch             :
    case rpc_s_auth_skew                 :
    case rpc_s_auth_badaddr              :
    case rpc_s_auth_badversion           :
    case rpc_s_auth_msg_type             :
    case rpc_s_auth_modified             :
    case rpc_s_auth_badorder             :
    case rpc_s_auth_badkeyver            :
    case rpc_s_auth_nokey                :
    case rpc_s_auth_mut_fail             :
    case rpc_s_auth_baddirection         :
    case rpc_s_auth_method               :
    case rpc_s_auth_badseq               :
    case rpc_s_auth_inapp_cksum          :
    case rpc_s_auth_field_toolong        :
        {
            s = "Access denied, reason = %s (0x%08x).";
            break;
        }
    case rpc_s_invalid_crc               :
    case rpc_s_binding_incomplete        :
    case rpc_s_key_func_not_allowed      :
    case rpc_s_unknown_stub_rtl_if_vers  :
    case rpc_s_unknown_ifspec_vers       :
    case rpc_s_proto_unsupp_by_auth      :
    case rpc_s_authn_challenge_malformed :
    case rpc_s_protect_level_mismatch    :
    case rpc_s_no_mepv                   :
    case rpc_s_stub_protocol_error       :
    case rpc_s_class_version_mismatch    :
    case rpc_s_helper_not_running        :
    case rpc_s_helper_short_read         :
    case rpc_s_helper_catatonic          :
    case rpc_s_helper_aborted            :
    case rpc_s_not_in_kernel             :
    case rpc_s_helper_wrong_user         :
    case rpc_s_helper_overflow           :
    case rpc_s_dg_need_way_auth          :
    case rpc_s_unsupported_auth_subtype  :
    case rpc_s_wrong_pickle_type         :
    case rpc_s_not_listening             :
    case rpc_s_ss_bad_buffer             :
    case rpc_s_ss_bad_es_action          :
    case rpc_s_ss_wrong_es_version       :
    case rpc_s_fault_user_defined        :
    case rpc_s_ss_incompatible_codesets  :
    case rpc_s_tx_not_in_transaction     :
    case rpc_s_tx_open_failed            :
    case rpc_s_partial_credentials       :
    case rpc_s_ss_invalid_codeset_tag    :
    case rpc_s_mgmt_bad_type             :
    case rpc_s_ss_invalid_char_input     :
    case rpc_s_ss_short_conv_buffer      :
    case rpc_s_ss_iconv_error            :
    case rpc_s_ss_no_compat_codeset      :
    case rpc_s_ss_no_compat_charsets     :
    case dce_cs_c_ok                     :
    case dce_cs_c_unknown                :
    case dce_cs_c_notfound               :
    case dce_cs_c_cannot_open_file       :
    case dce_cs_c_cannot_read_file       :
    case dce_cs_c_cannot_allocate_memory :
    case rpc_s_ss_cleanup_failed         :
    case rpc_svc_desc_general            :
    case rpc_svc_desc_mutex              :
    case rpc_svc_desc_xmit               :
    case rpc_svc_desc_recv               :
    case rpc_svc_desc_dg_state           :
    case rpc_svc_desc_cancel             :
    case rpc_svc_desc_orphan             :
    case rpc_svc_desc_cn_state           :
    case rpc_svc_desc_cn_pkt             :
    case rpc_svc_desc_pkt_quotas         :
    case rpc_svc_desc_auth               :
    case rpc_svc_desc_source             :
    case rpc_svc_desc_stats              :
    case rpc_svc_desc_mem                :
    case rpc_svc_desc_mem_type           :
    case rpc_svc_desc_dg_pktlog          :
    case rpc_svc_desc_thread_id          :
    case rpc_svc_desc_timestamp          :
    case rpc_svc_desc_cn_errors          :
    case rpc_svc_desc_conv_thread        :
    case rpc_svc_desc_pid                :
    case rpc_svc_desc_atfork             :
    case rpc_svc_desc_cma_thread         :
    case rpc_svc_desc_inherit            :
    case rpc_svc_desc_dg_sockets         :
    case rpc_svc_desc_timer              :
    case rpc_svc_desc_threads            :
    case rpc_svc_desc_server_call        :
    case rpc_svc_desc_nsi                :
    case rpc_svc_desc_dg_pkt             :
    case rpc_m_cn_ill_state_trans_sa     :
    case rpc_m_cn_ill_state_trans_ca     :
    case rpc_m_cn_ill_state_trans_sg     :
    case rpc_m_cn_ill_state_trans_cg     :
    case rpc_m_cn_ill_state_trans_sr     :
    case rpc_m_cn_ill_state_trans_cr     :
    case rpc_m_bad_pkt_type              :
    case rpc_m_prot_mismatch             :
    case rpc_m_frag_toobig               :
    case rpc_m_unsupp_stub_rtl_if        :
    case rpc_m_unhandled_callstate       :
    case rpc_m_call_failed               :
    case rpc_m_call_failed_no_status     :
    case rpc_m_call_failed_errno         :
    case rpc_m_call_failed_s             :
    case rpc_m_call_failed_c             :
    case rpc_m_errmsg_toobig             :
    case rpc_m_invalid_srchattr          :
    case rpc_m_nts_not_found             :
    case rpc_m_invalid_accbytcnt         :
    case rpc_m_pre_v2_ifspec             :
    case rpc_m_unk_ifspec                :
    case rpc_m_recvbuf_toosmall          :
    case rpc_m_unalign_authtrl           :
    case rpc_m_unexpected_exc            :
    case rpc_m_no_stub_data              :
    case rpc_m_eventlist_full            :
    case rpc_m_unk_sock_type             :
    case rpc_m_unimp_call                :
    case rpc_m_invalid_seqnum            :
    case rpc_m_cant_create_uuid          :
    case rpc_m_pre_v2_ss                 :
    case rpc_m_dgpkt_pool_corrupt        :
    case rpc_m_dgpkt_bad_free            :
    case rpc_m_lookaside_corrupt         :
    case rpc_m_alloc_fail                :
    case rpc_m_realloc_fail              :
    case rpc_m_cant_open_file            :
    case rpc_m_cant_read_addr            :
    case rpc_svc_desc_libidl             :
    case rpc_m_ctxrundown_nomem          :
    case rpc_m_ctxrundown_exc            :
    case rpc_s_fault_codeset_conv_error  :
         {
            s = "RPC communication failed with iternal error, reason = %s (0x%08x).";
            break;
         }
    }

    dwError = VMCAGetDceRpcShortErrorString(dwRpcError, &szShortError);
    BAIL_ON_VMCA_ERROR(dwError);
    dwError = VMCAAllocateStringPrintfA(&szError, s, szShortError, dwRpcError);
    BAIL_ON_VMCA_ERROR(dwError);

    *szErrorMessage = szError;
cleanup:
    VMCA_SAFE_FREE_STRINGA(szShortError);
    return dwError;

error:
    VMCA_SAFE_FREE_STRINGA(szError);
    goto cleanup;
}

DWORD
VMCAGetWin32ErrorString(DWORD dwWin32Error, PSTR* szErrorMessage)
{
    PSTR szError = NULL;
    DWORD dwError = 0;

#ifndef _WIN32
    PCSTR szErrorString = VMCAGetWin32ErrorDesc(dwWin32Error);
    dwError = VMCAAllocateStringPrintfA(&szError, "Operation failed with error = %s (%u)", 
                    szErrorString, dwWin32Error);
    BAIL_ON_VMCA_ERROR(dwError);
#else
    DWORD dwNofOfBytes = 0;
    PSTR szWin32Error = NULL;

    dwNofOfBytes = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM
        |FORMAT_MESSAGE_ALLOCATE_BUFFER
        |FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwWin32Error,
        MAKELANGID(LANG_NEUTRAL,
        SUBLANG_DEFAULT
        ),
        (PSTR)&szWin32Error,
        0,
        NULL
        );
    if (szWin32Error != NULL)
    {
        dwError = VMCAAllocateStringPrintfA(&szError, "%s", szWin32Error);
        LocalFree(szWin32Error);
        BAIL_ON_VMCA_ERROR(dwError);
    }
    else
    {
        dwError = VMCAAllocateStringPrintfA(&szError, "Operation failed with error = (%u).", dwWin32Error);
        BAIL_ON_VMCA_ERROR(dwError);
    }
#endif

    *szErrorMessage = szError;
cleanup:
    return dwError;

error:
    VMCA_SAFE_FREE_STRINGA(szError);
    goto cleanup;
}
