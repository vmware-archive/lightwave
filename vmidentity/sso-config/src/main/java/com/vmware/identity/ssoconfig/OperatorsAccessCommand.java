package com.vmware.identity.ssoconfig;

import java.io.File;
import java.nio.file.NoSuchFileException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Scanner;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.identity.rest.idm.data.attributes.TenantConfigType;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.OperatorsAccessPolicyDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;

/**
 * Operators Access configuration Commands.
 *
 */
public class OperatorsAccessCommand extends SSOConfigCommand {

    @Option(name = "--show", usage = "Print current Operators Access Policy settings")
    private boolean showOption;

    @Option(name = "--enable", metaVar ="[True | False]", usage = "Enable Operators Access.")
    private String enabledOption;

    @Option(name = "--userBaseDn", metaVar = "[base Dn for users]", usage = "Set user base dn for operators access. Can only be used in system tenant context.")
    private String userBaseDnOption;

    @Option(name = "--groupBaseDn", metaVar = "[base Dn for groups]", usage = "Set group Base dn for operators access. Can only be used in system tenant context.")
    private String groupBaseDnOption;

    @Override
    public String getShortDescription() {
        return String.format(
                "Commands for managing operators access in tenant. Use %s %s for details.",
                this.getCommandName(), HELP_CMD
        );
    }

    @Override
    protected void execute() throws Exception {
        if (this.showOption){
            this.show();
        } else if (StringUtils.isNotEmpty(this.enabledOption)) {
            this.enable();
        } else {
            throw new UsageException("Either --show or --enable True|False are expected");
        }
    }

    private void show() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        List<TenantConfigType> configs = new ArrayList<TenantConfigType>(1);
        configs.add(TenantConfigType.OPERATORS_ACCESS);
        TenantConfigurationDTO tenantConfig = client.tenant().getConfig(this.tenant, configs);
        OperatorsAccessPolicyDTO tenantOperatorsPolicy = tenantConfig.getOperatorsAccessPolicy();
        System.out.println(String.format("Operators Access policy on tenant : '%s' :" ,tenant));
        SSOConfigurationUtils.displayOperatorsAccessPolicy(tenantOperatorsPolicy);
    }

    private void enable() throws Exception {
        boolean isEnabled = SSOConfigurationUtils.checkBoolean(this.enabledOption, false);
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        List<TenantConfigType> configs = new ArrayList<TenantConfigType>(1);
        configs.add(TenantConfigType.OPERATORS_ACCESS);
        TenantConfigurationDTO existingConfig = client.tenant().getConfig(this.tenant, configs);
        OperatorsAccessPolicyDTO existingPolicy = existingConfig.getOperatorsAccessPolicy();
        if ( (existingPolicy != null) || ( isEnabled == true ) ) {
            String existingUserBaseDn = null;
            String existingGroupBaseDn = null;
            if ( existingPolicy != null ) {
                existingUserBaseDn = existingPolicy.getUserBaseDn();
                existingGroupBaseDn = existingPolicy.getGroupBaseDn();
            }

            OperatorsAccessPolicyDTO policyDto = new OperatorsAccessPolicyDTO.Builder()
               .withEnabled(isEnabled)
               .withUserBaseDn( this.userBaseDnOption != null ? this.userBaseDnOption : existingUserBaseDn )
               .withGroupBaseDn( this.groupBaseDnOption != null ? this.groupBaseDnOption : existingGroupBaseDn )
               .build();

            TenantConfigurationDTO dto = TenantConfigurationDTO.builder()
               .withOperatorsAccessPolicy(policyDto).build();
            existingConfig = client.tenant().updateConfig(this.tenant, dto);
        }
        System.out.println(String.format("Successfully set Operators Access policy on tenant : '%s' :" ,tenant));
        SSOConfigurationUtils.displayOperatorsAccessPolicy(existingConfig.getOperatorsAccessPolicy());
    }
}
