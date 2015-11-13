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

import java.io.Closeable;
import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 2/1/12
 * Time: 3:49 PM
 * To change this template use File | Settings | File Templates.
 */
// in java >= 1.7 we can also consider switching to java.lang.AutoCloseable.
public interface ILdapConnection extends Closeable {

    public void setOption(LdapOption option, int value);

    public void setOption(LdapOption option, boolean value);

    public void bindConnection( String dn, String cred, LdapBindMethod method );

    public void bindSaslConnection(String userName, String domainName, String userPassword, String krbConfPath);

    public void addObject( String dn, LdapMod[] attributes );

    public void addObject( String dn, Collection<LdapMod> attributes );

    public void modifyObject( String dn, LdapMod attribute );

    public void modifyObject( String dn, LdapMod[] attributes );

    public void modifyObject( String dn, Collection<LdapMod> attributes );

    public ILdapMessage search( String base, LdapScope scope, String filter, String[] attributes, boolean attributesOnly );

    public ILdapMessage search( String base, LdapScope scope, String filter, Collection<String> attributes, boolean attributesOnly );

    public ILdapMessage search_ext(String base, LdapScope scope, String filter, Collection<String> attributes,
                                   boolean attributesOnly, Collection<LdapControlNative> sctrls,
                                   Collection<LdapControlNative> cctrls, TimevalNative timeout, int sizelimit);

    public ILdapPagedSearchResult search_one_page(String base, LdapScope scope, String filter, Collection<String> attributes, int pageSize, ILdapPagedSearchResult prevPagedSearchResult);

    public Collection<ILdapMessage> paged_search(String base, LdapScope scope, String filter, Collection<String> attributes, int pageSize);

    public void deleteObject(String dn);

    public void deleteObjectTree(String dn);

    @Override
    public void close();  // we don't expect dispose to throw

    public void bindSaslSrpConnection(String upn, String userPassword);
}
