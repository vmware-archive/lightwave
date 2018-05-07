/*
 *  Copyright (c) 2017 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.ssoconfig;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.protocol.JSONUtils;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.*;
import net.minidev.json.JSONObject;
import net.minidev.json.parser.JSONParser;
import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;

/**
 * OIDC Client configuration commands.
 *
 */
public class OidcClientCommand extends SSOConfigCommand {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(OidcClientCommand.class);

    private static final String REDIRECT_URIS_KEY = "redirect_uris";
    private static final String LOGOUT_URI_KEY = "logout_uri";
    private static final String POST_LOGOUT_URIS_KEY = "post_logout_uris";
    private static final String TOKEN_ENDPOINT_AUTH_METHOD_KEY = "token_endpoint_auth_method";
    private static final String CLIENT_ASSERTION_LIFETIME_MS_KEY = "client_assertion_lifetime";
    private static final String CERTIFICATE_SUBJECT_DN_KEY = "certificate_subject_dn";

    @Option(name = "--delete", metaVar = "[OIDC Client Id]", usage = "Delete the OIDC Client with the id.")
    private String idToDelete = null;

    @Option(name = "--register", metaVar = "[Metadata File path]", usage = "OIDC Client metadata file location to register.")
    private String metadataFile = null;

    @Option(name = "--update", metaVar = "[OIDC Client Id]", usage = "OIDC Client Id to update.")
    private String clientId = null;

    @Option(name = "--redirect-uri", metaVar = "[Redirect URI]", usage = "Add an oidc client redirect uri.")
    private String redirectUri;

    @Option(name = "--redirect-uri-template", metaVar = "[Redirect URI Template]", usage = "Add an oidc client redirect uri template.")
    private String redirectUriTemplate;

    @Option(name = "-m", aliases = {"--metadata-file-path"}, metaVar = "[Metadata File path]", usage = "OIDC Client metadata file location.")
    private String metadataFileToUpdate;

    @Option(name = "--list", metaVar = "[all | OIDC Client Id]", usage = "List tenant's OIDC Client configurations."
            + " Type [all] to list all or type in the specific OIDC Client name.")
    private String clientIdToList = null;

    @Override
    public String getShortDescription() {
        return String.format("Commands for OIDC client configurations. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (StringUtils.isNotEmpty(metadataFile)) {
            registerOIDCClient();
        } else if(StringUtils.isNotEmpty(clientId)){
            updateOIDCClient();
        } else if (StringUtils.isNotEmpty(idToDelete)) {
            deleteOIDCClient();
        } else if (StringUtils.isNotEmpty(clientIdToList)) {
            list();
        }
	}

    private void registerOIDCClient() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        Scanner scanner = new Scanner(new File(metadataFile));
        try {
            OIDCClientDTO clientDTO = client.oidcClient().register(
                                            tenant,
                                            getOIDCClientMetadataDTO(
                                                    scanner.useDelimiter("\\Z").next()
                                            )
            );
            logger.info(String.format(
                                "Successfully registered OIDC Client [%s] for tenant %s",
                                clientDTO.getClientId(),
                                tenant)
            );
        } finally {
            scanner.close();
        }
    }

    private void updateOIDCClient() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        OIDCClientDTO clientDTO = null;
        if (StringUtils.isNotEmpty(redirectUri) || StringUtils.isNotEmpty(redirectUriTemplate)) {
            OIDCClientMetadataDTO existingMetadata = client.oidcClient().get(tenant, clientId).getOIDCClientMetadataDTO();
            List<String> redirectUris = existingMetadata.getRedirectUris();
            List<String> redirectUriTemplates = existingMetadata.getRedirectUriTemplates();
            if (StringUtils.isNotEmpty(redirectUri)) {
                redirectUris.add(redirectUri);
            }
            if (StringUtils.isNotEmpty(redirectUriTemplate)) {
                redirectUriTemplates.add(redirectUriTemplate);
            }
            OIDCClientMetadataDTO newMetadata = new OIDCClientMetadataDTO.Builder().
            withRedirectUris(redirectUris).
            withRedirectUriTemplates(redirectUriTemplates).
            withTokenEndpointAuthMethod(existingMetadata.getTokenEndpointAuthMethod()).
            withPostLogoutRedirectUris(existingMetadata.getPostLogoutRedirectUris()).
            withLogoutUri(existingMetadata.getLogoutUri()).
            withCertSubjectDN(existingMetadata.getCertSubjectDN()).
            withAuthnRequestClientAssertionLifetimeMS(existingMetadata.getAuthnRequestClientAssertionLifetimeMS()).
            withMultiTenant(existingMetadata.isMultiTenant()).
            withLogoutUriTemplate(existingMetadata.getLogoutUriTemplate()).
            withPostLogoutRedirectUriTemplates(existingMetadata.getPostLogoutRedirectUriTemplates()).build();
            clientDTO = client.oidcClient().update(tenant, clientId, newMetadata);
        } else if (StringUtils.isNotEmpty(metadataFile)) {
            Scanner scanner = new Scanner(new File(metadataFile));
            try {
                clientDTO = client.oidcClient().update(tenant, clientId,
                                                getOIDCClientMetadataDTO(
                                                        scanner.useDelimiter("\\Z").next()));
            } finally {
                scanner.close();
            }
        }
        if (clientDTO != null) {
            logger.info(String.format(
                    "Successfully updated OIDC Client [%s] for tenant %s",
                    clientId,
                    tenant));
            displayOIDCClient(clientDTO);
        } else {
            logger.info(String.format(
                    "Error updating OIDC Client [%s] for tenant %s",
                    clientId,
                    tenant));
        }
    }

    private OIDCClientMetadataDTO getOIDCClientMetadataDTO(String metadata) {
        try {
            JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
            JSONObject jsonConfig = (JSONObject) jsonParser.parse(metadata);
            final List<String> redirectURIList = new ArrayList<String>();
            final List<String> postLogoutURIList = new ArrayList<String>();
            String tokenEndpointMethod = null;
            Long clientAssertionLifetimeMS = 0L;
            String logoutURI = null;
            String certDN = null;

            if (jsonConfig.get(TOKEN_ENDPOINT_AUTH_METHOD_KEY) != null) {
                tokenEndpointMethod = JSONUtils.getString(jsonConfig, TOKEN_ENDPOINT_AUTH_METHOD_KEY);
            }
            if (jsonConfig.get(CLIENT_ASSERTION_LIFETIME_MS_KEY) != null) {
                clientAssertionLifetimeMS = JSONUtils.getLong(jsonConfig, CLIENT_ASSERTION_LIFETIME_MS_KEY);
            }
            if (jsonConfig.get(REDIRECT_URIS_KEY) != null) {
                final String[] redirectURIs = JSONUtils.getStringArray(jsonConfig, REDIRECT_URIS_KEY);
                redirectURIList.addAll(Arrays.asList(redirectURIs));
            }
            if (jsonConfig.get(LOGOUT_URI_KEY) != null) {
                logoutURI = JSONUtils.getString(jsonConfig, LOGOUT_URI_KEY);
            }
            if (jsonConfig.get(POST_LOGOUT_URIS_KEY) != null) {
                final String[] postLogoutURIs = JSONUtils.getStringArray(jsonConfig, POST_LOGOUT_URIS_KEY);
                postLogoutURIList.addAll(Arrays.asList(postLogoutURIs));
            }
            if (jsonConfig.get(CERTIFICATE_SUBJECT_DN_KEY) != null) {
                certDN = JSONUtils.getString(jsonConfig, CERTIFICATE_SUBJECT_DN_KEY);
            }

            return OIDCClientMetadataDTO.builder()
                        .withRedirectUris(redirectURIList)
                        .withLogoutUri(logoutURI)
                        .withPostLogoutRedirectUris(postLogoutURIList)
                        .withTokenEndpointAuthMethod(tokenEndpointMethod)
                        .withAuthnRequestClientAssertionLifetimeMS(clientAssertionLifetimeMS)
                        .withCertSubjectDN(certDN)
                        .build();
        } catch (ParseException e) {
            throw new RuntimeException("Error: Failed to parse OIDC Client metadata.", e);
        } catch (net.minidev.json.parser.ParseException e) {
            throw new RuntimeException("Error: Failed to parse OIDC Client metadata.", e);
        }
    }

    private void deleteOIDCClient() throws Exception {
        final IdmClient client = SSOConfigurationUtils.getIdmClient();
        client.oidcClient().delete(tenant, idToDelete);
        logger.info("Successfully deleted OIDC Client for tenant " + tenant);
    }

    private void list() throws Exception {
        final IdmClient idmClient = SSOConfigurationUtils.getIdmClient();
        if (clientIdToList.equalsIgnoreCase("all")) {
            final List<OIDCClientDTO> clientList = idmClient.oidcClient().getAll(tenant);
            if (clientList == null || clientList.isEmpty()) {
                System.out.println("No OIDC Clients have been configured for tenant: " + tenant);
                return;
            }
            System.out.println("OIDC Client for tenant " + tenant);
            for (OIDCClientDTO client : clientList) {
                displayOIDCClient(client);
            }
        } else {
            final OIDCClientDTO client = idmClient.oidcClient().get(tenant, clientIdToList);
            displayOIDCClient(client);
        }
    }

    private void displayOIDCClient(OIDCClientDTO client) {
        final OIDCClientMetadataDTO metadata = client.getOIDCClientMetadataDTO();

        System.out.println("Configuration for OIDC Client: " + client.getClientId() + "\n");
        // Print Auth Method
        SSOConfigurationUtils.displayParamNameAndValue("Auth Method", metadata.getTokenEndpointAuthMethod());
        SSOConfigurationUtils.displayParamNameAndValue(
                "Assertion Lifetime (milli seconds)",
                metadata.getAuthnRequestClientAssertionLifetimeMS()
        );
        // Print the Redirect URIs
        final List<String> redirectURIs = metadata.getRedirectUris();
        if (redirectURIs != null) {
            for (String uri : redirectURIs) {
                SSOConfigurationUtils.displayParamNameAndValue("Redirect URI", uri);
            }
        }
        // Print the Logout URI
        final String logoutURI = metadata.getLogoutUri();
        if (logoutURI != null) {
            SSOConfigurationUtils.displayParamNameAndValue("Logout URI", logoutURI);
        }
        // Print the Post Logout URIs
        final List<String> postLogoutURIs = metadata.getPostLogoutRedirectUris();
        if (postLogoutURIs != null) {
            for (String uri : postLogoutURIs) {
                SSOConfigurationUtils.displayParamNameAndValue("Post Logout URI", uri);
            }
        }
        if (client.getOIDCClientMetadataDTO().isMultiTenant()) {
            // Print the Redirect URI templates
            final List<String> redirectURITemplates = metadata.getRedirectUriTemplates();
            if (redirectURITemplates != null) {
                for (String uri : redirectURITemplates) {
                    SSOConfigurationUtils.displayParamNameAndValue("Redirect URI Template", uri);
                }
            }
            // Print the Logout URI template
            final String logoutURITemplate = metadata.getLogoutUriTemplate();
            if (logoutURITemplate != null) {
                SSOConfigurationUtils.displayParamNameAndValue("Logout URI Template", logoutURITemplate);
            }
            // Print the Post Logout URI templates
            final List<String> postLogoutURITemplates = metadata.getPostLogoutRedirectUriTemplates();
            if (postLogoutURITemplates != null) {
                for (String uri : postLogoutURITemplates) {
                    SSOConfigurationUtils.displayParamNameAndValue("Post Logout URI Template", uri);
                }
            }
        }
        final String certSubjectDN = metadata.getCertSubjectDN();
        if (certSubjectDN != null) {
            SSOConfigurationUtils.displayParamNameAndValue("Certificate Subject DN", certSubjectDN);
        }
        SSOConfigurationUtils.displaySeparationLine();
    }
}
