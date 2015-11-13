/*
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
 */

package com.vmware.identity.openidconnect.server;

import java.security.cert.Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.id.Issuer;
import com.vmware.identity.idm.NoSuchTenantException;

/**
 * @author Yehia Zayour
 */
public class TenantInfoRetriever {
    private final IdmClient idmClient;

    public TenantInfoRetriever(IdmClient idmClient) {
        Validate.notNull(idmClient);
        this.idmClient = idmClient;
    }

    public TenantInformation retrieveTenantInfo(String tenant) throws ServerException {
        Validate.notEmpty(tenant, "tenant");

        try {
            this.idmClient.getTenant(tenant);
        } catch (NoSuchTenantException e) {
            throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription("non-existent tenant"), e);
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving tenant"), e);
        }

        RSAPrivateKey privateKey;
        try {
            privateKey = (RSAPrivateKey) this.idmClient.getTenantPrivateKey(tenant);
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving tenant private key"), e);
        }
        if (privateKey == null) {
            throw new IllegalStateException("tenant has no private key!");
        }

        RSAPublicKey publicKey = null;
        List<Certificate> certChain;
        try {
            certChain = this.idmClient.getTenantCertificate(tenant);
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving tenant cert"), e);
        }
        if (certChain != null && !certChain.isEmpty() && certChain.get(0) != null) {
            publicKey = (RSAPublicKey) certChain.get(0).getPublicKey();
        }
        if (publicKey == null) {
            throw new IllegalStateException("tenant has no public key!");
        }

        String issuer;
        String brandName;
        String logonBannerTitle;
        String logonBannerContent;
        boolean logonBannerEnableCheckbox;
        long idTokenBearerLifetimeMs;
        long idTokenHokLifetimeMs;
        long accessTokenBearerLifetimeMs;
        long accessTokenHokLifetimeMs;
        long refreshTokenBearerLifetimeMs;
        long refreshTokenHokLifetimeMs;
        long clockToleranceMs;
        try {
            issuer                       = this.idmClient.getOIDCEntityID(tenant);
            brandName                    = this.idmClient.getBrandName(tenant);
            logonBannerTitle             = this.idmClient.getLogonBannerTitle(tenant);
            logonBannerContent           = this.idmClient.getLogonBannerContent(tenant);
            logonBannerEnableCheckbox    = this.idmClient.getLogonBannerCheckboxFlag(tenant);
            idTokenBearerLifetimeMs      = this.idmClient.getMaximumBearerTokenLifetime(tenant);
            idTokenHokLifetimeMs         = this.idmClient.getMaximumHoKTokenLifetime(tenant);
            accessTokenBearerLifetimeMs  = this.idmClient.getMaximumBearerTokenLifetime(tenant);
            accessTokenHokLifetimeMs     = this.idmClient.getMaximumHoKTokenLifetime(tenant);
            refreshTokenBearerLifetimeMs = this.idmClient.getMaximumBearerRefreshTokenLifetime(tenant);
            refreshTokenHokLifetimeMs    = this.idmClient.getMaximumHoKRefreshTokenLifetime(tenant);
            clockToleranceMs             = this.idmClient.getClockTolerance(tenant);
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving tenant properties"), e);
        }

        return new TenantInformation.Builder(tenant).
                privateKey(privateKey).
                publicKey(publicKey).
                issuer(new Issuer(issuer)).
                brandName(brandName).
                logonBannerTitle(logonBannerTitle).
                logonBannerContent(logonBannerContent).
                logonBannerEnableCheckbox(logonBannerEnableCheckbox).
                idTokenBearerLifetimeMs(idTokenBearerLifetimeMs).
                idTokenHokLifetimeMs(idTokenHokLifetimeMs).
                accessTokenBearerLifetimeMs(accessTokenBearerLifetimeMs).
                accessTokenHokLifetimeMs(accessTokenHokLifetimeMs).
                refreshTokenBearerLifetimeMs(refreshTokenBearerLifetimeMs).
                refreshTokenHokLifetimeMs(refreshTokenHokLifetimeMs).
                clockToleranceMs(clockToleranceMs).build();
    }

    public String getDefaultTenantName() throws ServerException {
        try {
            return this.idmClient.getDefaultTenant();
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving default tenant name"), e);
        }
    }
}
