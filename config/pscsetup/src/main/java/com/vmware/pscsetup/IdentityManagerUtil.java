/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.pscsetup;

import java.io.File;
import java.io.FileOutputStream;
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

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.certificate.VMCAClient;
import com.vmware.identity.idm.IIdmServiceContext;
import com.vmware.identity.idm.IdmServiceContextFactory;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.Tenant;
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
		saveCert(certs);
		saveSSoServerCertificates();

		log.info("Starting Identity Manager service");

		startService();

		log.info("Configuring Identity Manager service");

		configureService(certs);
	}

	private void saveSSoServerCertificates()
			throws IdentityManagerInstallerException {
		log.info("Starting saving sso server certificates");

		VMCAClient vmcaClient = new VMCAClient(username, domainName, password,
				"localhost");

		String certDirPath = InstallerUtils.getInstallerHelper()
				.getSSOCertPath();
		Path sslRoorCertX509Path = Paths.get(certDirPath,
				InstallerUtils.SSL_ROOT_CERT_X509_NAME);
		String leafCertKeyPath = InstallerUtils.joinPath(certDirPath,
				InstallerUtils.LEAF_KEY_PRIVATE_NAME);
		String leafCertPath = InstallerUtils.joinPath(certDirPath,
				InstallerUtils.LEAF_CERT_X509_NAME);
		String leafCertPkcsPath = InstallerUtils.joinPath(certDirPath,
				InstallerUtils.LEAF_CERT_PKCS_NAME);

		X509Certificate rootCert;
		KeyStore ks;
		OutputStream outputStream = null;
		try {
			rootCert = vmcaClient.getRootCertificate();
			saveCert(rootCert.getEncoded(), sslRoorCertX509Path);

			ks = KeyStore.getInstance("VKS");
			ks.load(new VecsLoadStoreParameter("MACHINE_SSL_CERT"));
			Certificate sslTrustAnchor = ks.getCertificate("__MACHINE_CERT");
			Key key = ks.getKey("__MACHINE_CERT", "changeme".toCharArray());

			saveCert(sslTrustAnchor.getEncoded(), Paths.get(leafCertPath));
			saveKey(key.getEncoded(), Paths.get(leafCertKeyPath));

			KeyStore outStore = KeyStore.getInstance("PKCS12");
			outStore.load(null);
			Certificate[] outChain = { sslTrustAnchor, rootCert };

			outStore.setKeyEntry("ssoserver", key,
					InstallerUtils.CERT_PASSWORD_SSL.toCharArray(), outChain);
			outputStream = new FileOutputStream(leafCertPkcsPath);
			outStore.store(outputStream,
					InstallerUtils.CERT_PASSWORD_SSL.toCharArray());

		} catch (KeyStoreException e) {
			log.error("Error getting trusted ssl certificate", e);
			throw new IdentityManagerInstallerException(
					"Error getting trusted ssl certificate", e);
		} catch (Exception e) {
			log.error("Error saving certificate", e);
			throw new IdentityManagerInstallerException(
					"Error saving certificate", e);
		} finally {
			if (outputStream != null)
				try {
					outputStream.close();
				} catch (IOException e) {
					log.error(e.getStackTrace().toString());
				}
		}
	}

	private void saveCert(byte[] cert, Path path)
			throws SecureTokenServerInstallerException {
		try {
			String privateKeyString = InstallerUtils.BEGIN_CERT
					+ InstallerUtils.getPemEncodedString(cert)
					+ InstallerUtils.END_CERT;
			InstallerUtils.writeToFile(path, privateKeyString);
		} catch (UnsupportedEncodingException e) {
			log.error("Failed to get encoded certificate", e);
			throw new SecureTokenServerInstallerException(
					"Failed to get encoded certificate", e);
		} catch (IOException e) {
			log.error("Failed to get encoded certificate", e);
			throw new SecureTokenServerInstallerException(
					"Failed to save certificate", e);
		}
	}

	private void saveKey(byte[] cert, Path path)
			throws SecureTokenServerInstallerException {
		try {
			String privateKeyString = InstallerUtils.BEGIN_PRIVATE_KEY
					+ InstallerUtils.getPemEncodedString(cert) + "\n"
					+ InstallerUtils.END_PRIVATE_KEY;
			InstallerUtils.writeToFile(path, privateKeyString);
		} catch (UnsupportedEncodingException e) {
			log.error("Failed to get encoded private key", e);
			throw new SecureTokenServerInstallerException(
					"Failed to get encoded private key", e);
		} catch (IOException e) {
			log.error("Failed to save private key", e);
			throw new SecureTokenServerInstallerException(
					"Failed to save private key", e);
		}
	}

	private void startService() throws IdentityManagerInstallerException {
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

		client.authenticate(tenantName, getAdminUser(), getAdminPassword());
	}

	private void saveCert(SigningCertificates certs)
			throws IdentityManagerInstallerException {

		String certsPath = InstallerUtils.getInstallerHelper().getSSOCertPath();
		try {
			InstallerUtils.createCertificateKeyDirectory(certsPath);
		} catch (IOException e) {
			throw new IdentityManagerInstallerException(
					"Failed to create certificate key directory: "
							+ e.getMessage(), e);
		}

		try {
			if (certs.getKey() == null) {
				return;
			}
			byte[] privBytes = certs.getKey().getEncoded();
			String privateKeyString = InstallerUtils.BEGIN_PRIVATE_KEY
					+ InstallerUtils.getPemEncodedString(privBytes)
					+ InstallerUtils.END_PRIVATE_KEY;

			Path path = Paths.get(certsPath, InstallerUtils.LEAF_SIGN_CERT_KEY);
			InstallerUtils.writeToFile(path, privateKeyString);
		} catch (UnsupportedEncodingException e) {
			log.error(e.toString());
			throw new IdentityManagerInstallerException(
					"Failed to get encoded private key", e);
		} catch (IOException e) {
			log.error(e.toString());
			throw new IdentityManagerInstallerException(
					"Failed to save private key", e);
		}
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
