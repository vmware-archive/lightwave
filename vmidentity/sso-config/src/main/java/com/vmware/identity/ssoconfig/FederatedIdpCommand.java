package com.vmware.identity.ssoconfig;

import java.io.File;
import java.nio.file.NoSuchFileException;
import java.util.List;
import java.util.Map.Entry;
import java.util.Scanner;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.FederatedIdpDTO;
import com.vmware.identity.rest.idm.data.FederatedOidcConfigDTO;
import com.vmware.identity.rest.idm.data.FederatedSamlConfigDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.TokenClaimGroupDTO;

/**
 * Federated IDP configuration Commands.
 *
 */
public class FederatedIdpCommand extends SSOConfigCommand {

	private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederatedIdpCommand.class);

    @Option(name = "-e", aliases = {"--entity-id"}, metaVar = "[Entity ID]", usage = "Alias of the federated IDP.")
    private String entityId;

    @Option(name = "--alias", metaVar = "[Alias]", usage = "Alias of the federated IDP.")
    private String alias;

    @Option(name = "--register", metaVar = "[Metadata file path]", usage = "Path to registration information.")
    private String metadataFilePath;

    @Option(name = "--enable-jit", metaVar ="[True | False]", usage = "Jit provisioning switch.")
    private String jit;

    @Option(name = "--delete", metaVar = "[Entity ID]", usage = "Delete the federated IDP with the entity ID.")
    private String entityIdToDelete;

    @Option(name = "--list", metaVar = "[all | Federated IDP entity ID]", usage = "List federated IDP configurations."
            + " Type [all] to list all or type in the specific federated IDP entity ID.")
    private String federatedIdpToList;

    @Override
    public String getShortDescription() {
        return String.format(
                "Commands for Federated IDP configurations. Use %s %s for details.",
                this.getCommandName(), HELP_CMD
        );
    }

    @Override
    protected void execute() throws Exception {
        if (StringUtils.isNotEmpty(metadataFilePath)) {
            registerIdpWithMetadata(metadataFilePath);
        } else if (StringUtils.isNotEmpty(entityIdToDelete)) {
            deleteIdp(entityIdToDelete);
        } else if (StringUtils.isNotEmpty(federatedIdpToList)) {
            list();
        } else {
            registerIdp(entityId);
        }
	}

    private void registerIdpWithMetadata(String metadataFilePath) throws Exception {
        File configFile = new File(metadataFilePath);
        if (!configFile.exists()) {
            throw new NoSuchFileException("Error: No such file " + metadataFilePath);
        }
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        Scanner scanner = new Scanner(configFile);
        String metadata = scanner.useDelimiter("\\Z").next();
        scanner.close();
        client.federatedIdp().register(tenant, metadata);
        logger.info("Successfully registered federated IDP for tenant " + tenant);
    }

    private void registerIdp(String entityId) throws Exception {
        if (StringUtils.isEmpty(entityId)) {
            throw new IllegalArgumentException("Federated IDP entity ID is not provided.");
        }
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        FederatedIdpDTO currentIdpConfig = client.federatedIdp().get(tenant, entityId.trim());

        com.vmware.identity.rest.idm.data.FederatedIdpDTO.Builder builder = FederatedIdpDTO.builder();
        builder.withEntityID(entityId);
        builder.withProtocol(currentIdpConfig.getProtocol());

        alias = SSOConfigurationUtils.checkString(alias, currentIdpConfig.getAlias());
        builder.withAlias(alias);

        builder.withSamlConfig(currentIdpConfig.getSamlConfig());
        builder.withOidcConfig(currentIdpConfig.getOidcConfig());
        builder.withRoleGroupMappings(currentIdpConfig.getRoleGroupMappings());

        boolean jitEnabled = SSOConfigurationUtils.checkBoolean(jit, currentIdpConfig.isJitEnabled());
        builder.withJitEnabled(jitEnabled);

        client.federatedIdp().register(tenant, builder.build());
        logger.info("Successfully registered federated IDP for tenant " + tenant);
    }

    private void deleteIdp(String entityId) throws Exception {
        boolean jitUsersDeleted = true;
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        client.federatedIdp().delete(tenant, entityId, jitUsersDeleted);
        logger.info(
                String.format(
                        "Successfully deleted federated IDP for tenant %s. JIT users %s",
                            tenant,
                            jitUsersDeleted ? "deleted" : "not deleted"
                )
        );
    }

    private void list() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        if (federatedIdpToList.equalsIgnoreCase("all")) {
            List<FederatedIdpDTO> idps = client.federatedIdp().getAll(tenant);
            if (idps == null || idps.isEmpty()) {
                System.out.println("No federated IDP has been configured for tenant: " + tenant);
                return;
            }
            for (FederatedIdpDTO idp : idps) {
                displayFederatedlIdp(idp);
            }
        } else {
            displayFederatedlIdp(client.federatedIdp().get(tenant, federatedIdpToList));
        }
    }

    private void displayFederatedlIdp(FederatedIdpDTO idp) {
        System.out.println("Configuration for entity ID: " + idp.getEntityID() + "\n");
        SSOConfigurationUtils.displayParamNameAndValue("Entity ID", idp.getEntityID());
        SSOConfigurationUtils.displayParamNameAndValue("Protocol", idp.getProtocol());
        SSOConfigurationUtils.displayParamNameAndValue("Alias", idp.getAlias());
        String protocol = idp.getProtocol();
        if (protocol.equals(IDPConfig.IDP_PROTOCOL_OAUTH_2_0)) {
            FederatedOidcConfigDTO oidcConfig = idp.getOidcConfig();
            if (oidcConfig != null) {
                SSOConfigurationUtils.displayParamNameAndValue("Issuer Type", oidcConfig.getIssuerType());
                SSOConfigurationUtils.displayParamNameAndValue("Client ID", oidcConfig.getClientId());
                SSOConfigurationUtils.displayParamNameAndValue("Client Secret", oidcConfig.getClientSecret());
                SSOConfigurationUtils.displayParamNameAndValue("Redirect URL", oidcConfig.getRedirectURL());
                SSOConfigurationUtils.displayParamNameAndValue("Authorize Endpoint", oidcConfig.getAuthorizeEndpoint());
                SSOConfigurationUtils.displayParamNameAndValue("Token Endpoint", oidcConfig.getTokenEndpoint());
                SSOConfigurationUtils.displayParamNameAndValue("Logout Endpoint", oidcConfig.getLogoutEndpoint());
                SSOConfigurationUtils.displayParamNameAndValue("Post Logout Endpoint", oidcConfig.getPostLogoutEndpoint());
                SSOConfigurationUtils.displayParamNameAndValue("Metadata Endpoint", oidcConfig.getMetadataEndpoint());
                SSOConfigurationUtils.displayParamNameAndValue("JWKS Endpoint", oidcConfig.getJwksEndpoint());
            }
        } else if (protocol.equals(IDPConfig.IDP_PROTOCOL_SAML_2_0)) {
            FederatedSamlConfigDTO samlConfig = idp.getSamlConfig();
            if (samlConfig.getSsoServices() != null) {
                for (ServiceEndpointDTO e : samlConfig.getSsoServices()) {
                    SSOConfigurationUtils.displayParamNameAndValue("SSO Service", e.getEndpoint());
                }
            }
            if (samlConfig.getSloServices() != null) {
                for (ServiceEndpointDTO e : samlConfig.getSloServices()) {
                    SSOConfigurationUtils.displayParamNameAndValue("SLO Service", e.getEndpoint());
                }
            }
            if (samlConfig.getNameIDFormats() != null) {
                for (String s : samlConfig.getNameIDFormats()) {
                    SSOConfigurationUtils.displayParamNameAndValue("NameID Format", s);
                }
            }
            if (samlConfig.getSubjectFormats() != null && !samlConfig.getSubjectFormats().isEmpty()) {
                System.out.println("\nSubject Format mappings: ");
                for (Entry<String, String> e : samlConfig.getSubjectFormats().entrySet()) {
                    SSOConfigurationUtils.displayParamNameAndValue(e.getKey(), e.getValue());
                }
            }
        }
        SSOConfigurationUtils.displayParamNameAndValue("Jit Enabled", idp.isJitEnabled());
        SSOConfigurationUtils.displayParamNameAndValue("UPN Suffix", idp.getUpnSuffix());
        if (idp.getRoleGroupMappings() != null && !idp.getRoleGroupMappings().isEmpty()) {
            System.out.println("\nToken claim/group mappings: ");
            for (TokenClaimGroupDTO e : idp.getRoleGroupMappings()) {
                SSOConfigurationUtils.displayParamNameAndValues(e.getClaimName() + "/" + e.getClaimValue(), e.getGroups());
            }
        }

        SSOConfigurationUtils.displaySeparationLine();
    }
}
