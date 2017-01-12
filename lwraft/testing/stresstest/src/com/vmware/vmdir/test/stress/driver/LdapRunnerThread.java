package com.vmware.vmdir.test.stress.driver;

import java.security.GeneralSecurityException;
import java.util.Map;

import com.unboundid.ldap.sdk.LDAPConnection;
import com.unboundid.ldap.sdk.LDAPException;
import com.unboundid.util.ssl.SSLUtil;
import com.unboundid.util.ssl.TrustAllTrustManager;

abstract public class LdapRunnerThread extends Thread {

    Map<String, String> runConfigMap;
    String ldapHost;
    String ldapPort;
    String bindDN;
    String bindPassword;
    String baseDN;
    String protocol;

    public LdapRunnerThread(Map<String, String> configMap) {
        runConfigMap = configMap;
        ldapHost = runConfigMap.get(ConfigReader.keyHost);
        ldapPort = runConfigMap.get(ConfigReader.keyPort);
        bindDN = runConfigMap.get(ConfigReader.keyBindDN);
        bindPassword = runConfigMap.get(ConfigReader.keyBindPassword);
        baseDN = runConfigMap.get(ConfigReader.keyBaseDN);
        protocol = runConfigMap.get(ConfigReader.keyProtocol);
    }

    public LDAPConnection getConnection(String hostName, int portNumber) {
        LDAPConnection ldapConn = null;

        try {
            if (protocol.equalsIgnoreCase("LDAPS"))
            {
                SSLUtil sslUtil = new SSLUtil(new TrustAllTrustManager());
                ldapConn = new LDAPConnection(sslUtil.createSSLSocketFactory());
            }
            else
            {
                ldapConn = new LDAPConnection();
            }

            ldapConn.connect(hostName, portNumber);

        } catch (LDAPException e1) {
            e1.printStackTrace();
            if (ldapConn != null) {
                ldapConn.close();
            }
        } catch (GeneralSecurityException e) {
            e.printStackTrace();
            if (ldapConn != null) {
                ldapConn.close();
            }
        }

        return ldapConn;
    }
}
