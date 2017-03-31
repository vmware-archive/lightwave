package com.vmware.identity.ssoconfig;

import java.net.URI;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.SchemaObjectMappingDTO;

public class IdentityProviderCommand extends SSOConfigCommand {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdentityProviderCommand.class);

    private static final String LDAPS_SCHEMA = "ldaps";
    private static final String SYSTEM_DOMAIN = "SYSTEM_DOMAIN";

    @Option(name = "--list", metaVar = "[ALL | Identity Provider]", usage = "List tenant identity provider configurations."
            + " Type [ALL] to list all or type in the specific identity provider name.")
    private String providerToList;

    @Option(name = "--provider", metaVar = "[Provider name]", usage = "Identity provider to update configuratons.")
    private String name;

    @Option(name = "--alias", metaVar = "[Alias]", usage = "Set identity store alias.")
    private String alias;

    @Option(name = "--spn", metaVar = "[SPN]", usage = "Set Sertvice Principal Name.")
    private String spn;

    @Option(name = "--friendly-name", metaVar = "[Friendly name]", usage = "Set identity store friendly name.")
    private String friendlyName;

    @Option(name = "--use-machine-account", metaVar = "[True | False]", usage = "Set machine account flag.")
    private String useMachineAccount;

    @Option(name = "--matching-rule-in-chain", metaVar = "[True | False]", usage = "Set matching rule in chain flag.")
    private String matchingRuleInChain;

    @Option(name = "--base-dn-for-nested-groups", metaVar = "[True | False]", usage = "Set base dn for nested groups flag.")
    private String baseDnForNestedGroups;

    @Option(name = "--site-affinity", metaVar = "[True | False]", usage = "Set site affinity flag.")
    private String siteAffinity;

    @Option(name = "--direct-groups-search", metaVar = "[True | False]", usage = "Set direct groups search flag.")
    private String directGroupsSearch;

    @Option(name = "--link-account-with-upn", metaVar = "[True | False]", usage = "Set link account with upn flag. For smartcard login.")
    private String linkAccountWithUpn;

    @Option(name = "--hint-attribute-name", metaVar = "[Hint attribute name]", usage = "Set hint attribute name."
            + " Used for smartcard login, the account attribute that match to username hint. Default is \"samAccountName\"")
    private String hintAttributeName;

//    @Option(name = "--system-domain-store-password", metaVar = "[Password]", usage = "Set system domain store password.")
//    private String systemDomainStorePwd;

    @Option(name = "--test-tls", metaVar = "[all | Identity provider name]", usage = "Test if TLS is supported on identity provider server."
            + " Type [all] to test all or type in the specific identity provider name.")
    private String testProvider;

    @Override
    public String getShortDescription() {
        return String.format("Commands for identity provider configurations. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (StringUtils.isNotEmpty(providerToList)) {
            displayIdentityProviderInfo();
//        Currently updating info on system domain is not supported on server side, will enable this cmd once server side change is made.
//        } else if (StringUtils.isNotEmpty(systemDomainStorePwd)) {
//            updateSystemDomainStorePassword();
        } else if (StringUtils.isNotEmpty(testProvider)) {
            testIdentityProviderServerSupportsTls();
        } else {
            setIdentityProviderConfigurations();
        }
	}

    private void setIdentityProviderConfigurations() throws Exception {
        if (StringUtils.isEmpty(name)) {
            throw new IllegalArgumentException("Please use --provider option to provide the provider name to update provider's configuration.");
        }

        IdmClient client = SSOConfigurationUtils.getIdmClient();
        IdentityProviderDTO provider = client.provider().get(tenant, name);
        if (provider == null) {
            throw new IllegalArgumentException(String.format("Identity provider [%s] does not exists.", name));
        }
        IdentityProviderDTO.Builder builder = new IdentityProviderDTO.Builder();
        builder.withAlias(SSOConfigurationUtils.checkString(alias, provider.getAlias()));
        builder.withFriendlyName(SSOConfigurationUtils.checkString(friendlyName, provider.getFriendlyName()));
        // TODO remove comment below when hint attribut name is merged in lightwave
        //builder.withHintAttributeName(SSOConfigurationUtils.checkString(hintAttributeName, provider.getHintAttributeName()));
        builder.withMatchingRuleInChainEnabled(SSOConfigurationUtils.checkBoolean(matchingRuleInChain, provider.isMatchingRuleInChainEnabled()));
        builder.withServicePrincipalName(SSOConfigurationUtils.checkString(spn, provider.getServicePrincipalName()));
        builder.withBaseDnForNestedGroupsEnabled(SSOConfigurationUtils.checkBoolean(baseDnForNestedGroups, provider.isBaseDnForNestedGroupsEnabled()));
        builder.withSiteAffinityEnabled(SSOConfigurationUtils.checkBoolean(siteAffinity, provider.isSiteAffinityEnabled()));
        builder.withDirectGroupsSearchEnabled(SSOConfigurationUtils.checkBoolean(directGroupsSearch, provider.isDirectGroupsSearchEnabled()));
        //builder.withLinkAccountWithUPN(SSOConfigurationUtils.checkBoolean(linkAccountWithUpn, provider.getLinkAccountWithUPN()));
        client.provider().update(tenant, name, builder.build());
        logger.info("Updated configuration for identity provider: " + name);
    }

//    private void updateSystemDomainStorePassword() throws Exception {
//        IdmClient client = SSOConfigurationUtils.getIdmClient();
//        IdentityProviderDTO systemDomainStore = null;
//        for (IdentityProviderDTO provider : client.provider().getAll(tenant)) {
//            if (provider.getDomainType().equalsIgnoreCase(SYSTEM_DOMAIN)) {
//                systemDomainStore = provider;
//                break;
//            }
//        }
//        if (systemDomainStore != null) {
//            IdentityProviderDTO updatedSystemDomainStore = new IdentityProviderDTO.Builder()
//                    .withPassword(systemDomainStorePwd)
//                    .withDomainType(systemDomainStore.getDomainType())
//                    .withAlias(systemDomainStore.getAlias())
//                    .withType(systemDomainStore.getType())
//                    .withName(systemDomainStore.getName())
//                    .withUserBaseDN(systemDomainStore.getUserBaseDN())
//                    .withCertificates(systemDomainStore.getCertificates())
//                    .withAttributesMap(systemDomainStore.getAttributesMap())
//                    .withAuthenticationType(systemDomainStore.getAuthenticationType())
//                    .withBaseDnForNestedGroupsEnabled(systemDomainStore.isBaseDnForNestedGroupsEnabled())
//                    .withConnectionStrings(systemDomainStore.getConnectionStrings())
//                    .withDirectGroupsSearchEnabled(systemDomainStore.isDirectGroupsSearchEnabled())
//                    .withFriendlyName(systemDomainStore.getFriendlyName())
//                    .withGroupBaseDN(systemDomainStore.getGroupBaseDN())
//                    .build();
//            client.provider().update(tenant, systemDomainStore.getName(), updatedSystemDomainStore);
//            logger.info("Updated system domain store password on tenant: " + tenant);
//        }
//    }

    private void testIdentityProviderServerSupportsTls() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        System.out.println(String.format("\n********** IDENTITY PROVIDER CONNECTION LDAPS WITH TLSv1 MINIMUM PROTOCOL FOR %s **********", tenant));
        if (testProvider.equalsIgnoreCase("all")) {
            List<IdentityProviderDTO> providers = client.provider().getAll(tenant);
            for (IdentityProviderDTO provider : providers) {
                testIdentitySourceServerSupportsTls(provider);
            }
        } else {
            testIdentitySourceServerSupportsTls(client.provider().get(tenant, testProvider));
        }
    }

    private void testIdentitySourceServerSupportsTls(IdentityProviderDTO provider) throws Exception {
        for (String connectionString : provider.getConnectionStrings()) {
            URI connectionURI = new URI(connectionString);
            if (connectionURI.getScheme() == null) {
                continue;
            }
            boolean isLdaps = connectionURI.getScheme().compareToIgnoreCase(LDAPS_SCHEMA) == 0;
            if (isLdaps) {
                try {
                    SSOConfigurationUtils.checkMinimumProtocolIsSupported(connectionURI);
                    SSOConfigurationUtils.displayParamNameAndValue("Test TLS Connection", connectionString + " PASSED");
                } catch (Exception e) {
                    SSOConfigurationUtils.displayParamNameAndValue("Test TLS Connection", connectionString + " FAILED - " + e.toString());
                }
            }
        }
    }

    private void displayIdentityProviderInfo() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        if (providerToList.equalsIgnoreCase("all")) {
            List<IdentityProviderDTO> providers = client.provider().getAll(tenant);
            System.out.println(String.format("Total number of identity providers retrieved for tenant: %s : %d", tenant, providers.size()));
            for(IdentityProviderDTO provider : providers){
                displayIdentityProviderInfo(provider);
            }
        } else {
            displayIdentityProviderInfo(client.provider().get(tenant, providerToList));
        }
    }

    private void displayIdentityProviderInfo(IdentityProviderDTO provider) {
        System.out.println("\n****** IDENTITY PROVIDER INFORMATION ******");
        SSOConfigurationUtils.displayParamNameAndValue("Identity Provider Name", provider.getName());
        SSOConfigurationUtils.displayParamNameAndValue("Domain Type", provider.getDomainType() != null ? provider.getDomainType() : StringUtils.EMPTY);
        SSOConfigurationUtils.displayParamNameAndValue("Alias", provider.getAlias());
        SSOConfigurationUtils.displayParamNameAndValue("Frendly Name", provider.getFriendlyName());
        SSOConfigurationUtils.displayParamNameAndValue("Provider Type", provider.getType());
        SSOConfigurationUtils.displayParamNameAndValue("Authentication Type", provider.getAuthenticationType());
        SSOConfigurationUtils.displayParamNameAndValue("User BaseDN", provider.getUserBaseDN());
        SSOConfigurationUtils.displayParamNameAndValue("Group BaseDN", provider.getGroupBaseDN());
        SSOConfigurationUtils.displayParamNameAndValue("Username", provider.getUsername());
        SSOConfigurationUtils.displayParamNameAndValue("Service Principal Name", provider.getServicePrincipalName());

        Map<String, String> attributeMappings = provider.getAttributesMap();
        if(attributeMappings != null){
            System.out.println("****** Attributes ******");
            for(Entry<String,String> attributeMapping : attributeMappings.entrySet()){
                SSOConfigurationUtils.displayParamNameAndValue(attributeMapping.getKey(), attributeMapping.getValue());
            }
        }

        System.out.println("****** Connection Strings ******");
        SSOConfigurationUtils.displayParamNameAndValues("Connection String", provider.getConnectionStrings());

        System.out.println("****** UPN Suffixes ******");
        SSOConfigurationUtils.displayParamNameAndValues("UPN Suffix", provider.getUpnSuffixes());

        SSOConfigurationUtils.displayParamNameAndValue("Search Timeout In Seconds",String.valueOf(provider.getSearchTimeOutInSeconds()));

        Map<String, SchemaObjectMappingDTO> schemaMappings = provider.getSchema();
        if(schemaMappings != null){
            System.out.println("****** Identity Store Schema Mappings ******");
            for(Entry<String, SchemaObjectMappingDTO> schemaMapping : schemaMappings.entrySet()){
                System.out.println("------ Identity Object ------");
                SSOConfigurationUtils.displayParamNameAndValue("Object Class", schemaMapping.getValue().getObjectClass());
                SSOConfigurationUtils.displayParamNameAndValue("Object Id", schemaMapping.getKey());
                for(Entry<String, String> attributeMapping : schemaMapping.getValue().getAttributeMappings().entrySet()){
                    SSOConfigurationUtils.displayParamNameAndValue("Attribute Id", attributeMapping.getKey());
                    SSOConfigurationUtils.displayParamNameAndValue("Attribute Name", attributeMapping.getValue());
                }
            }
        }

        SSOConfigurationUtils.displayParamNameAndValue("BaseDN for Nested Groups Enabled", provider.isBaseDnForNestedGroupsEnabled() == null ? "null" : String.valueOf(provider.isBaseDnForNestedGroupsEnabled()));
        SSOConfigurationUtils.displayParamNameAndValue("Direct Group Search Enabled", provider.isDirectGroupsSearchEnabled() == null ? "null" : String.valueOf(provider.isDirectGroupsSearchEnabled()));
        SSOConfigurationUtils.displayParamNameAndValue("Use Machine Account", provider.isMachineAccount() == null ? "null" : String.valueOf(provider.isMachineAccount()));
        SSOConfigurationUtils.displayParamNameAndValue("Matching Rule in Chain Enabled", provider.isMatchingRuleInChainEnabled() == null ? "null" : String.valueOf(provider.isMatchingRuleInChainEnabled()));
        SSOConfigurationUtils.displayParamNameAndValue("Site Affinity Enabled", provider.isSiteAffinityEnabled() == null ? "null" : String.valueOf(provider.isSiteAffinityEnabled()));
    }
}
