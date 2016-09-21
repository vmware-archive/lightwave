/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.lang.ProcessBuilder.Redirect;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.SecureRandom;
import java.util.Random;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.certificate.VMCAClient;
import com.vmware.identity.idm.IIdmServiceContext;
import com.vmware.identity.idm.IdmServiceContextFactory;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.installer.ReleaseUtil;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.idm.client.IServiceContextProvider;
import com.vmware.identity.interop.ldap.LdapConstants;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;
import com.vmware.provider.VecsLoadStoreParameter;

public class IdentityManagerUtil {
    private static final Logger log = LoggerFactory
            .getLogger(IdentityManagerUtil.class);

    private final String _idmHost = "localhost";
    private String username;
    private final String domainName;
    private final String password;

    private DirectoryBindInformation _bindInfo = null;

    private final ServiceContextProvider _idmClientCtxtProvider;

    public IdentityManagerUtil(String username, String domainName,
            String password) {
        Validate.validateNotEmpty(username, "Username");

        this.username = username;
        this.domainName = domainName;
        this.password = password;
        _idmClientCtxtProvider = new ServiceContextProvider();
    }


    public void install() throws Exception {

        log.info("Creating signing certs");

        SigningCertificates certs = SigningCertificates.create("localhost",
                domainName, username, password);

        // create idmLogin.txt file and remove signkey and ssoserver.key once we used them.
        createIDMLoginFile();
        log.info("Starting Identity Manager service");
        startService();

        log.info("Configuring Identity Manager service");

        configureService(certs);
    }

	private void createIDMLoginFile() throws IdentityManagerInstallerException {

		char[] CHARSET_AZ_09 = PSCConstants.CHARSET_ALPHANUMERIC
				.toCharArray();
		String idmFile = InstallerUtils.joinPath(InstallerUtils
				.getInstallerHelper().getIdmLoginPath(),
				PSCConstants.IDM_LOGIN_FILE);
		try {
			File file = new File(InstallerUtils
					.getInstallerHelper().getIdmLoginPath());
			if(!file.exists()){
				file.mkdirs();
			}

			file= new File(idmFile);
			BufferedWriter bw = new BufferedWriter(
					new FileWriter(file));
			String ranString = randomString(CHARSET_AZ_09, 70);
			bw.write(ranString, 0, ranString.length());

			Path idmLoginPath = Paths.get(idmFile);
			InstallerUtils.getInstallerHelper().setPermissions(idmLoginPath);

		} catch (FileNotFoundException ex) {
			log.error("IDM Login file, " + idmFile + " not found", ex);
            throw new IdentityManagerInstallerException(
					"IDM Login file not found", ex);
		} catch (IOException ex) {
			log.error("Error while creating idm login", ex);
            throw new IdentityManagerInstallerException(
					"Error creating idm login file", ex);
        }
    }

	private static String randomString(char[] characterSet, int length) {
		Random random = new SecureRandom();
		char[] result = new char[length];
		for (int i = 0; i < result.length; i++) {
			// picks a random index out of character set > random character
			int randomCharIndex = random.nextInt(characterSet.length);
			result[i] = characterSet[randomCharIndex];
        }
		return new String(result);
    }

    public static void startService() throws IdentityManagerInstallerException {
        String[] svcCommand = InstallerUtils.getInstallerHelper()
                .getIDMServiceStartCommand();
        ProcessBuilder pb = new ProcessBuilder(svcCommand);
        pb.redirectErrorStream(true);

        String logFileName = InstallerUtils.getInstallerHelper()
                .getIDMServiceLogFile();
        File logFile = new File(logFileName);
        pb.redirectOutput(Redirect.appendTo(logFile));

        try {
            final Process p = pb.start();

            int exitCode = p.waitFor();

            if (exitCode != 0) {
                throw new IdentityManagerInstallerException(String.format(
                        "Failed to start idm service [error code: %d]",
                        exitCode), null);
            }

        } catch (InterruptedException | IOException e) {
            log.error("Failed to start idm service", e);
            throw new IdentityManagerInstallerException(
                    "Failed to start idm service", e);

        }
    }

    private void configureService(SigningCertificates certs) throws Exception {

        DirectoryBindInformation bindInfo = getBindInformation();

        String tenantDomain = getDomainFromDN(bindInfo.getBindDN());
        String tenantName = tenantDomain;

        if (tenantName == null || tenantName.length() == 0) {
            throw new IllegalStateException(String.format(
                    "An invalid tenant name [%s] has been configured.",
                    tenantName));
        }

        CasIdmClient client = new CasIdmClient(_idmHost, _idmClientCtxtProvider);

        Tenant tenant = client.getTenant(tenantName);

        if (tenant == null) {
            throw new IllegalStateException(String.format(
                    "Failed to find tenant [%s] on IDM server.", tenantName));
        }

        client.setTenantCredentials(tenantName, certs.getCerts(),
                certs.getKey());

        client.setClockTolerance(tenantName, InstallerUtils.CLOCK_TOLERANCE);
        client.setDelegationCount(tenantName,
                InstallerUtils.INITIAL_DELEGATION_COUNT);
        client.setRenewCount(tenantName, InstallerUtils.INITIAL_RENEW_COUNT);
        client.setMaximumBearerTokenLifetime(tenantName,
                InstallerUtils.MAX_LIFETIME_BEARER_TOKENS);
        client.setMaximumHoKTokenLifetime(tenantName,
                InstallerUtils.MAX_LIFETIME_HOK_TOKENS);

        client.updatePasswordExpirationConfiguration(tenantName,
                PasswordExpiration.createDefaultSettings());

	// Set default brand name for LIGHTWAVE
        if(ReleaseUtil.isLightwave()) {
            client.setBrandName(tenantName, "LIGHTWAVE Single Sign On");
	}

        client.authenticate(tenantName, getAdminUser(), getAdminPassword());
    }

    private DirectoryBindInformation getBindInformation() {
        if (_bindInfo == null) {
            IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance()
                    .getRegistryAdapter();

            IRegistryKey rootKey = regAdapter
                    .openRootKey((int) RegKeyAccess.KEY_READ);

            try {
                String adminDN = regAdapter.getStringValue(rootKey,
                        InstallerUtils.CONFIG_DIRECTORY_ROOT_KEY,
                        InstallerUtils.CONFIG_DIRECTORY_DCACCOUNT_DN_VALUE,
                        false);

                Integer port = regAdapter.getIntValue(rootKey,
                        InstallerUtils.CONFIG_DIRECTORY_PARAMETERS_KEY,
                        InstallerUtils.CONFIG_DIRECTORY_LDAP_PORT_VALUE, true);
                // fall back to lotus default port
                if (port == null || port == 0) {
                    port = LdapConstants.LDAP_PORT_LOTUS;
                }

                String adminPassword = regAdapter.getStringValue(rootKey,
                        InstallerUtils.CONFIG_DIRECTORY_CREDS_ROOT_KEY,
                        InstallerUtils.CONFIG_DIRECTORY_DCACCOUNT_PASSWORD,
                        false);

                _bindInfo = new DirectoryBindInformation(adminDN,
                        adminPassword, port);
            } finally {
                rootKey.close();
            }
        }

        return _bindInfo;
    }

    private String getAdminUser() {
        DirectoryBindInformation bindInfo = getBindInformation();

        return String.format("%s@%s",
                getAdminUserNameFromDN(bindInfo.getBindDN()),
                getDomainFromDN(bindInfo.getBindDN()));
    }

    private String getAdminPassword() {
        DirectoryBindInformation bindInfo = getBindInformation();

        return bindInfo.getPassword();
    }

    private static String getDomainFromDN(String dn) {
        final String dcPrefix = "DC=";
        StringBuilder sb = new StringBuilder();

        if (dn != null && dn.length() > 0) {
            String[] parts = dn.toUpperCase().split(",");

            int iPart = 0;

            for (String part : parts) {
                if (part.startsWith(dcPrefix)) {
                    if (iPart++ > 0) {
                        sb.append(".");
                    }

                    sb.append(part.substring(dcPrefix.length()));
                }
            }
        }

        return sb.toString().toLowerCase();
    }

    private static String getAdminUserNameFromDN(String dn) {
        final String cnPrefix = "CN=";
        String userName = "Administrator";

        if (dn != null && dn.length() > 0) {
            String[] parts = dn.toUpperCase().split(",");

            if (parts[0].startsWith(cnPrefix)) {
                userName = parts[0].substring(cnPrefix.length());
            }
        }

        return userName.toLowerCase();
    }

    private static class DirectoryBindInformation {
        private final String _bindDN;
        private Integer _port;
        private final String _password;

        public DirectoryBindInformation(String dn, String password, Integer port) {
            _bindDN = dn;
            _password = password;
            if (port == null || port == 0) {
                _port = LdapConstants.LDAP_PORT;
            } else {
                _port = port;
            }
        }

        public String getBindDN() {
            return _bindDN;
        }

        public String getPassword() {
            return _password;
        }

        public int getPort() {
            return _port;
        }
    }

    private static class ServiceContextProvider extends IServiceContextProvider {
        private static final String _correlationId = "da88e3e1-8ad6-423b-A022-e880d72198be";

        public ServiceContextProvider() {
        }

        @Override
        public IIdmServiceContext getServiceContext() {
            IIdmServiceContext ctxt = IdmServiceContextFactory
                    .getIdmServiceContext(_correlationId);
            return ctxt;
        }
    }
}
