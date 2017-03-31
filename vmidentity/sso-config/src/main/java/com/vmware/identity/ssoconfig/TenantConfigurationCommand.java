package com.vmware.identity.ssoconfig;

import java.io.File;
import java.util.Scanner;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.BrandPolicyDTO;
import com.vmware.identity.rest.idm.data.ProviderPolicyDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;

public class TenantConfigurationCommand extends SSOConfigCommand {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TenantConfigurationCommand.class);

    @Option(name = "--default-provider", metaVar = "[Provider name]", usage = "Set default provider for the tenant.")
    private String defaultProvider;

    @Option(name = "--default-provider-alias", metaVar = "[Provider alias]", usage = "Set default provider alias for the tenant.")
    private String defaultProviderAlias;

    @Option(name = "--enable-idp-selection", metaVar = "[True | False]", usage = "Set provider selection flag for the tenant.")
    private String idpSelection;

    @Option(name = "--enable-logon-banner", metaVar = "[True | False]", usage = "Set logon banner flag for the tenant.")
    private String logonBanner;

    @Option(name = "--logon-banner-checkbox", metaVar = "[True | False]", usage = "Set logon banner checkbox flag for the tenant.")
    private String logonBannerCheckbox;

    @Option(name = "--logon-banner-title", metaVar = "[True | False]", usage = "Set logon banner title for the tenant.")
    private String logonBannerTitle;

    @Option(name = "--logon-banner-file", metaVar = "[Logon banner content file]", usage = "Logon banner content file location.")
    private String logonBannerFile;

    @Option(name = "--add-group", metaVar = "[Group name]", usage = "Add group to the tenant and domain.")
    private String groupName;

    @Option(name = "--domain", metaVar = "[Domain name]", usage = "Domain name in the tenant.")
    private String domain;

    @Option(name = "--group-details", metaVar = "[Group description]", usage = "Description of the group to be added. Optional.")
    private String groupDetails;

    @Override
    public String getShortDescription() {
        return String.format("Commands for tenant configurations. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (isProviderPolicyUpdated()) {
            setProviderPolicy();
        } else if (isLogonBannerUpdated()) {
            setLogonBanner();
        } else if (StringUtils.isNotEmpty(groupName) && StringUtils.isNotEmpty(domain)) {
            addGroup();
        }
	}

    private void addGroup() throws Exception {
        VmdirClient client = SSOConfigurationUtils.getVmdirClient();
        GroupDTO group = new GroupDTO.Builder()
                .withName(groupName)
                .withDomain(domain)
                .withDetails(new GroupDetailsDTO.Builder().withDescription(groupDetails).build())
                .build();
        client.group().create(tenant, group);
        logger.info(String.format("Added group %s to domain %s in tenant %s.", groupName, domain, tenant));
    }

    private boolean isLogonBannerUpdated() {
        return StringUtils.isNotEmpty(logonBanner)
                || StringUtils.isNotEmpty(logonBannerCheckbox)
                || StringUtils.isNotEmpty(logonBannerTitle)
                || StringUtils.isNotEmpty(logonBannerFile);
    }

    private boolean isProviderPolicyUpdated() {
        return StringUtils.isNotEmpty(defaultProvider)
                || StringUtils.isNotEmpty(idpSelection)
                || StringUtils.isNotEmpty(defaultProviderAlias);
    }

    private void setProviderPolicy() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        ProviderPolicyDTO currentProviderPolicy = client.tenant().getConfig(tenant).getProviderPolicy();
        ProviderPolicyDTO updatedProviderPolicy = new ProviderPolicyDTO.Builder()
                .withDefaultProvider(SSOConfigurationUtils.checkString(defaultProvider, currentProviderPolicy.getDefaultProvider()))
                .withDefaultProviderAlias(SSOConfigurationUtils.checkString(defaultProviderAlias, currentProviderPolicy.getDefaultProviderAlias()))
                .withProviderSelectionEnabled(SSOConfigurationUtils.checkBoolean(idpSelection, currentProviderPolicy.isProviderSelectionEnabled()))
                .build();
        TenantConfigurationDTO tenantConfig = new TenantConfigurationDTO.Builder().withProviderPolicy(updatedProviderPolicy).build();
        client.tenant().updateConfig(tenant, tenantConfig);
        logger.info("Successfully updated provider policy for tenant " + tenant);
    }

    private void setLogonBanner() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        BrandPolicyDTO currentBrandPolicy = client.tenant().getConfig(tenant).getBrandPolicy();
        BrandPolicyDTO.Builder builder = new BrandPolicyDTO.Builder();
        boolean logonBannerDisabled = !SSOConfigurationUtils.checkBoolean(logonBanner, !currentBrandPolicy.isLogonBannerDisabled());
        builder.withLogonBannerDisabled(logonBannerDisabled);
        boolean logonBannerCheckboxEnabled = SSOConfigurationUtils.checkBoolean(logonBannerCheckbox, currentBrandPolicy.isLogonBannerCheckboxEnabled());
        builder.withLogonBannerCheckboxEnabled(logonBannerCheckboxEnabled);
        builder.withLogonBannerTitle(SSOConfigurationUtils.checkString(logonBannerTitle, currentBrandPolicy.getLogonBannerTitle()));
        Scanner scanner = new Scanner(new File(logonBannerFile));
        String logonBannerContent = scanner.useDelimiter("\\Z").next();
        scanner.close();
        builder.withLogonBannerContent(SSOConfigurationUtils.checkString(logonBannerContent, currentBrandPolicy.getLogonBannerContent()));
        TenantConfigurationDTO tenantConfig = new TenantConfigurationDTO.Builder().withBrandPolicy(builder.build()).build();
        client.tenant().updateConfig(tenant, tenantConfig);
        logger.info("Successfully updated logon banner for tenant " + tenant);
    }
}
