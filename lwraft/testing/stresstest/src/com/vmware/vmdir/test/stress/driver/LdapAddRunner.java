package com.vmware.vmdir.test.stress.driver;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.concurrent.BlockingQueue;

import com.unboundid.ldap.sdk.Attribute;
import com.unboundid.ldap.sdk.LDAPConnection;
import com.unboundid.ldap.sdk.LDAPException;
import com.unboundid.ldap.sdk.LDAPResult;
import com.unboundid.ldap.sdk.Modification;
import com.unboundid.ldap.sdk.ModificationType;
import com.unboundid.ldap.sdk.ModifyRequest;
import com.unboundid.ldap.sdk.ResultCode;

public class LdapAddRunner extends LdapRunnerThread {

    private BlockingQueue<String> dnBlockingQueue;
    private String rdnAttribute;
    private String[] ocs;
    private String[] ats;
    private int objectclassNum = 0;
    private int attributeNum = 0;
    private boolean bThreadIDPrefix = false;

    public LdapAddRunner(Map<String, String> configMap, BlockingQueue<String> dnBlockingQ)
            throws Exception {
        super(configMap);

        dnBlockingQueue = dnBlockingQ;
        try {
            rdnAttribute = new String(runConfigMap.get(ConfigReader.keyRDNAttribute));

            objectclassNum = Integer.valueOf(configMap.get(ConfigReader.keyObjectclassNum));
            ocs = new String[objectclassNum];
            for (int iCnt = 0; iCnt < objectclassNum; iCnt++) {
                ocs[iCnt] = new String(runConfigMap.get(ConfigReader.keyObjectclass
                        + String.valueOf(iCnt + 1)));
            }

            attributeNum = Integer.valueOf(configMap.get(ConfigReader.keyAttributeNum));
            ats = new String[attributeNum];
            for (int iCnt = 0; iCnt < attributeNum; iCnt++) {
                ats[iCnt] = new String(runConfigMap.get(ConfigReader.keyAttribute
                        + String.valueOf(iCnt + 1)));
            }

            String thrIDPrefix = new String(runConfigMap.get(ConfigReader.keyThreadIDPrefix));
            if (thrIDPrefix.equalsIgnoreCase("on")) {
                bThreadIDPrefix = true;
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    public void run() {
        Random randomGenerator = new Random();
        LDAPConnection ldapConn = getConnection(ldapHost, Integer.valueOf(ldapPort));

        long threadId = Thread.currentThread().getId();

        if (ldapConn != null) {
            try {
                ldapConn.bind(bindDN, bindPassword);

                resetPasswordPolicyEntry(ldapConn);

                // TODO, should be in a loop and wait for stop
                while (true) {
                    ResultCode addResultCode = null;
                    LDAPResult addResult = null;
                    Integer nextNumber = Math.abs(randomGenerator.nextInt());
                    String myNextID = nextNumber.toString();

                    if (bThreadIDPrefix) {
                        myNextID = threadId + "." + myNextID;
                    }

                    try {
                        String myDN = rdnAttribute + "=" + rdnAttribute + "." + myNextID + ","
                                + baseDN;

                        List<Attribute> myAttrs = new ArrayList<Attribute>();

                        for (int iCnt = 0; iCnt < objectclassNum; iCnt++) {
                            String thisOC = new String(runConfigMap.get(ConfigReader.keyObjectclass
                                    + String.valueOf(iCnt + 1)));
                            myAttrs.add(new Attribute("objectclass", thisOC));
                        }

                        for (int iCnt = 0; iCnt < attributeNum; iCnt++) {
                            String thisAT = new String(runConfigMap.get(ConfigReader.keyAttribute
                                    + String.valueOf(iCnt + 1)));
                            myAttrs.add(new Attribute(thisAT, thisAT + "." + myNextID));
                        }

                        addResult = ldapConn.add(myDN, myAttrs);
                        addResultCode = addResult.getResultCode();

                        if (dnBlockingQueue != null) {
                            dnBlockingQueue.put(myDN);
                        }
                    } catch (LDAPException le) {
                        if (le.getResultCode() != ResultCode.ENTRY_ALREADY_EXISTS) {
                            System.out.println(le.getMessage());
                        }
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                }

            } catch (LDAPException e) {
                e.printStackTrace();
            }

            ldapConn.close();
        }
    }

    private void resetPasswordPolicyEntry(LDAPConnection ldapConn) throws LDAPException {
        Modification[] mods = new Modification[7];

        mods[0] = new Modification(ModificationType.REPLACE, "vmIdentity-PasswordMaxLength", "50");
        mods[1] = new Modification(ModificationType.REPLACE,
                "vmIdentity-PasswordMaxIdenticalAdjacentChars", "50");
        mods[2] = new Modification(ModificationType.REPLACE, "vmIdentity-PasswordMinNumericCount",
                "1");
        mods[3] = new Modification(ModificationType.REPLACE,
                "vmIdentity-PasswordMinUpperCaseCount", "0");
        mods[4] = new Modification(ModificationType.REPLACE,
                "vmIdentity-PasswordMinAlphabeticCount", "1");
        mods[5] = new Modification(ModificationType.REPLACE,
                "vmIdentity-PasswordMinSpecialCharCount", "0");
        mods[6] = new Modification(ModificationType.REPLACE, "vmIdentity-PasswordMinLength", "2");

        String domainDN = runConfigMap.get(ConfigReader.keyDomainDN);
        ModifyRequest modifyRequest = new ModifyRequest("cn=password and lockout policy,"
                + domainDN, mods);

        // modifyRequest.setModifications(mods);

        ldapConn.modify(modifyRequest);
    }
}
