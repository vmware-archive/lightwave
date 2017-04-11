package com.vmware.identity.ssoconfig;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;
import java.util.Map.Entry;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.TokenClaimGroupDTO;

/**
 * External IDP configuration Commands.
 *
 */
public class ExternalIDPCommand extends SSOConfigCommand {

	private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(ExternalIDPCommand.class);

    @Option(name = "-e", aliases = {"--entity-id"}, metaVar = "[Entity ID]", usage = "Alias of the external IDP.")
    private String entityId;

    @Option(name = "--alias", metaVar = "[Alias]", usage = "Alias of the external IDP.")
    private String alias;

    @Option(name = "--register", metaVar = "[Metadata file path]", usage = "External IDP metadata file location to register.")
    private String metadataFile;

    @Option(name = "--enable-jit", metaVar ="[True | False]", usage = "Jit provisioning switch.")
    private String jit;

    @Option(name = "--delete", metaVar = "[Entity ID]", usage = "Delete the external IDP with the entity ID.")
    private String entityIdToDelete;

    @Option(name = "--delete-jit-users", metaVar = "[True | False]", usage = "Delete all jit users when deleting the external IDP.")
    private String deleteJitUsers;

    @Option(name = "--upn-suffix", metaVar = "[UPN suffix]", usage = "UPN suffix to be used locally for the external IDP.")
    private String upnSuffix;

    @Option(name = "--add-claim-group-mapping", metaVar = "[Entity ID]", usage = "Add claim/group mapping. Please provide the external IDP entity ID.")
    private String entityIdToAddMapping;

    @Option(name = "--delete-claim-group-mapping", metaVar = "[Entity ID]", usage = "Delete claim/group mapping. Please provide the external IDP entity ID.")
    private String entityIdToDeleteMapping;

    @Option(name = "--claim-name", metaVar = "[Token claim name]", usage = "External token claim name to be used for claim/group mapping.")
    private String claimName;

    @Option(name = "--claim-value", metaVar = "[Token claim value]", usage = "External token claim value to be used for claim/group mapping.")
    private String claimValue;

    @Option(name = "--groups", metaVar = "[Group names separated by comma]", usage = "Group name to be used for claim/group mapping.")
    private String groups;

    @Option(name = "--list", metaVar = "[all | External IDP entity ID]", usage = "List tenant external IDP configurations."
            + " Type [all] to list all or type in the specific external IDP entity ID.")
    private String ExtIdpToList;

    @Override
    public String getShortDescription() {
        return String.format("Commands for external IDP configurations. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (StringUtils.isNotEmpty(metadataFile)) {
            registerIdpWithMetadata();
        } else if (StringUtils.isNotEmpty(entityIdToDelete)) {
            removeIdp();
        } else if (StringUtils.isNotEmpty(ExtIdpToList)) {
            list();
        } else if (StringUtils.isNotEmpty(entityIdToAddMapping) || StringUtils.isNotEmpty(entityIdToDeleteMapping)){
            registerIdp(entityIdToAddMapping != null ? entityIdToAddMapping : entityIdToDeleteMapping);
        } else {
        	registerIdp(entityId);
        }
	}

    private void registerIdpWithMetadata() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        Scanner scanner = new Scanner(new File(metadataFile));
        String metadata = scanner.useDelimiter("\\Z").next();
        scanner.close();
        client.externalIdp().register(tenant, metadata);
        logger.info("Successfully registered external IDP for tenant " + tenant);
    }

    private void registerIdp(String entityId) throws Exception {
        if (StringUtils.isEmpty(entityId)) {
            throw new IllegalArgumentException("External IDP entity ID is not provided.");
        }
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        ExternalIDPDTO currentIdpConfig = client.externalIdp().get(tenant, entityId);

        com.vmware.identity.rest.idm.data.ExternalIDPDTO.Builder builder = ExternalIDPDTO.builder();
        builder.withEntityID(entityId);

        alias = SSOConfigurationUtils.checkString(alias, currentIdpConfig.getAlias());
        builder.withAlias(alias);

        upnSuffix = SSOConfigurationUtils.checkString(upnSuffix, currentIdpConfig.getUpnSuffix());
        builder.withUpnSuffix(upnSuffix);

        boolean jitEnabled = SSOConfigurationUtils.checkBoolean(jit, currentIdpConfig.isJitEnabled());
        builder.withJitEnabled(jitEnabled);

        builder.withTokenClaimGroups(getTokenClaimGroupDTOList(currentIdpConfig.getTokenClaimGroups()));

        client.externalIdp().register(entityId, builder.build());
        logger.info("Successfully registered external IDP for tenant " + tenant);
    }

    private List<TokenClaimGroupDTO> getTokenClaimGroupDTOList(List<TokenClaimGroupDTO> existingList) {
        if (StringUtils.isEmpty(entityIdToAddMapping) && StringUtils.isEmpty(entityIdToDeleteMapping)) {
            return existingList;
        } else if (StringUtils.isEmpty(claimName) || StringUtils.isEmpty(claimValue) || StringUtils.isEmpty(groups)) {
            throw new IllegalArgumentException("Token claim name and claim value and group name are required to set the claim/group mapping.");
        } else {
            if (existingList == null) {
                existingList = new ArrayList<>();
            }
            List<String> groupList = Arrays.asList(groups.split(","));
            if (StringUtils.isNotEmpty(entityIdToAddMapping)) {
                TokenClaimGroupDTO newClaimGroupMapping = new TokenClaimGroupDTO.Builder()
                .withClaimName(claimName)
                .withClaimValue(claimValue)
                .withGroups(groupList).build();
                existingList.add(newClaimGroupMapping);
                logger.info("Added claim group mapping for entity id " + entityIdToAddMapping);
            }
            if (StringUtils.isNotEmpty(entityIdToDeleteMapping)) {
                List<TokenClaimGroupDTO> newList = new ArrayList<>();
                for (TokenClaimGroupDTO mapping : existingList) {
                    // find the mapping and update the groups mapped to the claim name and value
                    if (mapping.getClaimName().equalsIgnoreCase(claimName) && mapping.getClaimValue().equalsIgnoreCase(claimValue)) {
                        List<String> newGroups = new ArrayList<>();
                        for (String group : mapping.getGroups()) {
                            if (!groupList.contains(group)) {
                                newGroups.add(group);
                            }
                        }
                        if (!newGroups.isEmpty()) {
                            TokenClaimGroupDTO newMapping = new TokenClaimGroupDTO.Builder()
                                    .withClaimName(claimName)
                                    .withClaimValue(claimValue)
                                    .withGroups(newGroups)
                                    .build();
                            newList.add(newMapping);
                        }
                    } else {
                        newList.add(mapping);
                    }
                }
                logger.info("Deleted claim group mapping for entity id " + entityIdToDeleteMapping);
            }
            return existingList;
        }
    }

    private void removeIdp() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        boolean jitUsersDeleted =SSOConfigurationUtils.checkBoolean(deleteJitUsers, false);
        client.externalIdp().delete(tenant, entityIdToDelete, jitUsersDeleted);
        logger.info("Successfully deleted external IDP for tenant " + tenant + ", delete JIT users " + jitUsersDeleted);
    }

    private void list() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        if (ExtIdpToList.equalsIgnoreCase("all")) {
            List<ExternalIDPDTO> extIdps = client.externalIdp().getAll(tenant);
            if (extIdps == null || extIdps.isEmpty()) {
                System.out.println("No external IDP has been configured for tenant: " + tenant);
                return;
            }
            for (ExternalIDPDTO idp : extIdps) {
                displayExternalIdp(idp);
            }
        } else {
            displayExternalIdp(client.externalIdp().get(tenant, ExtIdpToList));
        }
    }

    private void displayExternalIdp(ExternalIDPDTO extIdp) {
        System.out.println("Configuration for entity ID: " + extIdp.getEntityID() + "\n");
        SSOConfigurationUtils.displayParamNameAndValue("Entity ID", extIdp.getEntityID());
        SSOConfigurationUtils.displayParamNameAndValue("Alias", extIdp.getAlias());
        SSOConfigurationUtils.displayParamNameAndValue("Jit Enabled", extIdp.isJitEnabled());
        SSOConfigurationUtils.displayParamNameAndValue("UPN Suffix", extIdp.getUpnSuffix());
        if (extIdp.getSsoServices() != null) {
            for (ServiceEndpointDTO e : extIdp.getSsoServices()) {
                SSOConfigurationUtils.displayParamNameAndValue("SSO Service", e.getEndpoint());
            }
        }
        if (extIdp.getSloServices() != null) {
            for (ServiceEndpointDTO e : extIdp.getSloServices()) {
                SSOConfigurationUtils.displayParamNameAndValue("SLO Service", e.getEndpoint());
            }
        }
        if (extIdp.getNameIDFormats() != null) {
            for (String s : extIdp.getNameIDFormats()) {
                SSOConfigurationUtils.displayParamNameAndValue("NameID Format", s);
            }
        }
        if (extIdp.getSubjectFormats() != null && !extIdp.getSubjectFormats().isEmpty()) {
            System.out.println("\nSubject Format mappings: ");
            for (Entry<String, String> e : extIdp.getSubjectFormats().entrySet()) {
               SSOConfigurationUtils.displayParamNameAndValue(e.getKey(), e.getValue());
            }
        }
        if (extIdp.getTokenClaimGroups() != null && !extIdp.getTokenClaimGroups().isEmpty()) {
            System.out.println("\nToken claim/group mappings: ");
            for (TokenClaimGroupDTO e : extIdp.getTokenClaimGroups()) {
               SSOConfigurationUtils.displayParamNameAndValues(e.getClaimName() + "/" + e.getClaimValue(), e.getGroups());
            }
        }
        SSOConfigurationUtils.displaySeparationLine();
    }
}
