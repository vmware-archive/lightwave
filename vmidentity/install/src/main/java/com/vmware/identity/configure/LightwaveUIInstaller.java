/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;
import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
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

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

public class LightwaveUIInstaller implements IPlatformComponentInstaller {

    private static final String ID = "lightwave-ui";
    private static final String Name = "Lightwave UI";
    private static final String Description = "Lightwave UI OIDC Client regstiration";
    private VmIdentityParams params;

    public LightwaveUIInstaller() {
        params = null;
    }
    public LightwaveUIInstaller(String hostname, String domain, String username, String password ) {
        params = new VmIdentityParams();
        params.setHostname(hostname);
        params.setDomainName(domain);
        params.setUsername(username);
        params.setPassword(password);
    }
    public LightwaveUIInstaller(VmIdentityParams installParams) {
        params = installParams;
    }

    @Override
    public void install() throws Exception {

        String servername = params.getHostname();
        String subjectAltName = params.getSubjectAltName();
        String domain = params.getDomainName();
        String username = params.getUsername();
        String password = params.getPassword();
        System.out.println("Configuring Lightwave UI for domain : " + domain);
        registerOidcClientForLightwaveUI(servername, subjectAltName, domain, username, password);
        configureLandingPage();
    }

    private TrustManager[] trustAllCerts = new TrustManager[]{
            new X509TrustManager() {
                public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                    return null;
                }
                public void checkClientTrusted(
                    java.security.cert.X509Certificate[] certs, String authType) {
                }
                public void checkServerTrusted(
                    java.security.cert.X509Certificate[] certs, String authType) {
                }
            }
        };

    private String authenticate(String hostname, String domain, String username, String password){
        String openIdConnectUri = "https://" + hostname + "/openidconnect/token/" + domain;
        String data = "grant_type=password&username=" + username + "@" + domain + "&password=" + password + "&" +
                      "scope=openid+offline_access+id_groups+at_groups+rs_admin_server";
        String response = doPost(openIdConnectUri, data, null);

        if(response != null){
            String[] parts = response.split(",");
            if(parts != null && parts.length > 1){
                return parts [0].split(":")[1].replace("\"","");
            }
        }
        return response;
    }

    private String registerOidc(String hostname, String subjectAltName, String domain, String token){
        String oidcClientUri = "https://" + hostname + "/idm/tenant/" + domain + "/oidcclient";
        String pnid = VmAfClientUtil.getHostname();

        String redirectUriPattern = "\"https://%s/lightwaveui/Home\"";
        String logoutUriPattern = "\"https://%s/lightwaveui\"";

        String redirectUris = "\"redirectUris\": [" + String.format(redirectUriPattern, pnid);
        String postLogoutRedirectUris = "\"postLogoutRedirectUris\": [" + String.format(logoutUriPattern, pnid);

        if (subjectAltName != null && !subjectAltName.equalsIgnoreCase(pnid)) {
               redirectUris += ", " + String.format(redirectUriPattern, subjectAltName);
               postLogoutRedirectUris += ", " + String.format(logoutUriPattern, subjectAltName);
        }

        String data = "{ " + redirectUris + "]," +
              "\"tokenEndpointAuthMethod\": \"none\"," +
              postLogoutRedirectUris + "]," +
              "\"logoutUri\": " + String.format(logoutUriPattern, pnid) +
              " }";
        String response = doPost(oidcClientUri, data, token);
        return response;
    }

    private String doPost(String uri, String data, String token){

        String result = "";
        try {

            SSLContext sc = SSLContext.getInstance("SSL");
            sc.init(null, trustAllCerts, new java.security.SecureRandom());
            HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());

            URL url = new URL(uri);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setDoOutput(true);
            conn.setRequestMethod("POST");
            if(token != null){
                conn.setRequestProperty("Content-Type", "application/json");
                conn.setRequestProperty("Authorization", "Bearer " + token);
            }
            else
            {
                conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
            }
            OutputStream os = conn.getOutputStream();
            os.write(data.getBytes());
            os.flush();

            if (conn.getResponseCode() != HttpURLConnection.HTTP_OK) {
                throw new RuntimeException("Failed : HTTP error code : "
                    + conn.getResponseCode());
            }

            BufferedReader br = new BufferedReader(new InputStreamReader(
                    (conn.getInputStream())));

            String output;
            while ((output = br.readLine()) != null) {
                result += output;
            }
            conn.disconnect();

          } catch (Exception e) {
             System.err.println(e.toString());
          }
        return result;
    }

    private Boolean saveToDisk(String domain, String clientId){
        String filePath = "/opt/vmware/share/config/lightwave-ui-oidc.xml";
        File f = new File(filePath);
        Boolean success = true;
        if(!f.exists()) {
            success = createNewXml(filePath);
        }
        if(success){
            success = addNewXmlNode(filePath, domain, clientId);
        }
        return success;
    }

    private Boolean createNewXml(String xmlFile){

        try {
                DocumentBuilderFactory documentBuilderFactory = DocumentBuilderFactory.newInstance();
                DocumentBuilder documentBuilder = documentBuilderFactory.newDocumentBuilder();
                Document document = documentBuilder.newDocument();
                Element root = document.createElement("tenants");
                document.appendChild(root);
                writeXmlToDisk(xmlFile, document);
                return true;
        } catch(Exception e) {
            System.err.println(e.toString());
            return false;
        }
    }

    private Boolean addNewXmlNode(String xmlFile, String domain, String clientId){

        try {
                DocumentBuilderFactory documentBuilderFactory = DocumentBuilderFactory.newInstance();
                DocumentBuilder documentBuilder = documentBuilderFactory.newDocumentBuilder();
                Document document = documentBuilder.parse(xmlFile);
                Element root = document.getDocumentElement();
                Element tenantNode = document.createElement("tenant");
                Element name = document.createElement("name");
                name.appendChild(document.createTextNode(domain));
                Element clientIdNode = document.createElement("clientId");
                clientIdNode.appendChild(document.createTextNode(clientId));
                tenantNode.appendChild(name);
                tenantNode.appendChild(clientIdNode);
                root.appendChild(tenantNode);
                writeXmlToDisk(xmlFile, document);
                return true;
        } catch(Exception e){
            System.err.println(e.toString());
            return false;
        }
    }

    private void writeXmlToDisk(String xmlFile, Document document)
            throws TransformerFactoryConfigurationError, TransformerConfigurationException, TransformerException {
        DOMSource source = new DOMSource(document);
        TransformerFactory transformerFactory = TransformerFactory.newInstance();
        Transformer transformer = transformerFactory.newTransformer();
        StreamResult result = new StreamResult(xmlFile);
        transformer.transform(source, result);
    }

        private void configureLandingPage()
                throws SecureTokenServerInstallerException {

            String webappDir = InstallerUtils.joinPath(InstallerUtils
                    .getInstallerHelper().getTCBase(), "webapps");
            System.out.println("Configure ROOT index.jsp for Lightwave ");

            String rootPagePath = InstallerUtils.joinPath(webappDir, "ROOT",
                    "index.jsp");
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(
                    rootPagePath))) {

                writer.append("<html>");
                writer.append(System.lineSeparator());
                writer.append("<head>");
                writer.append(System.lineSeparator());
                writer.append(String
                        .format("<meta http-equiv=\"refresh\" content=\"0;URL=lightwaveui\">%s",
                                System.lineSeparator()));
                writer.append("</head>");
                writer.append(System.lineSeparator());
                writer.append("<body> </body>");
                writer.append(System.lineSeparator());
                writer.append("</html>");
                writer.append(System.lineSeparator());

            } catch (IOException e) {
                System.err.println("Failed to configure Landing Page");
                e.printStackTrace();
                throw new SecureTokenServerInstallerException(
                        "Failed to configure Landing Page", e);
            }
        }

    public void registerOidcClientForLightwaveUI(String hostname, String subjectAltName, String domain, String username, String password){
        try {
            System.out.println("Started regsitration for Oidc against tenant : " + domain);

            String token = authenticate(hostname, domain, username, password);
            String oidc = registerOidc(hostname, subjectAltName, domain, token);

            System.out.println("Oidc successfully added. Details - " + oidc);
            String[] parts = oidc.split(",");
            String clientId = "";
            Boolean found = false;
            for(int i=0; i<parts.length; i++){
                if(parts[i].contains("\"clientId\":")){
                    clientId = parts[i].split(":")[1].replace("\"", "");
                    found = true;
                    break;
                }
            }
            if(!found){
                throw new Exception("Client ID for OIDC not found in response.");
            }
            Boolean success = saveToDisk(domain, clientId);
            if(success){
                System.out.println("Oidc has been saved into local store against tenant : " + domain);

            }
            else {
                System.out.println("Failed to save Oidc into local store against tenant : " + domain);
            }
        } catch(Exception exc){
            System.out.println("Lightwave UI OIDC Client registration failed ... ");
            System.out.println(exc.toString());
        }
    }
    @Override
    public void upgrade() throws Exception{
    }

    @Override
    public void uninstall() {
        // TODO Auto-generated method stub

    }

    @Override
    public PlatformInstallComponent getComponentInfo() {
        return new PlatformInstallComponent(ID, Name, Description);
    }
    @Override
    public void migrate() {
        // No-op
    }
}
