/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */

package com.vmware.identity.idm.server.provider;

import java.security.cert.X509Certificate;
import java.util.Collection;

import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.provider.activedirectory.ActiveDirectoryProvider;
import com.vmware.identity.idm.server.provider.ldap.LdapProvider;
import com.vmware.identity.idm.server.provider.ldap.LdapWithAdMappingsProvider;
import com.vmware.identity.idm.server.provider.localos.LocalOsIdentityProvider;
import com.vmware.identity.idm.server.provider.vmwdirectory.SystemDomainAliasedProvider;
import com.vmware.identity.idm.server.provider.vmwdirectory.VMwareDirectoryProvider;

public class ProviderFactory implements IProviderFactory
{
    public
    IIdentityProvider buildProvider(String tenantName, IIdentityStoreData store, Collection<X509Certificate> trustedCertificates) throws Exception
    {
        IIdentityProvider provider = null;
        IIdentityStoreDataEx storeDataEx = null;
        if(store != null)
        {
            storeDataEx = store.getExtendedIdentityStoreData();
        }

        if ( (store!=null) && (storeDataEx != null))
        {
            switch (storeDataEx.getProviderType())
            {
                case IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY:
                {
                    provider = new ActiveDirectoryProvider(tenantName, store);
                    break;
                }
                case IDENTITY_STORE_TYPE_LDAP:
                {
                    provider = new LdapProvider(tenantName, store, trustedCertificates);
                    break;
                }
                case IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING:
                {
                    provider = new LdapWithAdMappingsProvider(tenantName, store, trustedCertificates);
                    break;
                }
                case IDENTITY_STORE_TYPE_VMWARE_DIRECTORY:
                {
                    IdmServerConfig settings = IdmServerConfig.getInstance();
                    boolean isSystemDomain = store.getName().equalsIgnoreCase(settings.getDirectoryConfigStoreDomain());

                    // if we are in back compat mode and it is a service provider system domain
                    if ( ( settings.isServiceProviderSystemDomainInBackCompatMode() == true )
                         &&
                         ( isSystemDomain )
                         &&
                         ( ServerUtils.isNullOrEmpty(storeDataEx.getAlias()) == false ) )
                    {

                        provider = new SystemDomainAliasedProvider(tenantName, store, settings.getServiceProviderSystemDomianUserAliases() );
                    }
                    else
                    {
                        provider = new VMwareDirectoryProvider(tenantName, store, isSystemDomain);
                    }
                    break;
                }
                case IDENTITY_STORE_TYPE_LOCAL_OS:
                {
                    provider = new LocalOsIdentityProvider( store );
                    break;
                }
                default :
                {
                    throw new UnsupportedOperationException(
                            String.format(
                                    "The identity store [%s] is not supported",
                                    store.getName()));
                }
            }
        }

        if (provider == null)
        {
            throw new IllegalStateException(String.format("Failed building identity provider using the identity store [%s]",
                                            store.getName()));
        }

        return provider;
    }
}
