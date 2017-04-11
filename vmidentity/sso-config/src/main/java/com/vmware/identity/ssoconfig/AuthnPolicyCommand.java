package com.vmware.identity.ssoconfig;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.kohsuke.args4j.Option;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;
import com.vmware.identity.rest.idm.data.ClientCertificatePolicyDTO;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;

public class AuthnPolicyCommand extends SSOConfigCommand{

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(AuthnPolicyCommand.class);

    @Option(name = "--cert-authn", metaVar = "[True | False]", usage = "Cert authentiation switch.")
    private String certAuthn;

    @Option(name = "--password-authn", metaVar = "[True | False]", usage = "Password authentication switch.")
    private String pwdAuthn;

    @Option(name = "--windows-authn", metaVar = "[True | False]", usage = "Windows authentication switch.")
    private String winAuthn;

    @Option(name = "--use-ocsp", metaVar = "[True | False]", usage = "OCSP switch.")
    private String useOcsp;

    @Option(name = "--ca-certs", metaVar = "[Root/Issuing CA files separated by comma]", usage = "Root or issuing CA files seperated by comma.")
    private String caCerts;

    @Option(name = "--use-cert-crl", metaVar = "[True | False]", usage = "Certificate CRL switch.")
    private String useCertCrl;

    @Option(name = "--crl-url", metaVar = "[URL]", usage = "CRL URL.")
    private String crlUrl;

    @Option(name = "--cert-policy-oids", metaVar = "[Oids separated by comma]", usage = "Cert Policy OIDs seperated by comma.")
    private String oids;

    @Option(name = "--revocation-check", metaVar = "[True | False]", usage = "Revocation check switch for client cert authentication.")
    private String revocationCheck;

    @Option(name = "--failover-to-crl", metaVar = "[True | False]", usage = "Fail over to CRL.")
    private String failoverToCrl;

    @Option(name = "--username-hint", metaVar = "[True | False]", usage = "Username hint switch.")
    private String hint;

    @Option(name = "--tc-certs", metaVar = "[Root/Issuing CA files separated by comma]", usage = "Set Tomcat client certificate authentication.")
    private String tcCerts;

    @Option(name = "--list", metaVar = "[all | tc-cert| Identity provider name]", usage = "List tenant authentication policy."
            + " Type [all] to list all."
            + " Or type in [tc-cert] to display Tomcat client certificate authentication configuration."
            + " Or type in a specific identity provider name.")
    private String list;

    @Override
    public String getShortDescription() {
        return String.format("Commands for tenant's authentication policy. Use %s %s for details.", this.getCommandName(), HELP_CMD);
    }

    @Override
    protected void execute() throws Exception {
        if (list != null) {
            if (list.equalsIgnoreCase("ALL")) {
                displayAuthnPolicy();
            } else if (list.equalsIgnoreCase("TC-CERT")){
                displayTCCertAuthn();
            } else {
                displayAuthnPolicy(list);
            }
        } else if (tcCerts != null) {
            setTCCertAuthn();
        } else {
            setAuthnPolicy();
        }
    }

    private void setAuthnPolicy() throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        TenantConfigurationDTO currentTenantConfig = client.tenant().getConfig(tenant);
        AuthenticationPolicyDTO currentAuthnPolicy = currentTenantConfig.getAuthenticationPolicy();
        ClientCertificatePolicyDTO currentCertPolicy = currentAuthnPolicy.getClientCertificatePolicy();

        com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO.Builder authnPolicyBuilder = new AuthenticationPolicyDTO.Builder();
        boolean isPasswordBasedAuthEnabled = SSOConfigurationUtils.checkBoolean(pwdAuthn, currentAuthnPolicy.isPasswordBasedAuthenticationEnabled());
        authnPolicyBuilder.withPasswordBasedAuthenticationEnabled(isPasswordBasedAuthEnabled);

        boolean isWindowsBasedAuthEnabled = SSOConfigurationUtils.checkBoolean(winAuthn, currentAuthnPolicy.isWindowsBasedAuthenticationEnabled());
        authnPolicyBuilder.withWindowsBasedAuthenticationEnabled(isWindowsBasedAuthEnabled);

        boolean isCertBasedAuthEnabled = SSOConfigurationUtils.checkBoolean(certAuthn, currentAuthnPolicy.isCertificateBasedAuthenticationEnabled());
        authnPolicyBuilder.withCertificateBasedAuthenticationEnabled(isCertBasedAuthEnabled);

        // TODO add rsa config

        com.vmware.identity.rest.idm.data.ClientCertificatePolicyDTO.Builder certPolicyBuilder = new ClientCertificatePolicyDTO.Builder();
        boolean isRevocationCheckEnabled = SSOConfigurationUtils.checkBoolean(revocationCheck, currentCertPolicy.isRevocationCheckEnabled());
        certPolicyBuilder.withRevocationCheckEnabled(isRevocationCheckEnabled);

        boolean isOcspEnabled = SSOConfigurationUtils.checkBoolean(useOcsp, currentCertPolicy.isOcspEnabled());
        certPolicyBuilder.withOcspEnabled(isOcspEnabled);

        boolean isFailoverToCrlEnabled =  SSOConfigurationUtils.checkBoolean(failoverToCrl, Boolean.parseBoolean(failoverToCrl));
        certPolicyBuilder.withFailOverToCrlEnabled(isFailoverToCrlEnabled);

        boolean isCertCrlEnabled = SSOConfigurationUtils.checkBoolean(useCertCrl, currentCertPolicy.isCrlDistributionPointUsageEnabled());
        certPolicyBuilder.withCrlDistributionPointUsageEnabled(isCertCrlEnabled);

//        boolean isUsernameHintEnabled = SSOConfigurationUtils.checkBoolean(hint, currentCertPolicy.isUserNameHintEnabled());
//        certPolicyBuilder.withUserNameHintEnabled(isUsernameHintEnabled);

        if (oids != null) {
            List<String> oidList = oids == null || oids.isEmpty() ? null : Arrays.asList(oids.split(","));
            certPolicyBuilder.withCertPolicyOIDs(oidList);
        } else {
            certPolicyBuilder.withCertPolicyOIDs(currentCertPolicy.getCertPolicyOIDs());
        }

        if (caCerts != null) {
            CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
            List<CertificateDTO> trustedCertificates = new ArrayList<>();
            String[] trustedCAFiles = caCerts.split(",");
            for (String fn : trustedCAFiles) {
                if (fn != null) {
                    fn = fn.trim();
                    if (fn.startsWith("~" + File.separator)) {
                        fn = System.getProperty("user.home") + fn.substring(1);
                    }
                    Path path = Paths.get(fn);
                    InputStream inStream = new FileInputStream(path.toString());
                    Certificate cert = certFactory.generateCertificate(inStream);
                    trustedCertificates.add(new CertificateDTO((X509Certificate) cert));
                    inStream.close();
                }
            }
            certPolicyBuilder.withTrustedCACertificates(trustedCertificates);
        } else {
            certPolicyBuilder.withTrustedCACertificates(currentCertPolicy.getTrustedCACertificates());
        }

        if (crlUrl!= null) {
            certPolicyBuilder.withCrlDistributionPointOverride(crlUrl);
        } else {
            certPolicyBuilder.withCrlDistributionPointOverride(currentCertPolicy.getCrlDistributionPointOverride());
        }

        ClientCertificatePolicyDTO certPolicy = certPolicyBuilder.build();
        AuthenticationPolicyDTO authnPolicy = authnPolicyBuilder.withClientCertificatePolicy(certPolicy).build();
        TenantConfigurationDTO tenantConfig = new TenantConfigurationDTO.Builder().withAuthenticationPolicy(authnPolicy).build();
        client.tenant().updateConfig(tenant, tenantConfig);
        System.out.println("Updated authentication policy on tenant: " + tenant);
    }

    private void displayAuthnPolicy() throws Exception {
        // Display Authentication policy of tenant
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        TenantConfigurationDTO tenantConfig = client.tenant().getConfig(tenant);
        AuthenticationPolicyDTO tenantAuthnPolicy = tenantConfig.getAuthenticationPolicy();
        System.out.println(String.format("Authentication policy on tenant : '%s' :" ,tenant));
        SSOConfigurationUtils.displayAuthenticationPolicy(tenantAuthnPolicy);

        // Display Authentication policy (if any) of identity sources belonging to tenant
        Collection<IdentityProviderDTO> providers = client.provider().getAll(tenant);
        System.out.println("Identity source-sepecific policy:");
        for(IdentityProviderDTO provider : providers) {
            displayAuthnPolicy(provider);
        }
    }

    private void displayAuthnPolicy(String providerName) throws Exception {
        IdmClient client = SSOConfigurationUtils.getIdmClient();
        IdentityProviderDTO provider = client.provider().get(tenant, providerName);
        displayAuthnPolicy(provider);
    }

    private void displayAuthnPolicy(IdentityProviderDTO provider) {
        String providerAuthnType = provider.getAuthenticationType();
        if(providerAuthnType != null) {
            SSOConfigurationUtils.displayParamNameAndValue(provider.getName(), providerAuthnType);
       } else {
           System.out.println(String.format("Identity source-specific policy for:"
                   + " '%s' is not set. Will use tenant Authentication policy.", provider.getName()));
       }
    }

    private void displayTCCertAuthn() throws Exception {
        String tcServerConfigFile = SSOConfigurationUtils.getTCServerConfigFileFullName();
        TCServerManager manager = new TCServerManager(tcServerConfigFile);

        SSOConfigurationUtils.displayParamNameAndValue(TCServerManager.trustStoreFileAttrName, manager.getAttrValue(TCServerManager.trustStoreFileAttrName));
        SSOConfigurationUtils.displayParamNameAndValue(TCServerManager.trustStorePasswordAttrName, manager.getAttrValue(TCServerManager.trustStorePasswordAttrName));
        SSOConfigurationUtils.displayParamNameAndValue(TCServerManager.trustStoreTypeAttrName, manager.getAttrValue(TCServerManager.trustStoreTypeAttrName));
    }

    private void setTCCertAuthn() throws Exception {
        // Backup TomCat configuration file for roll back if needed.
        String tcServerConfigFile = SSOConfigurationUtils.getTCServerConfigFileFullName();
        String tcConfigOrig = tcServerConfigFile + ".orig";
        if(Files.exists(FileSystems.getDefault().getPath(tcServerConfigFile))){
            Files.copy(FileSystems.getDefault().getPath(tcServerConfigFile), new FileOutputStream(tcConfigOrig));
        }
        else
        {
            throw new FileNotFoundException(String.format("TomCat server.xml not found at %s", tcServerConfigFile));
        }

        String defaultTrustedCaFile = SSOConfigurationUtils.getTCTrustedCAFileFullName();
        String defaultTrustedCaPassword = "changeme";
        String defaultTrustedCaType = "JKS";
        // Get trustedCaFile from TomCat configuration if it has been configured. Otherwise use the default value.
        TCServerManager tcManager = new TCServerManager(tcServerConfigFile);
        String trustedCaFile = tcManager
                .getAttrValue(TCServerManager.trustStoreFileAttrName);
        if (trustedCaFile == null) {
            tcManager.setAttrValue(TCServerManager.trustStoreFileAttrName, defaultTrustedCaFile);
            trustedCaFile = defaultTrustedCaFile;
        }
        // Get trustedCaPassword from TomCat configuration if it has been configured. Otherwise use the default value.
        String trustedCaPassword = tcManager
                .getAttrValue(TCServerManager.trustStorePasswordAttrName);
        if (trustedCaPassword == null) {
            tcManager.setAttrValue(TCServerManager.trustStorePasswordAttrName, defaultTrustedCaPassword);
            trustedCaPassword = defaultTrustedCaPassword;
        }
        // Get trustedCaType from TomCat configuration if it has been configured. Otherwise use the default value.
        String trustedCaType = tcManager
                .getAttrValue(TCServerManager.trustStoreTypeAttrName);
        if (trustedCaType == null) {
            tcManager.setAttrValue(TCServerManager.trustStoreTypeAttrName, defaultTrustedCaType);
            trustedCaType = defaultTrustedCaType;
        }

        // Backup trustedCAStore file for roll back if needed.
        String trustedCaFileOrig = trustedCaFile + ".orig";
        if(Files.exists(FileSystems.getDefault().getPath(trustedCaFile))){
            Files.copy(FileSystems.getDefault().getPath(trustedCaFile), new FileOutputStream(trustedCaFileOrig));
        }

        // Set attributes in tcManager.
        KeyStoreManager ksManager = null;
        // set clientCertAuth value to false in order to prompt the client certificate
        // only for the resourse/url configured in tomcat security realm
        tcManager.setAttrValue(TCServerManager.clientAuthAttrName, TCServerManager.clientAuthAttributeCertFalse);
        tcManager.setAttrValue(TCServerManager.trustStoreFileAttrName, trustedCaFile);
        tcManager.setAttrValue(TCServerManager.trustStorePasswordAttrName, trustedCaPassword);
        tcManager.setAttrValue(TCServerManager.trustStoreTypeAttrName, trustedCaType);
        String[] certFileNames = null;
        if (tcCerts != null) {
            certFileNames = tcCerts.split(",");
        }
        // import trusted CA certificates to trustedCaFile.
        if(certFileNames != null){
            ksManager = new KeyStoreManager(trustedCaFile, trustedCaPassword, trustedCaType);
            ksManager.importCerts(certFileNames, null);
        }

        try
        {
            // Save the configuration change to server.xml files
            tcManager.saveToXmlFile();
            if(ksManager != null){
                ksManager.saveToXmlFile();
            }
        }
        catch(Exception e){
            try {
                // Roll back to original server.xml file.
                if (Files
                        .exists(FileSystems.getDefault().getPath(tcConfigOrig))) {
                    Files.copy(FileSystems.getDefault().getPath(tcConfigOrig),
                            new FileOutputStream(tcServerConfigFile));
                }
                // Roll back to original trustedca file.
                if (Files.exists(FileSystems.getDefault().getPath(
                        tcServerConfigFile))) {
                    Files.copy(
                            FileSystems.getDefault()
                                    .getPath(trustedCaFileOrig),
                            new FileOutputStream(trustedCaFile));
                }
            } catch (IOException ioe) {
                System.out.println(String.format("Rolling back failed with exception: %s", ioe.toString()));
                System.out.println(String.format("To restore original configuration, please mannually copy %s to %s", tcConfigOrig, tcServerConfigFile));
                System.out.println(String.format("  and copy %s to %s", trustedCaFileOrig, trustedCaFile));
            }

            throw e;
        }
    }
}
