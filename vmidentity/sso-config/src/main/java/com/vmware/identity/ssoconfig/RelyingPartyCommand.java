package com.vmware.identity.ssoconfig;

import java.io.File;
import java.util.List;
import java.util.Scanner;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.AssertionConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.AttributeConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;

/**
 * Relying Party configuration commands.
 *
 */
public class RelyingPartyCommand extends SSOConfigCommand {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(RelyingPartyCommand.class);

    @Option(name = "--delete", metaVar = "[Relying Party Name]", usage = "Delete the relying party with the name.")
    private String nameToDelete;

    @Option(name = "--register", metaVar = "[Metadata File path]", usage = "Relying party metadata file location to register.")
    private String metadataFile;

    @Option(name = "--list", metaVar = "[all | Relying party name]", usage = "List tenant relying party configurations."
            + " Type [all] to list all or type in the specific relying party name.")
    private String rpToList;

    @Override
    public String getShortDescription() {
        return String.format("Commands for relying party configurations. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (StringUtils.isNotEmpty(metadataFile)) {
            registerRelyingParty();
        } else if (StringUtils.isNotEmpty(nameToDelete)) {
            deleteRelyingParty();
        } else if (StringUtils.isNotEmpty(rpToList)) {
            list();
        }
	}

    private void registerRelyingParty() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        Scanner scanner = new Scanner(new File(metadataFile));
        String metadata = scanner.useDelimiter("\\Z").next();
        scanner.close();
        client.relyingParty().register(tenant, metadata);
        logger.info("Successfully registered relying party for tenant " + tenant);
    }

    private void deleteRelyingParty() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        client.relyingParty().delete(tenant, nameToDelete);
        logger.info("Successfully deleted relying party for tenant " + tenant);
    }

    private void list() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        if (rpToList.equalsIgnoreCase("all")) {
            List<RelyingPartyDTO> rpList = client.relyingParty().getAll(tenant);
            if (rpList == null || rpList.isEmpty()) {
                System.out.println("No relying party has been configured for tenant: " + tenant);
                return;
            }
            System.out.println("Relying parties' name and entity ID for tenant " + tenant);
            for (RelyingPartyDTO rp : rpList) {
                displayRelyingParty(rp);
            }
        } else {
            RelyingPartyDTO rp = client.relyingParty().get(tenant, rpToList);
            displayRelyingParty(rp);
        }
    }

    private void displayRelyingParty(RelyingPartyDTO rp) {
        System.out.println("Configuration for relying party: " + rp.getName() + "\n");
        SSOConfigurationUtils.displayParamNameAndValue("Entity ID", rp.getUrl());
        SSOConfigurationUtils.displayParamNameAndValue("Authentication Request Signed", rp.isAuthnRequestsSigned());
        SSOConfigurationUtils.displayParamNameAndValue("Default Assertion Consumer Service", rp.getDefaultAssertionConsumerService());

        if (rp.getAssertionConsumerServices() != null) {
            for (AssertionConsumerServiceDTO e : rp.getAssertionConsumerServices()) {
                SSOConfigurationUtils.displayParamNameAndValue("Assertion Consumer Service", e.getName());
            }
        }

        if (rp.getAttributeConsumerServices() != null) {
            for (AttributeConsumerServiceDTO e : rp.getAttributeConsumerServices()) {
                SSOConfigurationUtils.displayParamNameAndValue("Assertion Consumer Service", e.getName());
            }
        }

        if (rp.getSingleLogoutServices() != null) {
            for (ServiceEndpointDTO e : rp.getSingleLogoutServices()) {
                SSOConfigurationUtils.displayParamNameAndValue("SLO Service", e.getEndpoint());
            }
        }
        SSOConfigurationUtils.displaySeparationLine();
    }
}
