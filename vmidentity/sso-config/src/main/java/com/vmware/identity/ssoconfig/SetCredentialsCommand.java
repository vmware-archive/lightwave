package com.vmware.identity.ssoconfig;

import java.io.File;
import java.io.FileWriter;
import java.security.KeyStore;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;

import org.apache.commons.lang.StringUtils;
import org.kohsuke.args4j.Option;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.client.ClientConfig;
import com.vmware.identity.openidconnect.client.ConnectionConfig;
import com.vmware.identity.openidconnect.client.MetadataHelper;
import com.vmware.identity.openidconnect.client.OIDCClient;
import com.vmware.identity.openidconnect.client.OIDCTokens;
import com.vmware.identity.openidconnect.client.TokenSpec;
import com.vmware.identity.openidconnect.common.ProviderMetadata;

/**
 * Provide command line options for setting admin and keystore credentials.
 */
public class SetCredentialsCommand extends SSOConfigCommand {

    @Option(name = "-au", aliases = {"--admin-username"}, metaVar = "[Admin username]", usage = "System admin username.")
    private static String adminUsername;

    @Option(name = "-ap", aliases = {"--admin-password"}, metaVar = "[Admin password]", usage = "System admin password.")
    private static String adminPwd;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SetCredentialsCommand.class);

    @Override
    public String getShortDescription() {
        return String.format("Commands for setting admin and keystore credentials. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (StringUtils.isNotEmpty(adminUsername)) {
            if (StringUtils.isNotEmpty(adminPwd)) {
                String token = getBearerTokenByUsernamePwd(adminUsername, adminPwd);
                File file = new File(SSOConfigurationUtils.CREDENTIALS_FILE);
                file.createNewFile(); // create the file if it does not exist
                FileWriter fw = new FileWriter(file.getPath());
                fw.write(token);
                fw.close();
                logger.info("Successfully updated credentials with admoin username and password.");
            }
        }
    }

    private String getBearerTokenByUsernamePwd(String adminUsername, String adminPwd) throws Exception {
        if (StringUtils.isEmpty(adminUsername) || StringUtils.isEmpty(adminPwd)) {
            throw new IllegalStateException("Admin username and/or password is missing.");
        }

        KeyStore keyStore = SSOConfigurationUtils.getKeyStore();
        // retrieve OIDC meta data
        MetadataHelper metadataHelper = new MetadataHelper.Builder(SSOConfigurationUtils.IDP_HOST_ADDRESS).
                domainControllerPort(SSOConfigurationUtils.DEFAULT_OP_PORT).
                tenant(tenant).
                keyStore(keyStore).
                build();

        ProviderMetadata providerMetadata = metadataHelper.getProviderMetadata();
        RSAPublicKey providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);
        // create a non-registered OIDC client and get bearer tokens by admin user name/password
        ConnectionConfig connectionConfig = new ConnectionConfig(providerMetadata, providerPublicKey, keyStore);
        ClientConfig clientConfig = new ClientConfig(connectionConfig, null, null, 10 * 60L /* clockToleranceInSeconds */);
        OIDCClient nonRegisteredClient = new OIDCClient(clientConfig);
        TokenSpec tokenSpec = new TokenSpec.Builder().resourceServers(Arrays.asList("rs_admin_server")).build();
        OIDCTokens oidcTokens = nonRegisteredClient.acquireTokensByPassword(
                adminUsername,
                adminPwd,
                tokenSpec);
        return oidcTokens.getAccessToken().getValue();
    }
}
