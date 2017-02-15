package com.vmware.identity.ssoconfig;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.directory.rest.common.data.PasswordDetailsDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

public class UserResourceCommand extends SSOConfigCommand {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(UserResourceCommand.class);

    @Option(name = "--change-user-password", metaVar = "[User name]", usage = "Change user's password.")
    private String userToUpdatePwd;

    @Option(name = "--current-password", metaVar = "[Password]", usage = "User's current password.")
    private String currentPwd;

    @Option(name = "--new-password", metaVar = "[Password]", usage = "Set user's password.")
    private String newPwd;

    @Option(name = "--domain", metaVar = "[Domain name]", usage = "User's domain.")
    private String domain;

    @Option(name = "--email", metaVar = "[Email]", usage = "User's email.")
    private String email;

    @Option(name = "--upn", metaVar = "[UPN]", usage = "User Principal Name.")
    private String upn;

    @Option(name = "--firstname", metaVar = "[Firstname]", usage = "User's firstname.")
    private String firstname;

    @Option(name = "--lastname", metaVar = "[Lastname]", usage = "User's lastname.")
    private String lastname;

    @Option(name = "--description", metaVar = "[Description]", usage = "User's description.")
    private String description;

    @Option(name = "--add-user", metaVar = "[User name]", usage = "Add a user. Please provide the username.")
    private String userToAdd;

    @Option(name = "--delete-user", metaVar = "[User name]", usage = "Delete a user. Please provide the username.")
    private String userToDelete;

    @Override
    public String getShortDescription() {
        return String.format("Commands for user configurations. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (StringUtils.isNotEmpty(userToUpdatePwd)) {
            changeUserPwd();
        } else if (StringUtils.isNotEmpty(userToAdd)) {
            addUser();
        } else if (StringUtils.isNotEmpty(userToDelete)) {
            deleteUser();
        }
	}

    private void changeUserPwd() throws Exception {
        if (currentPwd == null) {
            throw new IllegalArgumentException("Please use --current-password to provide the current password.");
        }
        if (newPwd == null) {
            throw new IllegalArgumentException("Please use --new-password to provide the new password.");
        }
        if (domain == null) {
            throw new IllegalArgumentException("Please use --domain to provide the user's domain.");
        }
        VmdirClient client = SSOConfigurationUtils.getVmdirClient();
        client.user().updatePassword(tenant, userToUpdatePwd, domain, currentPwd, newPwd);
        logger.info("Updated password for user " + userToUpdatePwd);
    }

    private void addUser() throws Exception {
        if (newPwd == null) {
            throw new IllegalArgumentException("Please use --new-password to provide the new password.");
        }
        if (domain == null) {
            throw new IllegalArgumentException("Please use --domain to provide the user's domain.");
        }
        UserDetailsDTO details = new UserDetailsDTO.Builder()
                .withEmail(email)
                .withFirstName(firstname)
                .withLastName(lastname)
                .withUPN(upn)
                .withDescription(description)
                .build();
        PasswordDetailsDTO pwdDetails = new PasswordDetailsDTO.Builder()
                .withPassword(newPwd)
                .build();
        UserDTO user = new UserDTO.Builder()
                .withDetails(details)
                .withDomain(domain)
                .withName(userToAdd)
                .withPasswordDetails(pwdDetails)
                .build();
        VmdirClient client = SSOConfigurationUtils.getVmdirClient();
        client.user().create(tenant, user);
        logger.info("Added a new user " + userToAdd);
    }

    private void deleteUser() throws Exception {
        if (domain == null) {
            throw new IllegalArgumentException("Please use --domain to provide the user's domain.");
        }
        VmdirClient client = SSOConfigurationUtils.getVmdirClient();
        client.user().delete(tenant, userToDelete, domain);
        logger.info("Deleted the user: " + userToDelete);
    }
}
