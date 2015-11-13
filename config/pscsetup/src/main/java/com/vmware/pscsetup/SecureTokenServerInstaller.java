/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.pscsetup;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.lang.ProcessBuilder.Redirect;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.SystemUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SecureTokenServerInstaller implements IPlatformComponentInstaller {

	private static final String ID = "vmware-secure-token-service";
	private static final String Name = "VMware Secure Token Service";
	private static final String Description = "VMware Secure Token Service";
	private static final Logger log = LoggerFactory
			.getLogger(SecureTokenServerInstaller.class);

	private String hostnameURL = null;

	@Override
	public void install() throws Exception {
		initialize();

		log.info("Configuring STS");
		//configureSTS();

		/*String tcSTSBase = "";
		Path tomcatTempDir = Paths.get(tcSTSBase, "temp");
		if (!Files.exists(tomcatTempDir)) {
			Files.createDirectories(tomcatTempDir);
		}

		String javaMaxMemory = "160m";
		String javaMaxPerm = "160m";
		String ssoLogDir = InstallerUtils.getInstallerHelper().getLogPaths();
		String gcLogFileName = InstallerUtils.getInstallerHelper()
				.getGcLogFile();
		String gcLogFilePath = InstallerUtils
				.joinPath(ssoLogDir, gcLogFileName);

		String jvmStsErrorFile = InstallerUtils.joinPath(ssoLogDir,
				"hs_err_sts_pid%p.log");
		String[] javaOptionsSetEnv = { "-Djdk.map.althashing.threshold=512",
				"-Xss228K", "-Xmx" + javaMaxMemory,
				"-XX:MaxPermSize=" + javaMaxPerm,
				"-XX:+HeapDumpOnOutOfMemoryError",
				"-XX:HeapDumpPath=" + ssoLogDir, "-XX:+PrintGCDetails",
				"-XX:+PrintGCDateStamps", "-XX:+PrintTenuringDistribution",
				"-Xloggc:" + gcLogFilePath, "-XX:+UseGCLogFileRotation",
				"-XX:NumberOfGCLogFiles=2", "-XX:GCLogFileSize=5M",
				"-XX:ErrorFile=" + jvmStsErrorFile,
				"-XX:HeapDumpPath=" + ssoLogDir };

		String[] javaOptionsWrapperConf = {
				"-Djava.endorsed.dirs=%CATALINA_HOME%\\common\\endorsed",
				"-Dcatalina.base=%CATALINA_BASE%",
				"-Dcatalina.home=%CATALINA_HOME%",
				"-Djava.io.tmpdir=%CATALINA_BASE%\temp",
				"-Djava.util.logging.manager=com.springsource.tcserver.serviceability.logging.TcServerLogManager",
				"-Djava.util.logging.config.file=%CATALINA_BASE%\\conf\\logging.properties",
				"-Dwrapper.dump.port=-1" };

		configTCSetenv(javaOptionsSetEnv);

		// Windows only wrapper configuration for tcServer
		configWrapperConf(javaOptionsWrapperConf, javaOptionsSetEnv);
*/
		copyCertForTC();

		// Only on Windows the STS service has to be installed as a service
		installInstAsWinService();

		startSTSService();
		// configureInfraNodeHomePage();
	}

	private void initialize() {
		if (hostnameURL == null) {
			String hostnameURL = VmAfClientUtil.getHostnameURL();
			this.hostnameURL = hostnameURL;
		}
	}

	private void configureInfraNodeHomePage()
			throws SecureTokenServerInstallerException {
		// TODO: Do configuration specific for install type. This does the
		// configuration for infrastructure node

		String webappDir = InstallerUtils.joinPath(InstallerUtils
				.getInstallerHelper().getTCBase(), "webapps");
		log.info("Configure ROOT index.html on infrastructure node");

		String rootPagePath = InstallerUtils.joinPath(webappDir, "ROOT",
				"index.html");
		try (BufferedWriter writer = new BufferedWriter(new FileWriter(
				rootPagePath))) {

			writer.append("<html>");
			writer.append(System.lineSeparator());
			writer.append("<head>");
			writer.append(System.lineSeparator());
			writer.append(String
					.format("<meta http-equiv=\"refresh\" content=\"0;URL=http://%s/websso/\">%s",
							hostnameURL, System.lineSeparator()));
			writer.append("</head>");
			writer.append(System.lineSeparator());
			writer.append("<body> </body>");
			writer.append(System.lineSeparator());
			writer.append("</html>");
			writer.append(System.lineSeparator());

		} catch (IOException e) {
			log.error("Failed to configure InfraNodeHomePage", e);
			throw new SecureTokenServerInstallerException(
					"Failed to configure InfraNodeHomePage", e);
		}
	}

	private void startSTSService() throws SecureTokenServerInstallerException {

		ProcessBuilder pb = new ProcessBuilder(InstallerUtils
				.getInstallerHelper().getSTSServiceStartCommand());
		pb.redirectErrorStream(true);

		String logFile = InstallerUtils.getInstallerHelper()
				.getIDMServiceLogFile();
		File log = new File(logFile);
		pb.redirectOutput(Redirect.appendTo(log));

		int exitCode = -1;
		try {
			final Process p = pb.start();
			exitCode = p.waitFor();
		} catch (IOException | InterruptedException e) {
			throw new SecureTokenServerInstallerException(
					"Failed to start STS service", e);
		}

		if (exitCode != 0) {
			throw new SecureTokenServerInstallerException(String.format(
					"Failed to start STS service [error code: %d]", exitCode),
					null);
		}

	}

	private void installInstAsWinService()
			throws SecureTokenServerInstallerException {
		if (SystemUtils.IS_OS_WINDOWS) {

			// TODO: Actually the way we install a TC instance may need be
			// revisited as part of the install/upgrade work,
			// In 2015, we have to do some manual copy in order to satisfy
			// permission requirements set by installer team.
			// in 5.5 we install our own version of TC instance, and we have a
			// better control of our instance (if I remembered right),
			// we may want to change back to that model.
			moveTanukiWrapperFiles();

			String wrapperBin = InstallerUtils.joinPath(InstallerUtils
					.getInstallerHelper().getVMIdentityInstallPath(),
					"wrapper", "bin");
			String command = InstallerUtils.joinPath(wrapperBin,
					"InstallApp-NT.bat");

			int exitCode = -1;
			try {
				exitCode = runCommand(command, command + ".log", InstallerUtils
						.getInstallerHelper().getTCBase());
			} catch (IOException | InterruptedException e) {
				throw new SecureTokenServerInstallerException(
						"Failed to install STS service", e);
			}

			if (exitCode != 0) {
				throw new SecureTokenServerInstallerException(String.format(
						"Failed to install STS service [error code: %d]",
						exitCode), null);
			}

			exitCode = -1;
			command = "sc.exe failure VMwareSTS reset= 86400 actions= restart/30000/restart/60000/restart/90000";
			try {
				exitCode = runCommand(command, "VMwareSTSReset.log", null);
			} catch (IOException | InterruptedException e) {
				throw new SecureTokenServerInstallerException(
						"Failed to set VMwareSTS service recovery options", e);
			}

			if (exitCode != 0) {
				throw new SecureTokenServerInstallerException(
						String.format(
								"Failed to set VMwareSTS service recovery options [error code: %d]",
								exitCode), null);
			}
		}

	}

	private int runCommand(String command, String logFile,
			String workingDirectory) throws InterruptedException, IOException {
		ProcessBuilder pb = new ProcessBuilder(command);
		if (workingDirectory != null)
			pb.directory(new File(workingDirectory));
		pb.redirectErrorStream(true);

		File log = new File(logFile);
		pb.redirectOutput(Redirect.appendTo(log));

		int exitCode = -1;

		final Process p = pb.start();
		exitCode = p.waitFor();

		return exitCode;
	}

	private void moveTanukiWrapperFiles()
			throws SecureTokenServerInstallerException {

		String wrapperRoot = InstallerUtils.joinPath(InstallerUtils
				.getInstallerHelper().getVMIdentityInstallPath(), "wrapper");
		Path wrapperBin = Paths.get(wrapperRoot, "bin");
		Path wrapperConf = Paths.get(wrapperRoot, "conf");
		Path wrapperLib = Paths.get(wrapperRoot, "lib");

		try {
			Files.createDirectories(Paths.get(wrapperRoot));

			Files.createDirectories(wrapperBin);
			Files.createDirectories(wrapperConf);
			Files.createDirectories(wrapperLib);

			String tcBase = InstallerUtils.getInstallerHelper().getTCBase();
			String wrapperArchPath = InstallerUtils.joinPath(tcBase, "conf",
					"wrapper.arch");
			String arch = "";

			try {
				List<String> contents = Files.readAllLines(
						Paths.get(wrapperArchPath), StandardCharsets.UTF_8);
				if (contents.size() > 0)
					arch = contents.get(0);
			} catch (IOException e) {
				log.error(e.getStackTrace().toString());
				throw new SecureTokenServerInstallerException(
						"Cannot read wrapper.arch", e);
			}
			// move bin and lib files
			String tanukisrc = InstallerUtils.joinPath(tcBase, "bin", arch);

			Files.move(Paths.get(tanukisrc, "wrapper.exe"),
					Paths.get(wrapperBin.toString(), "wrapper.exe"));
			Files.move(Paths.get(tanukisrc, "wrapper.dll"),
					Paths.get(wrapperBin.toString(), "wrapper.dll"));

			Files.move(Paths.get(tanukisrc, "wrapper.jar"),
					Paths.get(wrapperLib.toString(), "wrapper.jar"));
			Files.move(Paths.get(tanukisrc, "threaddumpwrapper.jar"),
					Paths.get(wrapperLib.toString(), "threaddumpwrapper.jar"));

			// move config files
			Files.move(Paths.get(tcBase, "conf", "wrapper.conf"),
					Paths.get(wrapperConf.toString(), "wrapper.conf"));
			Files.move(Paths.get(tcBase, "conf", "wrapper.arch"),
					Paths.get(wrapperConf.toString(), "wrapper.arch"));
		} catch (IOException e) {
			log.error("Failed to move tanuki wrapper files");
			throw new SecureTokenServerInstallerException(
					"Failed to move tanuki wrapper files", e);
		}

	}

	private void copyCertForTC() throws SecureTokenServerInstallerException {
		Path certDirPath = Paths.get(InstallerUtils.getInstallerHelper()
				.getTCBase(), "conf", InstallerUtils.LEAF_CERT_PKCS_NAME);
		Path certSourcePath = Paths.get(InstallerUtils.getInstallerHelper()
				.getSSOCertPath(), InstallerUtils.LEAF_CERT_PKCS_NAME);

		try {
			Files.copy(certSourcePath, certDirPath);
		} catch (IOException e) {
			log.error("Failed to copy pkcs certificate");
			throw new SecureTokenServerInstallerException(
					"Failed to copy pkcs certificate", e);
		}

	}

	private void configWrapperConf(String[] wrapperConf, String[] javaOptions)
			throws SecureTokenServerInstallerException {

		if (SystemUtils.IS_OS_WINDOWS) {
			String wrapperName = InstallerUtils.joinPath(InstallerUtils
					.getInstallerHelper().getTCBase(), "conf", "wrapper.conf");
			String wrapperLib = InstallerUtils.joinPath(InstallerUtils
					.getInstallerHelper().getVMIdentityInstallPath(),
					"wrapper", "lib");
			BufferedReader reader = null;
			BufferedWriter writer = null;

			String lineSeparator = System.lineSeparator();
			try {
				reader = new BufferedReader(new FileReader(wrapperName));
				writer = new BufferedWriter(
						new FileWriter(wrapperName + ".tmp"));
				String line;
				boolean foundJavaOptions = false;

				while ((line = reader.readLine()) != null) {
					boolean omitString = false;

					if (line.equalsIgnoreCase("set.CATALINA_HOME=..\\..")) {
						writer.append("set.CATALINA_HOME=%VMWARE_TOMCAT%");

						omitString = true;
					}

					if (line.equalsIgnoreCase("set.CATALINA_BASE=..\\..")) {
						writer.append("set.CATALINA_BASE=");
						writer.append(InstallerUtils.getInstallerHelper()
								.getTCBase());
						omitString = true;
					}

					if (line.contains("wrapper.java.classpath.1=%CATALINA_BASE%\\bin")
							&& line.contains("wrapper.jar")) {
						writer.append("wrapper.java.classpath.1=");
						writer.append(wrapperLib);
						writer.append("\\wrapper.jar");
						omitString = true;
					}

					if (line.contains("wrapper.java.classpath.2=%CATALINA_BASE%\\bin")
							&& line.contains("threaddumpwrapper.jar")) {
						writer.append("wrapper.java.classpath.2=");
						writer.append(wrapperLib);
						writer.append("\\threaddumpwrapper.jar");
						omitString = true;
					}
					if (line.contains("wrapper.java.library.path.1=%CATALINA_BASE%\\bin")) {
						omitString = true;
					}

					if (line.contains("wrapper.ntservice.dependency.1=")) {
						writer.append("wrapper.ntservice.dependency.1=VMwareIdentityMgmtService");
						omitString = true;
					}
					if (line.contains("wrapper.ntservice.name=")) {
						writer.append("wrapper.ntservice.name=VMwareSTS\n");
						omitString = true;
					}
					if (line.contains("wrapper.ntservice.displayname=")) {
						writer.append("wrapper.ntservice.displayname=VMware Security Token Service");
						omitString = true;
					}
					if (line.contains("wrapper.ntservice.description=")) {
						writer.append("wrapper.ntservice.description=VMware Single Sign-On STS Service");
						omitString = true;
					}

					// Comment out the above three properties
					if (line.contains("wrapper.cpu.timeout=")
							|| line.contains("wrapper.shutdown.timeout=")
							|| line.contains("wrapper.jvm_exit.timeout=")) {
						writer.append("#");
						writer.append(line);
						omitString = true;
					}

					boolean javaOptionMatch = line
							.contains("wrapper.java.additional");
					if (javaOptionMatch) {
						omitString = true;
						foundJavaOptions = true;
					}

					if (!javaOptionMatch && foundJavaOptions) {
						// I've seen the last java option, now append
						foundJavaOptions = false;
						int javaOptionIndex = 1;
						for (String option : wrapperConf) {
							writer.append(String.format(
									"wrapper.java.additional.%d=\"%s\"%s",
									javaOptionIndex, option, lineSeparator));
							javaOptionIndex += 1;
						}
						for (String option : javaOptions) {
							writer.append(String.format(
									"wrapper.java.additional.%d=\"%s\"%s",
									javaOptionIndex, option, lineSeparator));
							javaOptionIndex += 1;
						}
					}

					if (!omitString)
						writer.append(line);

					writer.append(lineSeparator);

				}

				// Add new Java service properties
				writer.append(lineSeparator);
				writer.append("# Timeout for how long Tanuki will wait to restart a frozen JVM"
						+ lineSeparator);
				writer.append("wrapper.ping.timeout=120" + lineSeparator);
				writer.append("wrapper.ping.interval.logged=300"
						+ lineSeparator);
				writer.append("wrapper.restart.delay=120" + lineSeparator);
				writer.append("wrapper.restart.reload_configuration=TRUE"
						+ lineSeparator);
				writer.append(lineSeparator);

				writer.append("# restart on abnormal exit" + lineSeparator);
				writer.append("wrapper.on_exit.default=RESTART" + lineSeparator);
				writer.append("wrapper.on_exit.0=SHUTDOWN" + lineSeparator);
				writer.append(lineSeparator);

				writer.append("# restart trigger on stuck transactions"
						+ lineSeparator);
				writer.append("wrapper.filter.trigger.1=SESSION_THRESHOLD_ERROR"
						+ lineSeparator);
				writer.append("wrapper.filter.action.1=RESTART" + lineSeparator);
				writer.append(lineSeparator);

				writer.append("# restart attempts" + lineSeparator);
				writer.append("wrapper.max_failed_invocations=20"
						+ lineSeparator);
				writer.append(lineSeparator);

				writer.append("# Windows service restart" + lineSeparator);
				writer.append("wrapper.ntservice.recovery.reset=86400"
						+ lineSeparator);
				writer.append("wrapper.ntservice.recovery.1.failure=RESTART"
						+ lineSeparator);
				writer.append("wrapper.ntservice.recovery.2.failure=RESTART"
						+ lineSeparator);
				writer.append("wrapper.ntservice.recovery.3.failure=RESTART"
						+ lineSeparator);
				writer.append("wrapper.ntservice.recovery.1.delay=30"
						+ lineSeparator);
				writer.append("wrapper.ntservice.recovery.2.delay=60"
						+ lineSeparator);
				writer.append("wrapper.ntservice.recovery.3.delay=90"
						+ lineSeparator);

			} catch (IOException e) {
				log.error("Failed to config wrapper.conf");
				throw new SecureTokenServerInstallerException(
						"Failed to config wrapper.conf", e);

			} finally {
				try {
					if (reader != null)

						reader.close();

					if (writer != null)
						writer.close();
				} catch (IOException e) {
					log.info("Failed to close wrapper.conf");
				}
			}

			try {
				Files.move(Paths.get(wrapperName + ".tmp"),
						Paths.get(wrapperName),
						StandardCopyOption.REPLACE_EXISTING);
			} catch (IOException e) {
				log.error("Failed to save wrapper.conf");
				throw new SecureTokenServerInstallerException(
						"Failed to save wrapper.conf", e);
			}
		}
	}

	private void configTCSetenv(String[] javaOptionsSetEnv)
			throws SecureTokenServerInstallerException {
		String setenvName = InstallerUtils.getInstallerHelper()
				.getTCSetenvName();
		String setenvPath = InstallerUtils.joinPath(InstallerUtils
				.getInstallerHelper().getTCBase(), "bin", setenvName);
		String envData = readFile(setenvPath);

		StringBuilder javaOptions = new StringBuilder();
		for (String opt : javaOptionsSetEnv) {
			javaOptions.append(opt);
			javaOptions.append(" ");
		}

		String replacementSetEnv = InstallerUtils.getInstallerHelper()
				.getSetEnvReplacement();

		// set options in setenv script file
		envData = Pattern
				.compile("JVM_OPTS=.*")
				.matcher(envData)
				.replaceAll(
						Matcher.quoteReplacement(replacementSetEnv
								+ "JVM_OPTS=\"" + javaOptions.toString() + "\""));

		envData = Pattern
				.compile("JAVA_OPTS=")
				.matcher(envData)
				.replaceAll(
						replacementSetEnv + "CATALINA_OPTS=-ea"
								+ System.lineSeparator() + replacementSetEnv
								+ "JAVA_OPTS=");

		writeToFile(setenvPath, envData);

	}

	private void writeToFile(String setenvPath, String content)
			throws SecureTokenServerInstallerException {
		Writer writer = null;

		try {
			BufferedOutputStream outputStream = new BufferedOutputStream(
					new FileOutputStream(setenvPath));
			writer = new OutputStreamWriter(outputStream);
			writer.write(content);
		} catch (Exception e) {
			log.error(e.getStackTrace().toString());
			throw new SecureTokenServerInstallerException(
					"cannot write to setenv file", e);
		} finally {
			if (writer != null)
				try {
					writer.close();
				} catch (IOException e) {
					log.error(e.getStackTrace().toString());
				}
		}
	}

	private String readFile(String setenvPath)
			throws SecureTokenServerInstallerException {
		try {
			String contents = new String(Files.readAllBytes(Paths
					.get(setenvPath)), StandardCharsets.UTF_8);
			return contents;
		} catch (IOException e) {
			log.error(e.getStackTrace().toString());
			throw new SecureTokenServerInstallerException(
					"Cannot write to setenv file", e);
		}
	}

	@Override
	public void upgrade() {
		// TODO Auto-generated method stub

	}

	@Override
	public void uninstall() {
		// TODO Auto-generated method stub

	}

	private void configureSTS() throws STSInstallerException {
		String configStsPath = InstallerUtils.getInstallerHelper()
				.getConfigureStsPath();
		Path configurePath = Paths.get(configStsPath);
		try {
			String configFileName = InstallerUtils.getInstallerHelper()
					.getConfigureStsFileName();
			InputStream link = getClass().getClassLoader().getResourceAsStream(
					configFileName);// "configure-sts.sh"));
			try {
				Files.copy(link, configurePath);
			} catch (IOException e) {
				log.error(e.toString());
				throw new STSInstallerException("Failed to extract "
						+ configFileName, e);
			}

			InstallerUtils.getInstallerHelper().setPermissions(configurePath);

			ProcessBuilder pb = new ProcessBuilder(configurePath.toString());
			pb.redirectErrorStream(true);

			File log = new File(configStsPath + ".log");
			pb.redirectOutput(Redirect.appendTo(log));

			final Process p = pb.start();

			int exitCode = p.waitFor();

			if (exitCode != 0) {
				throw new STSInstallerException(String.format(
						"Failed to run %s, errorcode %s", configStsPath,
						exitCode), null);
			}
		} catch (InterruptedException | IOException e) {
			throw new STSInstallerException("Failed to run " + configStsPath, e);
		} finally {
			try {
				Files.deleteIfExists(configurePath);
			} catch (IOException e) {
				log.debug("Failed to delete %s", configurePath);
			}
		}
	}

	@Override
	public PlatformInstallComponent getComponentInfo() {
		return new PlatformInstallComponent(ID, Name, Description);
	}
}
