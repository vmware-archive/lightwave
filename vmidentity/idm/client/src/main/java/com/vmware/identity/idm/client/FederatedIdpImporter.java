/*
 *  Copyright (c) 2012-2017 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.idm.client;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.*;
import net.minidev.json.JSONObject;
import net.minidev.json.parser.JSONParser;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.net.URI;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class FederatedIdpImporter {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederatedIdpImporter.class);

    private final CasIdmClient idmClient;

    public FederatedIdpImporter(CasIdmClient client) {
        ValidateUtil.validateNotNull(client, "IDM Client");
        this.idmClient = client;
    }

    /**
     * Import Federated IDP configuration
     *
     * @param tenantName
     * @param config
     * @return entityID of the imported configuration
     * @throws Exception
     * @throws IDMException
     */
    public String
    importConfig(
            String tenantName,
            String config
    ) throws Exception {
        ValidateUtil.validateNotEmpty(tenantName, "Tenant Name");
        ValidateUtil.validateNotEmpty(config, "Configuration");
        JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
        JSONObject jsonObject = (JSONObject) jsonParser.parse(config);

        String protocol = (String) jsonObject.get(FederatedIdpConfigNames.PROTOCOL);
        IDPConfig.validateProtocol(protocol);

        if (protocol.equals(IDPConfig.IDP_PROTOCOL_OAUTH_2_0)) {
            return importFederatedOidcConfig(tenantName, jsonObject);
        } else {
            throw new IDMException(
                            String.format(
                                    "Error: Unsupported protocol [%s] for federated IDP",
                                    protocol
                            )
            );
        }
    }

    private String
    importFederatedOidcConfig(String tenantName, JSONObject config) throws Exception {
        String entityId = (String) config.get(OidcNames.ISSUER);
        if (entityId == null || entityId.isEmpty()) {
            throw new IDMException("Error: No issuer specified for federated IDP");
        }

        OidcConfig oidcConfig = new OidcConfig()
                                        .setIssuerType((String)config.get(FederatedIdpConfigNames.ISSUER_TYPE))
                                        .setClientId((String)config.get(OidcNames.CLIENT_ID))
                                        .setClientSecret((String)config.get(OidcNames.CLIENT_SECRET))
                                        .setAuthorizeRedirectURI((String)config.get(OidcNames.AUTHORIZATION_ENDPOINT))
                                        .setTokenRedirectURI((String)config.get(OidcNames.TOKEN_ENDPOINT))
                                        .setMetadataURI((String)config.get(OidcNames.METADATA_ENDPOINT))
                                        .setJwksURI((String)config.get(OidcNames.JWKS_ENDPOINT))
                                        .setLogoutURI((String)config.get(OidcNames.LOGOUT_ENDPOINT))
                                        .setPostLogoutURI((String)config.get(OidcNames.POST_LOGOUT_ENDPOINT))
                                        .setRedirectURI((String)config.get(OidcNames.REDIRECT_URL));

        IDPConfig idpConfig = new IDPConfig(entityId, IDPConfig.IDP_PROTOCOL_OAUTH_2_0);
        idpConfig.setOidcConfig(oidcConfig);

        idmClient.setExternalIdpConfig(tenantName, idpConfig);

        return entityId;
    }
}
