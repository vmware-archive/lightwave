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

import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.Issuer;

/**
 * @author Yehia Zayour
 */
public class TenantInfo {
    private final String name;
    private final RSAPrivateKey privateKey;
    private final RSAPublicKey publicKey;
    private final X509Certificate certificate;
    private final AuthnPolicy authnPolicy;
    private final Issuer issuer;
    private final String brandName;
    private final String logonBannerTitle;
    private final String logonBannerContent;
    private final boolean logonBannerEnableCheckbox;
    private final long idTokenBearerLifetimeMs;
    private final long idTokenHokLifetimeMs;
    private final long accessTokenBearerLifetimeMs;
    private final long accessTokenHokLifetimeMs;
    private final long refreshTokenBearerLifetimeMs;
    private final long refreshTokenHokLifetimeMs;
    private final long clockToleranceMs;

    private TenantInfo(Builder builder) {
        Validate.notNull(builder, "builder");
        Validate.notEmpty(builder.name, "builder.name");
        Validate.notNull(builder.privateKey, "builder.privateKey");
        Validate.notNull(builder.publicKey, "builder.publicKey");
        Validate.notNull(builder.certificate, "builder.certificate");
        Validate.notNull(builder.authnPolicy, "builder.authnPolicy");
        Validate.notNull(builder.issuer, "builder.issuer");

        this.name                           = builder.name;
        this.privateKey                     = builder.privateKey;
        this.publicKey                      = builder.publicKey;
        this.certificate                    = builder.certificate;
        this.authnPolicy                    = builder.authnPolicy;
        this.issuer                         = builder.issuer;
        this.brandName                      = builder.brandName;
        this.logonBannerTitle               = builder.logonBannerTitle;
        this.logonBannerContent             = builder.logonBannerContent;
        this.logonBannerEnableCheckbox      = builder.logonBannerEnableCheckbox;
        this.idTokenBearerLifetimeMs        = builder.idTokenBearerLifetimeMs;
        this.idTokenHokLifetimeMs           = builder.idTokenHokLifetimeMs;
        this.accessTokenBearerLifetimeMs    = builder.accessTokenBearerLifetimeMs;
        this.accessTokenHokLifetimeMs       = builder.accessTokenHokLifetimeMs;
        this.refreshTokenBearerLifetimeMs   = builder.refreshTokenBearerLifetimeMs;
        this.refreshTokenHokLifetimeMs      = builder.refreshTokenHokLifetimeMs;
        this.clockToleranceMs               = builder.clockToleranceMs;
    }

    public String getName() {
        return this.name;
    }

    public RSAPrivateKey getPrivateKey() {
        return this.privateKey;
    }

    public RSAPublicKey getPublicKey() {
        return this.publicKey;
    }

    public X509Certificate getCertificate() {
        return this.certificate;
    }

    public AuthnPolicy getAuthnPolicy() {
        return this.authnPolicy;
    }

    public Issuer getIssuer() {
        return this.issuer;
    }

    public String getBrandName() {
        return this.brandName;
    }

    public String getLogonBannerTitle() {
        return this.logonBannerTitle;
    }

    public String getLogonBannerContent() {
        return this.logonBannerContent;
    }

    public boolean getLogonBannerEnableCheckbox() {
        return this.logonBannerEnableCheckbox;
    }

    public long getIDTokenBearerLifetimeMs() {
        return this.idTokenBearerLifetimeMs;
    }

    public long getIDTokenHokLifetimeMs() {
        return this.idTokenHokLifetimeMs;
    }

    public long getAccessTokenBearerLifetimeMs() {
        return this.accessTokenBearerLifetimeMs;
    }

    public long getAccessTokenHokLifetimeMs() {
        return this.accessTokenHokLifetimeMs;
    }

    public long getRefreshTokenBearerLifetimeMs() {
        return this.refreshTokenBearerLifetimeMs;
    }

    public long getRefreshTokenHokLifetimeMs() {
        return this.refreshTokenHokLifetimeMs;
    }

    public long getClockToleranceMs() {
        return this.clockToleranceMs;
    }

    public static class AuthnPolicy {
        private final boolean isPasswordAuthnEnabled;
        private final boolean isClientCertAuthnEnabled;
        private final boolean isWindowsAuthnEnabled;
        private final boolean isSecurIdAuthnEnabled;
        private final String securIdLoginGuide;

        public AuthnPolicy(
                boolean isPasswordAuthnEnabled,
                boolean isClientCertAuthnEnabled,
                boolean isWindowsAuthnEnabled,
                boolean isSecurIdAuthnEnabled,
                String securIdLoginGuide) {
            this.isPasswordAuthnEnabled = isPasswordAuthnEnabled;
            this.isClientCertAuthnEnabled = isClientCertAuthnEnabled;
            this.isWindowsAuthnEnabled = isWindowsAuthnEnabled;
            this.isSecurIdAuthnEnabled = isSecurIdAuthnEnabled;
            this.securIdLoginGuide = securIdLoginGuide;
        }

        public boolean isPasswordAuthnEnabled() {
            return this.isPasswordAuthnEnabled;
        }

        public boolean isClientCertAuthnEnabled() {
            return this.isClientCertAuthnEnabled;
        }

        public boolean isWindowsAuthnEnabled() {
            return this.isWindowsAuthnEnabled;
        }

        public boolean isSecurIDAuthnEnabled() {
            return this.isSecurIdAuthnEnabled;
        }

        public String getSecurIDLoginGuide() {
            return this.securIdLoginGuide;
        }
    }

    public static class Builder {
        private String name;
        private RSAPrivateKey privateKey;
        private RSAPublicKey publicKey;
        private X509Certificate certificate;
        private AuthnPolicy authnPolicy;
        private Issuer issuer;
        private String brandName;
        private String logonBannerTitle;
        private String logonBannerContent;
        private boolean logonBannerEnableCheckbox;
        private long idTokenBearerLifetimeMs;
        private long idTokenHokLifetimeMs;
        private long accessTokenBearerLifetimeMs;
        private long accessTokenHokLifetimeMs;
        private long refreshTokenBearerLifetimeMs;
        private long refreshTokenHokLifetimeMs;
        private long clockToleranceMs;

        public Builder(String name) {
            Validate.notEmpty(name, "name");
            this.name = name;
        }

        public Builder privateKey(RSAPrivateKey privateKey) {
            Validate.notNull(privateKey, "privateKey");
            this.privateKey = privateKey;
            return this;
        }

        public Builder publicKey(RSAPublicKey publicKey) {
            Validate.notNull(publicKey, "publicKey");
            this.publicKey = publicKey;
            return this;
        }

        public Builder certificate(X509Certificate certificate) {
            Validate.notNull(certificate, "certificate");
            this.certificate = certificate;
            return this;
        }

        public Builder authnPolicy(AuthnPolicy authnPolicy) {
            Validate.notNull(authnPolicy, "authnPolicy");
            this.authnPolicy = authnPolicy;
            return this;
        }

        public Builder issuer(Issuer issuer) {
            Validate.notNull(issuer, "issuer");
            this.issuer = issuer;
            return this;
        }

        public Builder brandName(String brandName) {
            this.brandName = brandName;
            return this;
        }

        public Builder logonBannerTitle(String logonBannerTitle) {
            this.logonBannerTitle = logonBannerTitle;
            return this;
        }

        public Builder logonBannerContent(String logonBannerContent) {
            this.logonBannerContent = logonBannerContent;
            return this;
        }

        public Builder logonBannerEnableCheckbox(boolean logonBannerEnableCheckbox) {
            this.logonBannerEnableCheckbox = logonBannerEnableCheckbox;
            return this;
        }

        public Builder idTokenBearerLifetimeMs(long idTokenBearerLifetimeMs) {
            this.idTokenBearerLifetimeMs = idTokenBearerLifetimeMs;
            return this;
        }

        public Builder idTokenHokLifetimeMs(long idTokenHokLifetimeMs) {
            this.idTokenHokLifetimeMs = idTokenHokLifetimeMs;
            return this;
        }

        public Builder accessTokenBearerLifetimeMs(long accessTokenBearerLifetimeMs) {
            this.accessTokenBearerLifetimeMs = accessTokenBearerLifetimeMs;
            return this;
        }

        public Builder accessTokenHokLifetimeMs(long accessTokenHokLifetimeMs) {
            this.accessTokenHokLifetimeMs = accessTokenHokLifetimeMs;
            return this;
        }

        public Builder refreshTokenBearerLifetimeMs(long refreshTokenBearerLifetimeMs) {
            this.refreshTokenBearerLifetimeMs = refreshTokenBearerLifetimeMs;
            return this;
        }

        public Builder refreshTokenHokLifetimeMs(long refreshTokenHokLifetimeMs) {
            this.refreshTokenHokLifetimeMs = refreshTokenHokLifetimeMs;
            return this;
        }

        public Builder clockToleranceMs(long clockToleranceMs) {
            this.clockToleranceMs = clockToleranceMs;
            return this;
        }

        public TenantInfo build() {
            return new TenantInfo(this);
        }
    }
}