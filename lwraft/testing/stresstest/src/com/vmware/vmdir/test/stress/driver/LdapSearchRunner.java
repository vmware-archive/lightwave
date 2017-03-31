package com.vmware.vmdir.test.stress.driver;

import java.util.Map;
import java.util.Random;

import com.unboundid.ldap.sdk.BindResult;
import com.unboundid.ldap.sdk.Filter;
import com.unboundid.ldap.sdk.LDAPConnection;
import com.unboundid.ldap.sdk.LDAPException;
import com.unboundid.ldap.sdk.ResultCode;
import com.unboundid.ldap.sdk.SearchRequest;
import com.unboundid.ldap.sdk.SearchResult;
import com.unboundid.ldap.sdk.SearchScope;
import com.unboundid.util.ValuePattern;

public class LdapSearchRunner extends LdapRunnerThread {

    private ValuePattern[] filterPatterns;
    private String[] filterBases;
    private String[] filterScopes;
    private int searchDefinitionNum = 0;

    public LdapSearchRunner(Map<String, String> configMap) throws Exception {
        super(configMap);

        try {
            searchDefinitionNum = Integer.valueOf(configMap
                    .get(ConfigReader.keySearchDefinitionNum));
            filterPatterns = new ValuePattern[searchDefinitionNum];
            filterBases = new String[searchDefinitionNum];
            filterScopes = new String[searchDefinitionNum];

            for (int iCnt = 0; iCnt < searchDefinitionNum; iCnt++) {
                filterPatterns[iCnt] = new ValuePattern(runConfigMap.get(ConfigReader.keyFilter
                        + String.valueOf(iCnt + 1)), null);
                filterBases[iCnt] = new String(runConfigMap.get(ConfigReader.keySearchBase
                        + String.valueOf(iCnt + 1)));
                filterScopes[iCnt] = new String(runConfigMap.get(ConfigReader.keyScope
                        + String.valueOf(iCnt + 1)));
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    public void run() {
        Random randomGenerator = new Random();
        String searchScope = runConfigMap.get(ConfigReader.keyScope);
        LDAPConnection ldapConn = getConnection(ldapHost, Integer.valueOf(ldapPort));
        SearchRequest searchRequest = new SearchRequest("", SearchScope.ONE,
                Filter.createPresenceFilter("objectClass"), new String[0]);

        if (ldapConn != null) {
            SearchResult searchResult = null;

            try {
                ldapConn.bind(bindDN, bindPassword);

                // TODO, should be in a loop and wait for stop
                while (true) {
                    int filterIndex = Math.abs(randomGenerator.nextInt() % searchDefinitionNum);

                    String nextFilter = filterPatterns[filterIndex].nextValue();
                    String nextBase = filterBases[filterIndex];
                    String nextScope = filterScopes[filterIndex];

                    try {

                        searchRequest.setBaseDN(nextBase);
                        searchRequest.setFilter(nextFilter);
                        if (nextScope.equalsIgnoreCase("base")) {
                            searchRequest.setScope(SearchScope.BASE);
                        } else if (nextScope.equalsIgnoreCase("one")) {
                            searchRequest.setScope(SearchScope.ONE);
                        } else {
                            searchRequest.setScope(SearchScope.SUB);
                        }

                        searchResult = ldapConn.search(searchRequest);

                    } catch (LDAPException e) {
                        // e.printStackTrace();
                        if (e.getResultCode() != ResultCode.NO_SUCH_OBJECT) {
                            break;
                        }
                    }
                }

            } catch (LDAPException e) {
                e.printStackTrace();
            }

            ldapConn.close();
        }
    }
}
