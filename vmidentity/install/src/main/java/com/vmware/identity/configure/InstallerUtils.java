package com.vmware.identity.configure;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.SystemUtils;

public class InstallerUtils {

    public static final String CONFIG_DIRECTORY_DCACCOUNT_DN_VALUE = "dcAccountDN";
    public static final String CONFIG_DIRECTORY_LDAP_PORT_VALUE = "LdapPort";
    public static final String CONFIG_DIRECTORY_DCACCOUNT_PASSWORD = "dcAccountPassword";
    public static final String HOSTNAME_FILE = "hostname.txt";
    public static final String IDM_HOST = "localhost";
    public static final String SSL_CERT_TYPE = "X.509";
    public static final String SIGNING_CERT_TYPE = "X.509";
    public static final String STS_TCPORT = "443";
    public static final String STS_LOCALPORT = "7444";
    public static final String CONFIG_IDENTITY_KEY = "Software\\VMware\\Identity\\Configuration";
    public static final String ROOT_CERT_NAME = "ssoserverRoot.crt";
    public static final String LEAF_KEY_PUBLIC_NAME = "ssoserver.pub";
    public static final String LEAF_KEY_PRIVATE_NAME = "ssoserver.key";
    public static final Integer KEY_LENGTH = 2048;
    public static final long VMCA_CERT_EXPIRY_START_LAG = 10 * 60 * 1000;
    private static final long MILLIS_IN_SECOND = 1000;
    private static final long SECONDS_IN_MINUTE = 60;
    private static final long MINUTES_IN_HOUR = 60;
    private static final long HOURS_IN_DAY = 24;
    private static final long DAYS_IN_YEAR = 180;
    public static final long VMCA_DEFAULT_CERT_VALIDITY = MILLIS_IN_SECOND
            * SECONDS_IN_MINUTE * MINUTES_IN_HOUR * HOURS_IN_DAY * DAYS_IN_YEAR;

    public static final String CERT_PASSWORD_SSL = "changeme";
    public static final String LEAF_CERT_ALIAS = "ssoserver";
    public static final String LEAF_CERT_X509_NAME = "ssoserver.crt";
    public static final String LEAF_CERT_PKCS_NAME = "ssoserver.p12";
    public static final String SSL_ROOT_CERT_X509_NAME = "ssoserverRoot.crt";
    public static final String LEAF_SIGN_CERT_KEY = "ssoserverSign.key";
    public static final String LEAF_SIGN__PUBLIC_CERT_KEY = "ssoserverSign.pub";
    public static final String LEAF_SIGN_CERT_X509_NAME = "ssoserverSign.crt";
    public static final String LEAF_SIGN_CERT_ALIAS = "ssoserverSign";
    public static final int CLOCK_TOLERANCE = 10 * 60 * 1000;
    public static final int INITIAL_RENEW_COUNT = 10;
    public static final int INITIAL_DELEGATION_COUNT = 10;
    public static final long MAX_LIFETIME_HOK_TOKENS = 30l * 24 * 60 * 60
            * 1000;// 30d
    public static final long MAX_LIFETIME_BEARER_TOKENS = 5l * 60 * 1000; // 5min
    public static final String BEGIN_CERT = "-----BEGIN CERTIFICATE-----\n";
    public static final String END_CERT = "\n-----END CERTIFICATE-----";
    public static final String BEGIN_PRIVATE_KEY = "-----BEGIN PRIVATE KEY-----\n";
    public static final String END_PRIVATE_KEY = "-----END PRIVATE KEY-----";
    public static final String BEGIN_PUBLIC_KEY = "-----BEGIN PUBLIC KEY-----\n";
    public static final String END_PUBLIC_KEY = "-----END PUBLIC KEY-----";
    public static final String DEFAULT_USER = "Administrator";
    public static final String CERTIFICATE_NAME = "Acme";
    public static final String CERTIFICATE_COUNTRY = "US";
    public static final String CERTIFICATE_EMAIL = "email@acme.com";
    public static final int REVERSE_PROXY_PORT = 443;

    public static String CONFIG_DIRECTORY_ROOT_KEY = null;
    public static String CONFIG_DIRECTORY_PARAMETERS_KEY = null;
    public static String CONFIG_DIRECTORY_CREDS_ROOT_KEY = null;

    static {

        if (SystemUtils.IS_OS_LINUX) {

            CONFIG_DIRECTORY_ROOT_KEY = "Services\\vmdir";
        } else if (SystemUtils.IS_OS_WINDOWS) {

            CONFIG_DIRECTORY_ROOT_KEY = "System\\CurrentControlset\\Services\\VMwareDirectoryService";
        } else {
            throw new IllegalStateException(
                    "Only Linux and Windows platforms is supported");
        }

        CONFIG_DIRECTORY_CREDS_ROOT_KEY = CONFIG_DIRECTORY_ROOT_KEY;
        CONFIG_DIRECTORY_PARAMETERS_KEY = CONFIG_DIRECTORY_ROOT_KEY
                + "\\Parameters";
    }

    public static InstallerHelper getInstallerHelper() {
        if (SystemUtils.IS_OS_LINUX) {
            return new LinuxInstallerHelper();
        } else {
            return new WinInstallerHelper();
        }
    }

    public static String joinPath(String path1, String path2) {
        return String.format("%s%s%s", path1, File.separator, path2);
    }

    public static String joinPath(String path1, String path2, String path3) {
        return String.format("%s%s%s%s%s", path1, File.separator, path2,
                File.separator, path3);
    }

    public static void writeToFile(Path path, String content)
            throws FileNotFoundException, IOException {
        Writer writer = null;

        try {
            BufferedOutputStream outputStream = new BufferedOutputStream(
                    new FileOutputStream(path.toString()));
            writer = new OutputStreamWriter(outputStream);
            writer.write(content);
        } finally {
            if (writer != null)
                writer.close();
        }
    }

    public static String getPemEncodedString(byte[] privBytes)
            throws UnsupportedEncodingException {

        String encoded = new String(Base64.encodeBase64(privBytes));
        StringBuffer pemencode = new StringBuffer();
        for (int x = 0; x < encoded.length(); x++) {

            if ((x > 0) && (x % 64 == 0)) {
                pemencode.append("\n");
                pemencode.append(encoded.charAt(x));
            } else {
                pemencode.append(encoded.charAt(x));

            }
        }
        return pemencode.toString();
    }

    public static void createCertificateKeyDirectory(String certDir)
            throws IOException {
        Path certDirPath = Paths.get(certDir);
        if (Files.notExists(certDirPath)) {

            Files.createDirectories(certDirPath);
        }
        getInstallerHelper().setPermissions(certDirPath);
    }
}