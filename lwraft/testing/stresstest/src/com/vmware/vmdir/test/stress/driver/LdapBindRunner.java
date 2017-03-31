package com.vmware.vmdir.test.stress.driver;

import java.util.Map;

import com.unboundid.ldap.sdk.BindResult;
import com.unboundid.ldap.sdk.LDAPConnection;
import com.unboundid.ldap.sdk.LDAPException;
import com.unboundid.ldap.sdk.ResultCode;
import com.unboundid.util.ValuePattern;

public class LdapBindRunner extends LdapRunnerThread {

    private ValuePattern bindPattern;

    public LdapBindRunner(Map<String, String> configMap) throws Exception {
        super(configMap);

        try {
            bindPattern = new ValuePattern(runConfigMap.get(ConfigReader.keyBindPattern), null);
        } catch (Exception e) {
            throw e;
        }
    }

    public void run() {

        String rdnPrefix = runConfigMap.get(ConfigReader.keyBindRDNPrefix);
        String passwordPrefix = runConfigMap.get(ConfigReader.keyBindPasswordPrefix);
        String bindContainerDN = runConfigMap.get(ConfigReader.keyBindContainerDN);

        LDAPConnection ldapConn = getConnection(ldapHost, Integer.valueOf(ldapPort));

        if (ldapConn != null) {
            // TODO, should be in a loop and wait for stop
            while (true) {
                BindResult bindResult = null;
                String tmpPattern = bindPattern.nextValue();
                final String tmpBindDN = rdnPrefix + tmpPattern + "," + bindContainerDN;
                final String tmpBindPass = passwordPrefix + tmpPattern;

                try {
                    bindResult = ldapConn.bind(tmpBindDN, tmpBindPass);
                } catch (LDAPException e) {
                    // e.printStackTrace();
                    if (e.getResultCode() != ResultCode.NO_SUCH_OBJECT
                            && e.getResultCode() != ResultCode.INVALID_CREDENTIALS) {
                        break;
                    }
                }
            }

            ldapConn.close();
        }
    }
}
