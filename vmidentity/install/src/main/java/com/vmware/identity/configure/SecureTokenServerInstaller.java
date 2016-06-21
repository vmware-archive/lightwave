/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
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

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.lang.SystemUtils;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
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

        Thread.sleep(20000);
        configureSTS();

        String tcSTSBase = "";
        Path tomcatTempDir = Paths.get(tcSTSBase, "temp");
        if (!Files.exists(tomcatTempDir)) {
            Files.createDirectories(tomcatTempDir);
        }
        // Only on Windows the STS service has to be installed as a service
        installInstAsWinService();

        startSTSService();
        configureInfraNodeHomePage();
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

            String wrapper_bin = new WinInstallerHelper().getWrapperBinPath();
            String wrapper_exe = InstallerUtils.joinPath(wrapper_bin, "wrapper.exe");
            String wrapper_conf = InstallerUtils.joinPath(InstallerUtils
                        .getInstallerHelper().getTCBase(), "conf", "wrapper.conf");

            // Install STS instance as windows service using Service controller
            String sc_binPath = wrapper_exe + " -s " +  wrapper_conf ;
            String command = "sc.exe create VMwareSTS type= own start= auto error= normal binPath= "+sc_binPath+ " depend= VMwareIdentityMgmtService displayname= \"VMware Secure Token Service\" ";

            int exitCode = -1;
            try {
                Process p = Runtime.getRuntime().exec(command);
                exitCode = p.waitFor();
            } catch (IOException | InterruptedException e) {
                throw new SecureTokenServerInstallerException(
                        "Failed to install STS instance as Win service", e);
            }

            if (exitCode != 0) {
                throw new SecureTokenServerInstallerException(String.format(
                        "Failed to install STS service [error code: %d]",
                        exitCode), null);
            }

            // update the description
            exitCode = -1;
            String desc = "VMware Single Sign-On STS Service";
            command = "sc.exe description VMwareSTS "+desc ;
            try {
                Process p = Runtime.getRuntime().exec(command);
                exitCode = p.waitFor();
            } catch (IOException | InterruptedException e) {
                throw new SecureTokenServerInstallerException(
                        "Failed to set VMwareSTS service description", e);
            }

            if (exitCode != 0) {
                throw new SecureTokenServerInstallerException(
                        String.format(
                                "Failed to set VMwareSTS service description [error code: %d]",
                                exitCode), null);
            }

            // Set recovery options
            exitCode = -1;
            command = "sc failure VMwareSTS reset= 86400 actions= restart/30000/restart/60000/restart/90000";
            try {
                Process p = Runtime.getRuntime().exec(command);
                exitCode = p.waitFor();
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

    @Override
    public void upgrade() {
        // TODO Auto-generated method stub

    }

    @Override
    public void uninstall() {
        // TODO Auto-generated method stub

    }

    private void configureSTS() throws STSInstallerException {
        if (SystemUtils.IS_OS_LINUX) {
            String configStsPath = InstallerUtils.getInstallerHelper()
                    .getConfigureStsPath();
            Path configurePath = Paths.get(configStsPath);
            try {
                String configFileName = InstallerUtils.getInstallerHelper()
                        .getConfigureStsFileName();
                InputStream link = getClass().getClassLoader().getResourceAsStream(
                        configFileName);// "configure-sts.sh"));

                if (link != null && configurePath != null) {
                    try {
                        Files.copy(link, configurePath);
                    } catch (IOException e) {
                        log.error(e.toString());
                        throw new STSInstallerException("Failed to extract "
                                + configFileName, e);
                    }

                    InstallerUtils.getInstallerHelper().setPermissions(
                            configurePath);

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
    }

    @Override
    public PlatformInstallComponent getComponentInfo() {
        return new PlatformInstallComponent(ID, Name, Description);
    }
}
