/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.ProcessBuilder.Redirect;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.Key;
import java.security.KeyStore;
import java.security.cert.Certificate;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import com.vmware.identity.configure.IPlatformComponentInstaller;
import com.vmware.identity.configure.InstallerUtils;
import com.vmware.identity.configure.PlatformInstallComponent;

import com.vmware.provider.VecsLoadStoreParameter;

public class ReverseProxyInstaller implements IPlatformComponentInstaller {

	private static final String ID = "vmware-proxy-service";
	private static final String Name = "VMware Proxy Service";
	private static final String Description = "VMware Proxy Service";
	private static final Logger log = LoggerFactory
			.getLogger(ReverseProxyInstaller.class);

	@Override
	public void install() throws Exception {

		String keysPath = InstallerUtils.getInstallerHelper()
				.getReverseProxyPath();
		try {
			InstallerUtils.createCertificateKeyDirectory(keysPath);
		} catch (IOException e) {
			throw new ReverseProxyInstallerException(
					"Failed to create certificate key directory: "
							+ e.getMessage(), e);
		}

		KeyStore ks = KeyStore.getInstance("VKS");
		ks.load(new VecsLoadStoreParameter("MACHINE_SSL_CERT"));
		Certificate sslTrustAnchor = ks.getCertificate("__MACHINE_CERT");
		Key key = ks.getKey("__MACHINE_CERT", "changeme".toCharArray());

		saveCert(sslTrustAnchor.getEncoded(),
				Paths.get(keysPath, "machine-ssl-cert.pem"));
		saveKey(key.getEncoded(), Paths.get(keysPath, "machine-ssl-key.pem"));

		restartService();
	}

	private void restartService() throws ReverseProxyInstallerException {
		String[] svcCommand = InstallerUtils.getInstallerHelper()
				.getReverseProxyServiceCommand();
		ProcessBuilder pb = new ProcessBuilder(svcCommand);
		pb.redirectErrorStream(true);

		String logFile = InstallerUtils.getInstallerHelper()
				.getReverseProxyServiceLog();
		File log = new File(logFile);
		pb.redirectOutput(Redirect.appendTo(log));

		try {
			final Process p = pb.start();

			int exitCode = p.waitFor();

			if (exitCode != 0) {
				throw new ReverseProxyInstallerException(
						String.format(
								"Failed to start reverse proxy service [error code: %d]",
								exitCode), null);
			}

		} catch (InterruptedException ie) {
			throw new ReverseProxyInstallerException(
					"Failed to start reverse proxy service", ie);

		} catch (IOException e) {
			throw new ReverseProxyInstallerException(
					"Failed to start reverse proxy service", e);
		}
	}

	private void saveCert(byte[] cert, Path path)
			throws ReverseProxyInstallerException {
		try {
			String privateKeyString = InstallerUtils.BEGIN_CERT
					+ InstallerUtils.getPemEncodedString(cert)
					+ InstallerUtils.END_CERT;
			InstallerUtils.writeToFile(path, privateKeyString);
		} catch (UnsupportedEncodingException e) {
			log.error(e.toString());
			throw new ReverseProxyInstallerException(
					"Failed to get encoded certificate", e);
		} catch (IOException e) {
			log.error(e.toString());
			throw new ReverseProxyInstallerException(
					"Failed to save certificate", e);
		}
	}

	private void saveKey(byte[] cert, Path path)
			throws ReverseProxyInstallerException {
		try {
			String privateKeyString = InstallerUtils.BEGIN_PRIVATE_KEY
					+ InstallerUtils.getPemEncodedString(cert) + "\n"
					+ InstallerUtils.END_PRIVATE_KEY;
			InstallerUtils.writeToFile(path, privateKeyString);
		} catch (UnsupportedEncodingException e) {
			log.error(e.toString());
			throw new ReverseProxyInstallerException(
					"Failed to get encoded private key", e);
		} catch (IOException e) {
			log.error(e.toString());
			throw new ReverseProxyInstallerException(
					"Failed to save private key", e);
		}
	}

	@Override
	public void upgrade() throws Exception {
		// TODO Auto-generated method stub

	}

	@Override
	public void uninstall() {
		// TODO Auto-generated method stub

	}

    @Override
    public void migrate() {

    }

	@Override
	public PlatformInstallComponent getComponentInfo() {
		return new PlatformInstallComponent(ID, Name, Description);
	}
}
