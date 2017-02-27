package com.vmware.identity.ssoconfig;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.URI;
import java.net.UnknownHostException;
import java.nio.file.FileSystems;
import java.security.KeyStore;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.List;
import java.util.Scanner;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.SystemUtils;
import org.apache.http.conn.ssl.DefaultHostnameVerifier;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.ssl.TrustStrategy;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.installer.ReleaseUtil;
import com.vmware.identity.rest.afd.client.AfdClient;
import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;

/**
 * SSO Configuration utility functions.
 *
 */
public class SSOConfigurationUtils {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SSOConfigurationUtils.class);
    static String IDP_HOST_ADDRESS = "localhost";

    static {
        javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(
        new javax.net.ssl.HostnameVerifier(){

            public boolean verify(String hostname,
                    javax.net.ssl.SSLSession sslSession) {
                if (hostname.equals("localhost")) {
                    return true;
                }
                return false;
            }
        });

        try {
            IDP_HOST_ADDRESS = InetAddress.getLocalHost().getCanonicalHostName();
        } catch (UnknownHostException e) {
            logger.error("Cannot resolve IDP FQDN. Using the default - " + IDP_HOST_ADDRESS);
        }
    }

    static final String DEFAULT_TENANT = "vsphere.local";
    static final int DEFAULT_OP_PORT = 443;

    static final String CREDENTIALS_FILE = getTcStsInstanceConfDir() + "/oidc_token";
    static final String KEY_VALUE_SEPARATOR = ":";
    static final String CHARSET = "UTF-8";

    private static IdmClient idmClient;
    private static AfdClient afdClient;
    private static VmdirClient vmdirClient;

    private static final String DISPLAY_PARAM_PASSWORD_AUTH = "IsPasswordAuthEnabled";
    private static final String DISPLAY_PARAM_WINDOWS_AUTH = "IsWindowsAuthEnabled";
    private static final String DISPLAY_PARAM_CERT_AUTH = "IsTLSClientCertAuthnEnabled";
    private static final int DEFAULT_DISPLAY_WIDTH = 100;
    private static final int LDAPS_PORT = 636;

    private static final String UNDEFINED_CONFIG = "UndefinedConfig";

    static IdmClient getIdmClient() throws Exception {
        if (idmClient == null) {
            TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            trustManagerFactory.init(getKeyStore());
            SSLContext sslContext = SSLContext.getInstance("SSL");
            sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
            idmClient = new IdmClient(IDP_HOST_ADDRESS, DEFAULT_OP_PORT, new DefaultHostnameVerifier(), sslContext);
            idmClient.setToken(new AccessToken(getBearerToken(), AccessToken.Type.JWT));
        }

        return idmClient;
    }

    static String getBearerToken() {
        String bearerToken = null;
        try {
            Scanner scanner = new Scanner(new File(CREDENTIALS_FILE));
            bearerToken = scanner.useDelimiter("\\Z").next();
            scanner.close();
        } catch (FileNotFoundException e) {
            logger.error("Bearer token is not available. Please use set-credentials command to retrieve bearer token with admin username and password.");
        }
        return bearerToken;
    }

    static AfdClient getAfdClient() throws Exception {
        if (afdClient == null) {
            TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            trustManagerFactory.init(getKeyStore());
            SSLContext sslContext = SSLContext.getInstance("SSL");
            sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
            afdClient = new AfdClient(IDP_HOST_ADDRESS, DEFAULT_OP_PORT, new DefaultHostnameVerifier(), sslContext);
            afdClient.setToken(new AccessToken(getBearerToken(), AccessToken.Type.JWT));
        }

        return afdClient;
    }

    static VmdirClient getVmdirClient() throws Exception {
        if (vmdirClient == null) {
            TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            trustManagerFactory.init(getKeyStore());
            SSLContext sslContext = SSLContext.getInstance("SSL");
            sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
            vmdirClient = new VmdirClient(IDP_HOST_ADDRESS, DEFAULT_OP_PORT, new DefaultHostnameVerifier(), sslContext);
            vmdirClient.setToken(new AccessToken(getBearerToken(), AccessToken.Type.JWT));
        }

        return vmdirClient;
    }

    static void closeAllResources() {
        if (idmClient != null) {
            idmClient.close();
        }

        if (afdClient != null) {
            afdClient.close();
        }

        if (vmdirClient != null) {
            vmdirClient.close();
        }
    }

    static KeyStore getKeyStore() throws Exception {
        KeyStore keyStore = null;
        keyStore = KeyStore.getInstance("JKS");
        keyStore.load(null, null);
        populateSSLCertificates(keyStore);
	    return keyStore;
    }

    private static void populateSSLCertificates(KeyStore keyStore) throws Exception {
        AfdClient afdClient =
                new AfdClient(IDP_HOST_ADDRESS, DEFAULT_OP_PORT, NoopHostnameVerifier.INSTANCE, new SSLContextBuilder()
                        .loadTrustMaterial(null, new TrustStrategy() {
                            @Override
                            public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                                return true;
                            }
                        }).build());

        List<CertificateDTO> certs = afdClient.vecs().getSSLCertificates();
        afdClient.close();
        int index = 1;
        for (CertificateDTO cert : certs) {
            keyStore.setCertificateEntry(String.format("VecsSSLCert%d", index), cert.getX509Certificate());
            index++;
        }
    }

    static String getTCServerConfigFileFullName() {
        return FileSystems.getDefault().getPath(getTcStsInstanceConfDir(), "server.xml").toString();
    }

    static String getTcStsInstanceConfDir() {
        String tcSTSConfDir = "";
        try {
            tcSTSConfDir = ReleaseUtil.isLightwave() ? "/opt/vmware/vmware-sts/conf" : "/usr/lib/vmware-sso/vmware-sts/conf/";
        } catch (IOException e) {
            logger.error(e);
            System.exit(1);
        }
        return SystemUtils.IS_OS_WINDOWS ? FileSystems.getDefault().getPath(System
                .getenv("VMWARE_RUNTIME_DATA_DIR"), "VMwareSTSService/conf/").toString()
                : tcSTSConfDir;
    }

    static String getTCTrustedCAFileFullName() {
        return FileSystems.getDefault().getPath(getTcStsInstanceConfDir(), "trustedca.jks").toString();
    }

    static void displayAuthenticationPolicy(AuthenticationPolicyDTO policy) {
        displayAuthnEnableStatus(policy.isPasswordBasedAuthenticationEnabled(),
                policy.isWindowsBasedAuthenticationEnabled(),
                policy.isCertificateBasedAuthenticationEnabled());
        if (policy.getClientCertificatePolicy() != null) {
            displayParamNameAndValue("revocationCheckEnabled", policy.getClientCertificatePolicy().isRevocationCheckEnabled());
            displayParamNameAndValue("useOCSP", policy.getClientCertificatePolicy().isOcspEnabled());
            displayParamNameAndValue("useCRLAsFailOver", policy.getClientCertificatePolicy().isFailOverToCrlEnabled());
            String ocspUrl = policy.getClientCertificatePolicy().getOcspUrlOverride();
            if (null != ocspUrl) {
                displayParamNameAndValue("OCSPUrl (obsoleted! Please use the \"-add_alt_ocsp command\" to add per-site alternative ocsp responder/responders)", ocspUrl);
            }

            // TODO diaplay alt ocsp
            displayParamNameAndValue("useCertCRL", policy.getClientCertificatePolicy().isCrlDistributionPointUsageEnabled());
            String CRLUrl = policy.getClientCertificatePolicy().getCrlDistributionPointOverride();
            displayParamNameAndValue("CRLUrl", CRLUrl == null ? null : CRLUrl);
            List<String> oids = policy.getClientCertificatePolicy().getCertPolicyOIDs();
            if (oids != null) {
                for (String oid : oids) {
                    displayParamNameAndValue("oid", oid);
                }
            }
            List<CertificateDTO> trustedCAs = policy.getClientCertificatePolicy().getTrustedCACertificates();
            if(trustedCAs != null){
                for(CertificateDTO cert: trustedCAs){
                    displayParamNameAndValue("trustedCA", cert.getX509Certificate().getSubjectDN().toString());
                }
            }
        }
        //displayParamNameAndValue("enableHint", policy.getClientCertificatePolicy().isUserNameHintEnabled());

        System.out.println("\n\n");
    }

    static void displayAuthnEnableStatus(boolean password, boolean windows, boolean cert) {
        displayParamNameAndValue(DISPLAY_PARAM_PASSWORD_AUTH, password);
        displayParamNameAndValue(DISPLAY_PARAM_WINDOWS_AUTH, windows);
        displayParamNameAndValue(DISPLAY_PARAM_CERT_AUTH, cert);
        // TODO add RSA properties
        //displayParamNameAndValue(DISPLAY_PARAM_RSA_SECURID_AUTH, rsaSecurID);
    }

    /**
     * <p>
     * Output the given parameter and value to Standard output.
     * </p>
     * @param paramName
     *     Name of parameter to display
     * @param paramValue
     *     Value of the given param. If the value is null or empty, then {@value SsoConfig.UNDEFINED_CONFIG} is used.
     * @param width
     *     displayed string length
     */
    static void displayParamNameAndValue(String paramName, String paramValue, int width){
        if(StringUtils.isEmpty(paramValue) || paramValue == null){
            paramValue = UNDEFINED_CONFIG;
        }

        String space = "";
        paramName = paramName + ":";
        for(int i = 0; i < (width - paramName.length()); i++) {
            space = space + " ";
        }

        System.out.println(paramName + space + paramValue);
    }

    static void displayParamNameAndValue(String paramName, String paramValue) {
        displayParamNameAndValue(paramName, paramValue, DEFAULT_DISPLAY_WIDTH);
    }

    static void displayParamNameAndValue(String paramName, boolean paramValue){
        displayParamNameAndValue(paramName, String.valueOf(paramValue), DEFAULT_DISPLAY_WIDTH);
    }

    static void displayParamNameAndValue(String paramName, int paramValue){
        displayParamNameAndValue(paramName, String.valueOf(paramValue), DEFAULT_DISPLAY_WIDTH);
    }

    static void displayParamNameAndValues(String paramName, Collection<String> paramValues) {
        if (paramValues == null) {
            return;
        }
        for (String paramValue : paramValues) {
            displayParamNameAndValue(paramName, paramValue);
        }
    }

    static void displaySeparationLine() {
        StringBuilder sb = new StringBuilder();
        sb.append("\n\n");
        char delimiter = '=';
        for (int i = 0; i < DEFAULT_DISPLAY_WIDTH; i++) {
            sb.append(delimiter);
        }
        sb.append("\n\n");
        System.out.println(sb.toString());
    }

    static boolean checkBoolean(String val, boolean fallback) {
        if (val == null) {
            return fallback;
        }
        if (!val.equalsIgnoreCase("true") && !val.equalsIgnoreCase("false")) {
            throw new IllegalArgumentException("Invalid argument. The value must be [true] or [false].");
        } else {
            return Boolean.parseBoolean(val);
        }
    }

    static String checkString(String val, String fallback) {
        if (StringUtils.isEmpty(val)) {
            return fallback;
        } else {
            return val;
        }
    }

    static void checkMinimumProtocolIsSupported(URI uri) throws Exception {
        SSLContext context = null;
        SSLSocket sslSocket = null;
        String[] enabledProtocols = new String[] { "TLSv1",  "TLSv1.1",  "TLSv1.2" };
        try {
           context = SSLContext.getInstance("TLS");
           TrustManager tm = new X509TrustManager() {

               @Override
               public void checkClientTrusted(X509Certificate[] chain, String thumbp)
                       throws CertificateException {
                   //no-op for client
               }

               @Override
               public void checkServerTrusted(X509Certificate[] chain, String thumbp)
                       throws CertificateException {
                   //accept all
               }

               @Override
               public X509Certificate[] getAcceptedIssuers() {
                   return null;
               }
           };

           context.init(null, new TrustManager[]{tm}, null);

           SSLSocketFactory socketFactory = context.getSocketFactory();

           // SSLSocketFactory only builds the super type, need to cast to SSL socket.
           Socket s = socketFactory.createSocket(uri.getHost(), uri.getPort() == -1 ? LDAPS_PORT : uri.getPort());

           sslSocket = (SSLSocket) s;
           sslSocket.setEnabledProtocols( enabledProtocols );

           sslSocket.startHandshake();

        } finally {
            if (sslSocket != null) {
                try {
                    sslSocket.close();
                 } catch (IOException e) {
                    // do nothing
                 }
            }
        }
    }
}
