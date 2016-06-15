/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace VMDirInterop.LDAP
{
    public class LdapClientLibrary
    {
        public const string LDAP_DLL = "libldap_r.dll";
        public const string LBER_DLL = "liblber.dll";
        public const string CLIENT_DLL = "libvmdirclient.dll";

        public static void SetPath(string path)
        {
            SetDllDirectory(path);
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool SetDllDirectory(string lpPathName);

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_init(
            string HostName,
            int PortNumber
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_set_option(
            IntPtr ld,
            int option,
            IntPtr optdata
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_simple_bind_s(
            IntPtr ldap,
            string dn,
            string passwd
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSafeLDAPBind")]
        public static extern ulong VmDirSafeLDAPBind(
            out IntPtr ld,
            string host,
            string upn,
            string passwd
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_unbind_s(
            IntPtr ld
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_search_ext_s(
            IntPtr ld,
            string querybase,
            int scope,
            string filter,
            string[] attrs,
            int attrsonly,
            IntPtr[] sctrls,
            IntPtr[] cctrls,
            IntPtr timeout,
            int sizelimit,
            ref IntPtr res
            );

        [DllImport(LDAP_DLL, CharSet = CharSet.Auto)]
        public static extern int ldap_add_ext_s(
            IntPtr ld,
            IntPtr basedn,
            IntPtr[] attrs,
            IntPtr[] sctrls,
            IntPtr[] cctrls
        );

        [DllImport(LDAP_DLL, CharSet = CharSet.Auto)]
        public static extern int ldap_modify_ext_s(
            IntPtr ld,
            IntPtr basedn,
            IntPtr[] attrs,
            IntPtr[] sctrls,
            IntPtr[] cctrls
        );


        [DllImport(LDAP_DLL)]
        public static extern int ldap_delete_ext_s(
            IntPtr ld,
            string dn,
            IntPtr[] sctrls,
            IntPtr[] cctrls
        );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_first_entry(
            IntPtr ld,
            IntPtr chain
            );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_next_entry(
            IntPtr ld,
            IntPtr chain
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_count_entries(
            IntPtr ld,
            IntPtr chain
            );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_get_dn(
            IntPtr ld,
            IntPtr entry
            );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_first_attribute(
            IntPtr ld,
            IntPtr entry,
            out IntPtr ber
            );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_next_attribute(
            IntPtr ld,
            IntPtr entry,
            IntPtr ber
            );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_get_values_len(
            IntPtr ld,
            IntPtr entry,
            string target
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_count_values_len(
            IntPtr vals
            );

        [DllImport(LDAP_DLL)]
        public static extern void ldap_value_free_len(
            IntPtr vals
            );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_get_values(
            IntPtr ld,
            IntPtr entry,
            string target
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_count_values(
            IntPtr vals
            );

        [DllImport(LDAP_DLL)]
        public static extern void ldap_value_free(
            IntPtr vals
            );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_msgfree(
            IntPtr ldapMessage
        );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_create_page_control(
            IntPtr ld,
            int pagesize,
            IntPtr cookie,
            int iscritical,
            out IntPtr controlPage
        );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_parse_result(
            IntPtr ld,
            IntPtr msg,
            out int errcode,
            string matcheddnp,
            string errmsgp,
            string[] referralsp,
            ref IntPtr serverctrlsp,
            int freeit
        );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_parse_page_control(
            IntPtr ld,
            IntPtr controls,
            out ulong count,
            ref IntPtr cookie);

        [DllImport(LDAP_DLL)]
        public static extern int ldap_controls_free(
            IntPtr controls
        );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_control_free(
            IntPtr control
        );

        [DllImport(LDAP_DLL)]
        public static extern int ldap_memfree(
            IntPtr ldapPointer
        );

        [DllImport(LBER_DLL)]
        public static extern int ber_free(
            IntPtr berElement,
            int freeBuffer
        );

        [DllImport(LBER_DLL)]
        public static extern int ber_bvfree(
            IntPtr bv
        );

        [DllImport(LDAP_DLL)]
        public static extern IntPtr ldap_err2string(
            int err
        );
    }
}
