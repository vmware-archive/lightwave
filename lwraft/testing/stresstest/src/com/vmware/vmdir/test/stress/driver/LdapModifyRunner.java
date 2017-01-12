package com.vmware.vmdir.test.stress.driver;

import java.util.List;
import java.util.Map;
import java.util.Random;

import com.unboundid.asn1.ASN1OctetString;
import com.unboundid.ldap.sdk.Filter;
import com.unboundid.ldap.sdk.LDAPConnection;
import com.unboundid.ldap.sdk.LDAPException;
import com.unboundid.ldap.sdk.LDAPResult;
import com.unboundid.ldap.sdk.Modification;
import com.unboundid.ldap.sdk.ModificationType;
import com.unboundid.ldap.sdk.ModifyRequest;
import com.unboundid.ldap.sdk.ResultCode;
import com.unboundid.ldap.sdk.SearchRequest;
import com.unboundid.ldap.sdk.SearchResult;
import com.unboundid.ldap.sdk.SearchResultEntry;
import com.unboundid.ldap.sdk.SearchScope;
import com.unboundid.util.ValuePattern;

public class LdapModifyRunner extends LdapRunnerThread {

    private ValuePattern filterPattern;
    private String modifyAttribute;
    private String modifyValuePrefix;

    public LdapModifyRunner(Map<String, String> configMap) throws Exception {
        super(configMap);

        try {
            filterPattern = new ValuePattern(runConfigMap.get(ConfigReader.keyFilter), null);
            modifyAttribute = new String(runConfigMap.get(ConfigReader.keyModifyAttribute));
            modifyValuePrefix = new String(runConfigMap.get(ConfigReader.keyModifyValuePrefix));
        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    public void run() {
        Random randomGenerator = new Random();
        LDAPConnection ldapConn = getConnection(ldapHost, Integer.valueOf(ldapPort));
        SearchRequest searchRequest = new SearchRequest("", SearchScope.SUB,
                Filter.createPresenceFilter("objectClass"), new String[0]);
        Modification[] mods = new Modification[1];
        ModifyRequest modifyRequest = new ModifyRequest("", mods);

        if (ldapConn != null) {
            SearchResult searchResult = null;
            LDAPResult modResult = null;

            try {
                ldapConn.bind(bindDN, bindPassword);

                // TODO, should be in a loop and wait for stop
                while (true) {
                    String nextFilter = filterPattern.nextValue();

                    try {

                        searchRequest.setBaseDN(baseDN);
                        searchRequest.setFilter(nextFilter);

                        searchResult = ldapConn.search(searchRequest);

                    } catch (LDAPException e) {
                        // e.printStackTrace();
                        if (e.getResultCode() == ResultCode.NO_SUCH_OBJECT) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    List<SearchResultEntry> entryList = searchResult.getSearchEntries();
                    SearchResultEntry myEntry = entryList.get(entryList.size() - 1);

                    String updateEmailTo = modifyValuePrefix + Math.abs(randomGenerator.nextInt());
                    final ASN1OctetString[] values = new ASN1OctetString[1];
                    values[0] = new ASN1OctetString(updateEmailTo.getBytes());
                    mods[0] = new Modification(ModificationType.REPLACE, modifyAttribute, values);

                    modifyRequest.setModifications(mods);
                    modifyRequest.setDN(myEntry.getDN());

                    try {

                        modResult = ldapConn.modify(modifyRequest);

                    } catch (LDAPException e) {
                        e.printStackTrace();
                    }
                }

            } catch (LDAPException e) {
                e.printStackTrace();
            }

            ldapConn.close();
        }
    }
}
