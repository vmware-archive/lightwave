/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

public class FederatedIdentityProviderInfo {

    private final String tenant;
    private final String issuer;
    private final String logoutUri;
    private String jwkUri;
    private IssuerType issuerType;

    public enum IssuerType {
        CSP("csp");

        private String type;

        IssuerType(String type) {
            this.type = type;
        }

        public String getType() {
            return this.type;
        }
    }

    private FederatedIdentityProviderInfo(Builder builder) {
        this.tenant = builder.tenant;
        this.issuer = builder.issuer;
        this.logoutUri = builder.logoutUri;
        this.issuerType = builder.issuerType;
        this.jwkUri = builder.jwkUri;
    }

    public String getTenant() {
        return tenant;
    }

    public String getIssuer() {
        return this.issuer;
    }

    public String getLogoutURI() {
        return this.logoutUri;
    }

    public String getJwkURI() {
        return this.jwkUri;
    }

    public IssuerType getIssuerType() {
        return this.issuerType;
    }

    public static class Builder {

        private final String tenant;
        private final String issuer;
        private final String logoutUri;
        private String jwkUri;
        private IssuerType issuerType;

        public Builder(String tenant, String issuer, String logoutUri) {
            Validate.notEmpty(tenant, "tenant name");
            Validate.notEmpty(issuer, "idp issuer");
            Validate.notEmpty(logoutUri, "idp logout uri");
            this.tenant = tenant;
            this.issuer = issuer;
            this.logoutUri = logoutUri;
        }

        public Builder jwkUri(String jwkUri) {
            Validate.notEmpty(jwkUri, "idp jwk uri");
            this.jwkUri = jwkUri;
            return this;
        }

        public Builder issuerType(String issuerType) {
            if (StringUtils.isNotEmpty(issuerType)) {
                this.issuerType = IssuerType.valueOf(issuerType.toUpperCase());
            }
            return this;
        }

        public FederatedIdentityProviderInfo build() {
            if (this.issuerType == null) {
                this.issuerType = IssuerType.CSP;
            }

            return new FederatedIdentityProviderInfo(this);
        }
    }
}
