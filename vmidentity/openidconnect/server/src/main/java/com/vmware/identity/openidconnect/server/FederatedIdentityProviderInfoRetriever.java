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

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;

public class FederatedIdentityProviderInfoRetriever {

    private final CasIdmClient idmClient;

    public FederatedIdentityProviderInfoRetriever(CasIdmClient idmClient) {
        Validate.notNull(idmClient, "idm client");
        this.idmClient = idmClient;
    }

    public FederatedIdentityProviderInfo retrieveInfo(String tenant, String issuer) throws Exception {
        Validate.notEmpty(tenant, "tenant name");
        IDPConfig idpConfig = idmClient.getExternalIdpConfigForTenant(tenant, issuer);
        if (idpConfig == null) {
            throw new ServerException(ErrorObject.serverError(
                    String.format("failed to retrieve idp config for issuer [%s] in tenant [%s]", issuer, tenant)));
        }
        String logoutUri = idpConfig.getOidcConfig().getLogoutURI();
        Validate.notEmpty(logoutUri, "external idp logout uri");
        String issuerType = idpConfig.getOidcConfig().getIssuerType();
        String jwkUri = idpConfig.getOidcConfig().getJwksURI();
        Validate.notEmpty(jwkUri, "external idp jwk uri");
        return new FederatedIdentityProviderInfo.Builder(tenant, issuer, logoutUri)
                .issuerType(issuerType)
                .jwkUri(jwkUri)
                .roleGroupMappings(idpConfig.getTokenClaimGroupMappings())
                .build();
    }

    public FederatedIdentityProviderInfo retrieveInfo(String issuer) throws Exception {
        Validate.notEmpty(issuer, "idp issuer");
        String systemTenant = idmClient.getSystemTenant();
        return retrieveInfo(systemTenant, issuer);
    }
}
