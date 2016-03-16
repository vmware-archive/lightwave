/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

import java.net.URI;
import java.util.List;

import com.sun.jna.Callback;
import com.sun.jna.Pointer;
import com.sun.jna.TypeMapper;
import com.sun.jna.ptr.PointerByReference;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 2/2/12
 * Time: 12:41 PM
 * To change this template use File | Settings | File Templates.
 */
interface LdapSaslSRPInteractFunc extends Callback {
    int invoke (Pointer ld, int flags, Pointer pDefaults, Pointer pIn);
    };

interface ILdapClientLibrary {

    LdapConnectionCtx
    ldap_initializeWithUri(URI uri, List<LdapSetting> connOptions);

    Pointer
    ldap_init(String hostName, int  portNumber);

    void
    ldap_set_option(Pointer ld, int option, Pointer value);

    void
    ldap_bind_s(LdapConnectionCtx connectionCtx, String dn, String cred, int method);

    void
    ldap_sasl_bind_s(Pointer ld, String userName, String domainName, String password);

    void
    ldap_add_s(Pointer ld, String dn, Pointer[] attributes);

    void
    ldap_modify_s(Pointer ld, String dn, Pointer[] attributes);

    Pointer
    ldap_search_s(
            Pointer            ld,
            String             base,
            int                scope,
            String             filter,
            String[]           attrs,
            int                attrsonly
    );

    Pointer
    ldap_search_ext_s(
            Pointer            ld,
            String             base,
            int                scope,
            String             filter,
            String[]           attrs,
            int                attrsonly,
            Pointer[]          sctrls,
            Pointer[]          cctrls,
            Pointer            timeout,
            int                sizelimit
    );

    LdapPagedSearchResultPtr
    ldap_one_paged_search(
               Pointer ld,
               String base,
               int scope,
               String filter,
               String[] attrs,
               int pageSize,
               Pointer pBerCookie
               );

    void
    ldap_delete_s(Pointer ld, String dn);

    Pointer
    ldap_get_dn(Pointer ld, Pointer msg);

    Pointer
    ldap_first_entry(Pointer ld, Pointer pMessage);

    Pointer
    ldap_next_entry(Pointer ld, Pointer pEntry);

    int
    ldap_count_entries(Pointer ld, Pointer pMessage);

    Pointer
    ldap_first_attribute(
            Pointer            ld,
            Pointer            pEntry,
            PointerByReference ppBerElem);

    Pointer
    ldap_next_attribute(Pointer ld, Pointer pEntry, Pointer pBerElem);

    Pointer
    ldap_get_values_len(Pointer ld, Pointer pEntry, String attrName);

    int
    ldap_count_values_len(Pointer ppBerValues);

    void
    ldap_value_free_len(Pointer ppBerValues);

    void
    ldap_unbind(Pointer ld);

    void
    ldap_msgfree(Pointer msg);

    void
    ldap_memfree(Pointer mem);

    void
    ber_free( Pointer berElement, int freebuf );

    void
    ber_bvfree( Pointer berVal );

    String
    ldap_err2string( int errorCode );

    String
    getString(Pointer pDN);

    void
    ldap_sasl_srp_bind_s(Pointer ld, String bindDN, String password);

    TypeMapper
    getTypeMapper();
}
