/**
 *
 * Copyright 2011 VMware, Inc.  All rights reserved.
 */

/**
 * VMware Identity Service
 *
 * Native Adapter
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.pscsetup.interop;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;
import com.vmware.identity.configure.WinInstallerHelper;

import com.sun.jna.platform.win32.Advapi32Util;
import com.sun.jna.platform.win32.WinReg;

abstract class NativeAdapter {
	static {
		final String propName = "jna.library.path";

		final String LINUX_VMWARE_DEPLOY_PATH = "/opt/vmware/lib64";

		final String WIN_REG_DEPLOY_PATH = "SOFTWARE\\VMWare, Inc.\\VMware IC-Deploy";
		final String WIN_REG_INSTALL_KEY = "InstallPath";
		String WIN_VMWARE_DEPLOY_PATH = null;

		if (SystemUtils.IS_OS_WINDOWS) {
		    WIN_VMWARE_DEPLOY_PATH = WinInstallerHelper.readRegEdit(WIN_REG_DEPLOY_PATH, WIN_REG_INSTALL_KEY);
		}

		List<String> paths = null;
		if (SystemUtils.IS_OS_LINUX) {
			paths = Arrays.asList(LINUX_VMWARE_DEPLOY_PATH);
		} else if (SystemUtils.IS_OS_WINDOWS) {
			if (winRegistryValueExists(WIN_REG_DEPLOY_PATH, WIN_REG_INSTALL_KEY)) {
				WIN_VMWARE_DEPLOY_PATH = Advapi32Util.registryGetStringValue(
						WinReg.HKEY_LOCAL_MACHINE, WIN_REG_DEPLOY_PATH,
						WIN_REG_INSTALL_KEY);
			}

			paths = Arrays.asList(WIN_VMWARE_DEPLOY_PATH);
		} else {
			throw new IllegalStateException(
					"Only Windows and Linux platforms are supported");
		}

		// Check if the paths exist
		for (String pathString : paths) {
			Path path = Paths.get(pathString);
			if (Files.notExists(path)) {
				throw new IllegalStateException("Path \"" + pathString
						+ "\" does not exist");
			}
		}

		String propValue = System.getProperty(propName);

		StringBuilder jnalibpath = new StringBuilder(propValue == null ? ""
				: propValue);

		for (String path : paths) {
			File libDir = new File(path);

			if (libDir.exists() && libDir.isDirectory()) {
				if (jnalibpath.length() > 0) {
					jnalibpath.append(File.pathSeparator);
				}

				jnalibpath.append(path);
			}
		}

		propValue = jnalibpath.substring(0);

		if (!propValue.isEmpty()) {
			System.setProperty(propName, propValue);
		}
	}

	private static boolean winRegistryValueExists(String key, String value) {
		return Advapi32Util.registryKeyExists(WinReg.HKEY_LOCAL_MACHINE, key)
				&& Advapi32Util.registryValueExists(WinReg.HKEY_LOCAL_MACHINE,
						key, value);
	}
}
