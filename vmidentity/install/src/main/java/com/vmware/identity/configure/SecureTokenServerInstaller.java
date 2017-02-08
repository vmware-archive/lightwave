/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
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
import org.w3c.dom.Element;
import org.xml.sax.SAXException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SecureTokenServerInstaller implements IPlatformComponentInstaller {

    private static final String ID = "vmware-secure-token-service";
    private static final String Name = "VMware Secure Token Service";
    private static final String Description = "VMware Secure Token Service";

    private static final String SSL_ENABLED_ATTR= "SSLEnabled";
    private static final String SSL_ENABLED_PROTOCOL_ATTR="sslEnabledProtocols";
    private static final String SSL_IMPLEMENTATION_NAME_ATTR="sslImplementationName";
    private static final String STORE_ATTR="store";
    private static final String KEYALIAS_ATTR="keyAlias";
    private static final String KEYSTORETYPE_ATTR="keystoreType";

    private static final Logger log = LoggerFactory
            .getLogger(SecureTokenServerInstaller.class);

    private String hostnameURL = null;
    private VmIdentityParams params = null;
    private String sslEnabledProtocols = "";
    private String storename = "";
    private String sslImplementationName="";
    private String keyAlias ="";
    private String keyStoreType ="";


    public SecureTokenServerInstaller(VmIdentityParams installParams) {
        params = installParams;
    }

    @Override
    public void install() throws Exception {
        initialize();

        log.info("Configuring STS");
        configureSTS();

        // Only on Windows the STS service has to be installed as a service
        installInstAsWinService();

        startSTSService();
        String portNumber = HostnameReader.readPortNumber();
        if (portNumber == null) {
            throw new SecureTokenServerInstallerException("Invalid port number" , null);
        }

        new STSHealthChecker(hostnameURL, HostnameReader.readPortNumber()).checkHealth();
    }

    private void initialize() {
        if (hostnameURL == null) {
            String hostnameURL = VmAfClientUtil.getHostnameURL();
            this.hostnameURL = hostnameURL;
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

            createJavaSecurityFile();
            String wrapper_bin = new WinInstallerHelper().getWrapperBinPath();
            String wrapper_exe = InstallerUtils.joinPath(wrapper_bin, "wrapper.exe");
            String wrapper_conf = InstallerUtils.joinPath(InstallerUtils
                        .getInstallerHelper().getTCBase(), "conf", "wrapper.conf");

            // Install STS instance as windows service using Service controller
            String sc_binPath = "\"" + wrapper_exe + " -s " +  wrapper_conf +"\"";
            String command = "sc.exe create VMwareSTS  type= own start= auto error= normal binPath= "+sc_binPath+ " depend= VMwareIdentityMgmtService displayname= \"VMware Secure Token Service\" ";
            System.out.println(command);
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
        log.debug("SecureTokenServerInstaller : Upgrade");
        mergeServerXMl();

    }

    @Override
    public void uninstall() {
        // TODO Auto-generated method stub

    }

    @Override
    public void migrate() {

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
    private void mergeServerXMl() {

        getServerAttributes();
        setServerAttributes();

    }

    private void getServerAttributes() {

        File serverXmlFile = new File(InstallerUtils.joinPath(params.getBackupDir() , "server.xml"));
        try {
            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            Document doc = builder.parse(serverXmlFile);
            NodeList connectorList = doc.getElementsByTagName("Connector");
            for (int i = 0 ; i < connectorList.getLength(); i ++) {

                Node node = connectorList.item(i);
                Element element = (Element) node;
                boolean isSSLEnabled = element.hasAttribute(SSL_ENABLED_ATTR);
                if (isSSLEnabled) {
                    sslEnabledProtocols = element.getAttribute(SSL_ENABLED_PROTOCOL_ATTR);
                    log.debug("SecureTokenServerInstaller : getServerAttributes sslEnabledProtocols: %s",sslEnabledProtocols);
                    storename  = element.getAttribute(STORE_ATTR);
                    log.debug("SecureTokenServerInstaller : getServerAttributes storename: %s", storename);
                    sslImplementationName = element.getAttribute(SSL_IMPLEMENTATION_NAME_ATTR);
                    log.debug("SecureTokenServerInstaller : getServerAttributes sslImplementationName: %s",sslImplementationName);
                    keyAlias  = element.getAttribute(KEYALIAS_ATTR);
                    log.debug("SecureTokenServerInstaller : getServerAttributes keyAlias:" +keyAlias);
                    keyStoreType  = element.getAttribute(KEYSTORETYPE_ATTR);
                    log.debug("SecureTokenServerInstaller : getServerAttributes keyStoreType:" +keyStoreType);

                } else {
                    continue;
                }

            }
            log.debug("SecureTokenServerInstaller : getServerAttributes - Completed");
        } catch (Exception ex) {
            log.error("SecureTokenServerInstaller : getServerAttributes - failed");
        }

    }

    private void setServerAttributes() {

	try {
        String filePath = InstallerUtils.joinPath( InstallerUtils.getInstallerHelper().getTCBase(),
                            "conf","server.xml");
            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            Document doc = builder.parse(new File(filePath));
            NodeList connectorList = doc.getElementsByTagName("Connector");
            for (int i = 0; i < connectorList.getLength(); i++) {

                Node node = connectorList.item(i);
                Element element = (Element) node;
                boolean isSSLEnabled = element.hasAttribute(SSL_ENABLED_ATTR);
                if (isSSLEnabled) {
                    element.setAttribute(SSL_ENABLED_PROTOCOL_ATTR, sslEnabledProtocols);
                    element.setAttribute(STORE_ATTR,storename);
                    element.setAttribute(SSL_IMPLEMENTATION_NAME_ATTR,sslImplementationName);
                    element.setAttribute(KEYALIAS_ATTR,keyAlias);
                    element.setAttribute(KEYSTORETYPE_ATTR,keyStoreType);

                } else {
                    continue;
                }
            }

            Transformer transformer = TransformerFactory.newInstance().newTransformer();
            transformer.setOutputProperty(OutputKeys.INDENT, "yes");
            transformer.setOutputProperty(OutputKeys.METHOD, "xml");
            transformer.setOutputProperty(
                    "{http://xml.apache.org/xslt}indent-amount", "4");
            DOMSource source = new DOMSource(doc);
            StreamResult result = new StreamResult (new File(filePath));
            transformer.transform(source, result);
            log.debug("SecureTokenServerInstaller : setServerAttributes - Completed");
        } catch (Exception ex) {
           log.error("SecureTokenServerInstaller : setServerAttributes - failed");
        }

    }
    private void createJavaSecurityFile() {
        try {
        File  securityFile = new  File(InstallerUtils.joinPath(System.getenv("VMWARE_CFG_DIR"), "java","vmware-override-java.security"));
        File vmIdentitySecurityFile = new File (InstallerUtils.joinPath(InstallerUtils
                        .getInstallerHelper().getTCBase(), "conf","vmware-identity-override-java.security"));
        Files.copy(securityFile.toPath(), vmIdentitySecurityFile.toPath());
        } catch (Exception ex ) {
            System.out.println("Failedin to create VMIdentity java security File");
        }

    }
    @Override
    public PlatformInstallComponent getComponentInfo() {
        return new PlatformInstallComponent(ID, Name, Description);
    }
}
