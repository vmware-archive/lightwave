package com.vmware.vmdir.test.stress.driver;

import java.util.Map;
import java.util.concurrent.BlockingQueue;

import com.unboundid.ldap.sdk.LDAPConnection;
import com.unboundid.ldap.sdk.LDAPException;
import com.unboundid.ldap.sdk.LDAPResult;

import com.unboundid.ldap.sdk.ResultCode;

public class LdapDeleteRunner extends LdapRunnerThread {

    private BlockingQueue<String> dnBlockingQueue;

    public LdapDeleteRunner(Map<String, String> configMap, BlockingQueue<String> dnBlockingQ)
            throws Exception {
        super(configMap);

        dnBlockingQueue = dnBlockingQ;

        if (dnBlockingQueue == null) {
            throw new IllegalArgumentException("DN Blocking Queue is null");
        }
    }

    public void run() {
        LDAPConnection ldapConn = getConnection(ldapHost, Integer.valueOf(ldapPort));

        if (ldapConn != null) {
            try {
                ldapConn.bind(bindDN, bindPassword);

                // TODO, should be in a loop and wait for stop
                while (true) {
                    ResultCode deleteResultCode = null;
                    LDAPResult deleteResult = null;

                    String myDN = dnBlockingQueue.take();

                    try {
                        deleteResult = ldapConn.delete(myDN);
                        deleteResultCode = deleteResult.getResultCode();

                    } catch (LDAPException le) {
                        if (le.getResultCode() != ResultCode.ENTRY_ALREADY_EXISTS) {
                            System.out.println(le.getMessage());
                        }
                    }
                }

            } catch (LDAPException e) {
                e.printStackTrace();
            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            ldapConn.close();
        }
    }

}
