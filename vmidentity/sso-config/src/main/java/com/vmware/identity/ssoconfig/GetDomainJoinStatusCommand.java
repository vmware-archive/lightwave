package com.vmware.identity.ssoconfig;

import java.util.List;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.afd.client.AfdClient;
import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinInfoDTO;

public class GetDomainJoinStatusCommand extends SSOConfigCommand {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SSOConfigCommand.class);

    @Override
    public String getShortDescription() {
        return String.format("Command to get domain join status. Use %s to get domain join status.", this.getCommandName());
    }

    @Override
    protected void setCommandAndParseArguments(String command, List<String> arguments) {
        this.command = command;
        this.arguments = arguments;
        try {
            execute();
        } catch (Exception e) {
            logger.error("Encountered an error when running command.", e);
            System.exit(1);
        }
    }

    @Override
    protected void execute() throws Exception {
        AfdClient client = SSOConfigurationUtils.getAfdClient();
        ActiveDirectoryJoinInfoDTO joinInfo = client.activeDirectory().getStatus();
        System.out.println("********** DOMAIN JOIN INFORMATION **********\n");
        SSOConfigurationUtils.displayParamNameAndValue("Domain Join Status", joinInfo.getStatus());
        if (joinInfo.getName() != null) {
            SSOConfigurationUtils.displayParamNameAndValue("Domain Name", joinInfo.getName());
        }
        if (joinInfo.getAlias() != null) {
            SSOConfigurationUtils.displayParamNameAndValue("Domain Alias", joinInfo.getAlias());
        }
        if (joinInfo.getDn() != null) {
            SSOConfigurationUtils.displayParamNameAndValue("Domain DN", joinInfo.getDn());
        }
    }
}
