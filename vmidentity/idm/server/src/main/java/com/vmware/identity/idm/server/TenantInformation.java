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

package com.vmware.identity.idm.server;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.util.Collection;
import java.util.List;
import java.util.Set;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.ISystemDomainIdentityProvider;

public class TenantInformation
{
    private final Tenant _tenant;
    private final Collection<IIdentityProvider> _providers;
    private final Collection<IIdentityStoreData> _idsStores;
    private final int    _delegationCount;
    private final int    _renewCount;
    private final long   _clockTolerance;
    private final long   _maxHOKLifetime;
    private final long   _maxBearerTokenLifetime;
    private final long _maxBearerRefreshTokenLifetime;
    private final long _maxHoKRefreshTokenLifetime;
    private final String _signatureAlgorithm;
    private final String _brandName;
    private final String _logonBannerTitle;
    private final String _logonBannerContent;
    private final boolean _logonBannerEnableCheckbox;
    private final PrivateKey _key;
    private final List<Certificate> _certificate;
    private final Collection<List<Certificate>> _certificateChains;
    private final IIdentityProvider _adProvider;
    private final Collection<IDPConfig> _externalIdpConfigs;
    private final Collection<Attribute> _attributeDefinitions;
    private final String _entityId;
    private final String _entityAlias;
    private final Collection<String> _defaultProviders;
    private final AuthnPolicy _authnPolicy;
    private final boolean _enableIdpSelection;
    private final ReadWriteLock _rsaConfigFilesLock;
    private RSAAgentConfig _preRsaAgentConfig;

    public
    TenantInformation(
            Tenant tenant,
            Collection<IIdentityProvider> providers,
            Collection<IIdentityStoreData> idsStores,
            IIdentityProvider adProvider,
            int    delegationCount,
            int    renewCount,
            long   clockTolerance,
            long    maxHOKLifetime,
            long    maxBearerTokenLifetime,
            long maxBearerRefreshTokenLifetime,
            long maxHoKRefreshTokenLifetime,
            String signatureAlgorithm,
            String brandName,
            String logonBannerTitle,
            String logonBannerContent,
            boolean logonBannerEnableCheckbox,
            List<Certificate> certificate,
            Collection<List<Certificate>> certificateChains,
            PrivateKey              key,
            Collection<IDPConfig> externalIdpConfigs,
            Collection<Attribute> attributeDefinitions,
            String entityId,
            String alias,
            Collection<String> defaultProviders,
            AuthnPolicy authnPolicy,
            boolean idpSelectionFlag
            )
    {
        _tenant = tenant;
        _providers = providers;
        _idsStores = idsStores;
        _adProvider = adProvider;
        _delegationCount = delegationCount;
        _renewCount = renewCount;
        _clockTolerance = clockTolerance;
        _maxHOKLifetime = maxHOKLifetime;
        _maxBearerTokenLifetime = maxBearerTokenLifetime;
        _maxBearerRefreshTokenLifetime = maxBearerRefreshTokenLifetime;
        _maxHoKRefreshTokenLifetime = maxHoKRefreshTokenLifetime;
        _certificate = certificate;
        _certificateChains = certificateChains;
        _key          = key;
        _signatureAlgorithm = signatureAlgorithm;
        _brandName = brandName;
        _logonBannerTitle = logonBannerTitle;
        _logonBannerContent = logonBannerContent;
        _logonBannerEnableCheckbox = logonBannerEnableCheckbox;
        _externalIdpConfigs = externalIdpConfigs;
        _attributeDefinitions = attributeDefinitions;
        _entityId = entityId;
        _entityAlias = alias;
        _defaultProviders = defaultProviders;
        _authnPolicy = authnPolicy;
        _enableIdpSelection = idpSelectionFlag;
        _rsaConfigFilesLock = new ReentrantReadWriteLock();
        _preRsaAgentConfig = null;
    }

    public Tenant getTenant()
    {
        return _tenant;
    }

    public int getDelegationCount()
    {
        return _delegationCount;
    }

    public int getRenewCount()
    {
        return _renewCount;
    }

    public long getClockTolerance()
    {
        return _clockTolerance;
    }

    public long getMaxHOKLifetime()
    {
        return _maxHOKLifetime;
    }

    public long getMaxBearerTokenLifetime()
    {
        return _maxBearerTokenLifetime;
    }

    public long getMaxBearerRefreshTokenLifetime()
    {
        return _maxBearerRefreshTokenLifetime;
    }

    public long getMaxHoKRefreshTokenLifetime()
    {
        return _maxHoKRefreshTokenLifetime;
    }

    public List<Certificate> getTenantCertificate()
    {
        return _certificate;
    }

    public Collection<List<Certificate>> getTenantCertificates()
    {
        return _certificateChains;
    }

    public PrivateKey getPrivateKey()
    {
        return _key;
    }

    public String getSignatureAlgorithm()
    {
        return _signatureAlgorithm;
    }

    public Collection<IIdentityProvider> getProviders()
    {
        return _providers;
    }

    public Collection<IIdentityStoreData> getIdsStores()
    {
        return _idsStores;
    }

    private boolean matchUpnSuffixes(IIdentityProvider provider, String domainName) throws Exception
    {
        boolean result = false;
        Set<String> upnSuffixes = provider.getRegisteredUpnSuffixes();
        if (upnSuffixes != null)
        {
            result = upnSuffixes.contains(ValidateUtil.getCanonicalUpnSuffix(domainName));
        }

        return result;
    }

    public IIdentityProvider findProviderADAsFallBack(String domainName) throws Exception
    {
        if (_providers != null && !_providers.isEmpty())
        {
            for (IIdentityProvider provider : _providers)
            {
                if (provider.getDomain().equalsIgnoreCase(domainName) ||
                        (provider.getAlias() != null && provider.getAlias().equalsIgnoreCase(domainName))
                        || matchUpnSuffixes( provider, domainName )
                        )
                {
                    return provider;
                }
            }
        }

        return this.getAdProvider();
    }

    public
    IIdentityProvider findProviderByName(String name) throws Exception
    {
        for (IIdentityProvider provider : _providers)
        {
            if (provider.getName().equalsIgnoreCase(name))
            {
                return provider;
            }
        }

        return null;
    }

    public ISystemDomainIdentityProvider findSystemProvider() throws Exception
    {
        // There can be only one system provider
        if (_providers != null && !_providers.isEmpty())
        {
            for (IIdentityProvider provider : _providers)
            {
                if (provider instanceof ISystemDomainIdentityProvider)
                {
                    return (ISystemDomainIdentityProvider)provider;
                }
            }
        }

        return null;
    }

    public IIdentityProvider getAdProvider()
    {
        return _adProvider;
    }

    public Collection<IDPConfig> getExternalIdpConfigs()
    {
        return _externalIdpConfigs;
    }

    public String getBrandName()
    {
        return _brandName;
    }

    public String getLogonBannerTitle()
    {
        return _logonBannerTitle;
    }

    public String getLogonBannerContent()
    {
        return _logonBannerContent;
    }

    public boolean getLogonBannerCheckboxFlag()
    {
        return this._logonBannerEnableCheckbox;
    }

    public Collection<Attribute> getAttributeDefinitions()
    {
        return _attributeDefinitions;
    }

    public String getEntityId()
    {
        return _entityId;
    }

    public String getEntityAlias()
    {
        return _entityAlias;
    }

    public Collection<String> getDefaultProviders()
    {
        return _defaultProviders;
    }

    public AuthnPolicy getAuthnPolicy()
    {
        return _authnPolicy;
    }

    public boolean isIDPSelectionEnabled()
    {
        return _enableIdpSelection;
    }

    public ReadWriteLock get_rsaConfigFilesLock() {
        return _rsaConfigFilesLock;
    }

    public RSAAgentConfig get_preRsaAgentConfig() {
        return _preRsaAgentConfig;
    }

    public void set_preRsaAgentConfig(RSAAgentConfig _preRsaAgentConfig) {
        this._preRsaAgentConfig = _preRsaAgentConfig;
    }
}
