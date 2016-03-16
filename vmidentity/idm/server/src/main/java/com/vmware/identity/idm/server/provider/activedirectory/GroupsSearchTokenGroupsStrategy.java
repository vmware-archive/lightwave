/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */
package com.vmware.identity.idm.server.provider.activedirectory;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.provider.ILdapSchemaMapping;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapConnectionExWithGetConnectionString;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.performanceSupport.LdapQueryStat;

/**
 * Implementation of GroupSearchOptionStrategy that uses tokenGroups attribute to get the groups.
 */
class GroupsSearchTokenGroupsStrategy extends GroupSearchDefaultStrategy implements GroupSearchOptionStrategy {

    private final String ATTR_NAME_SAM_ACCOUNT;
    private final String ATTR_OBJECT_SID;
    private final String ATTR_GROUP_TYPE;
    private final String ATTR_DESCRIPTION;
    public static final int DEFAULT_FILTER_CLAUSE_COUNT = 500;

    private final IIdmAuthStatRecorder authStatRecorder;
    private final List<ILdapEntry> ldapEntries;
    private final ILdapSchemaMapping schemaMapping;
    private List<String> toProcessGroups;
    private ILdapConnectionEx connection;
    private ILdapConnectionEx gcConnection;
    private String connectionDomain;
    Set<String> processedGroups = new HashSet<>();

    public GroupsSearchTokenGroupsStrategy(ILdapConnectionEx connection, ILdapConnectionEx gcConnection, List<ILdapEntry> ldapEntries, String connectionDomain, String joinedDomain, ILdapSchemaMapping schemaMapping,
            IIdmAuthStatRecorder authStatRecorder) {

        super(schemaMapping, joinedDomain);
        this.ldapEntries = ldapEntries;
        this.authStatRecorder = authStatRecorder;
        this.schemaMapping = schemaMapping;
        this.connection = connection;
        this.gcConnection = gcConnection;
        this.connectionDomain = connectionDomain;

        ATTR_NAME_SAM_ACCOUNT = schemaMapping
                .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        ATTR_OBJECT_SID = schemaMapping
                .getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
        ATTR_DESCRIPTION = schemaMapping
                .getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
        ATTR_GROUP_TYPE = schemaMapping
                .getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupType);
    }

    @Override
    public Map<String, List<String>> getFilterByDomain() {
        Map<String, List<String>> domainFilter = new HashMap<>();

        String suffix = "(objectClass=group)";
        String clauseAttribute = "(objectSid=";

        String searchBaseDN = "";
        domainFilter.put(searchBaseDN, getFilters(suffix, toProcessGroups, clauseAttribute));

        return domainFilter;
    }

    @Override
    public List<String> getGroupSearchAttributes(boolean includeDescription) {
        List<String> ldapGroupAttributes = new ArrayList<>();

        ldapGroupAttributes.add(ATTR_NAME_SAM_ACCOUNT);
        ldapGroupAttributes.add(ATTR_OBJECT_SID);
        ldapGroupAttributes.add(ATTR_GROUP_TYPE);

        if (includeDescription) {
            ldapGroupAttributes.add(ATTR_DESCRIPTION);
        }

        return ldapGroupAttributes;
    }

    @Override
    public boolean useGCConnection() {
        return true;
    }

    @Override
    public String getConnectionDomain() {
        return connectionDomain;
    }

    @Override
    public void addParents(ILdapEntry entry) {
        //parents don't have to be added for tokenGroups attribute
    }

    private String getConnectionString(ILdapConnectionEx connection) {
        String connString = null;
        if (connection != null && connection instanceof ILdapConnectionExWithGetConnectionString) {
            connString = ((ILdapConnectionExWithGetConnectionString) connection).getConnectionString();
        }
        return connString;
    }

    @Override
    public boolean hasGroupsToProcess() throws Exception {
        populateGroupsToProcess();
        return toProcessGroups.size() > 0;
    }

    @Override
    public void addProcessedGroup(String dn) {
        processedGroups.add(dn);
    }

    private void populateGroupsToProcess() throws Exception {
        toProcessGroups = new ArrayList<String>();

        if (ldapEntries == null || ldapEntries.size() == 0)
            return;

        populateGroupSids(connection, ldapEntries);
        if (gcConnection != null)
            populateGroupSids(gcConnection, ldapEntries);

        //ldapEntries were processed that's why the list is cleared
        ldapEntries.clear();
    }

    private void populateGroupSids(ILdapConnectionEx connection, List<ILdapEntry> ldapEntries) {
        String tokenGroupsAttribute = IdentityStoreAttributeMapping.AttributeIds.GroupAttributeTokenGroups;

        String ATTR_TOKEN_GROUPS = schemaMapping.getUserAttribute(tokenGroupsAttribute);
        String[] attributes = new String[] { ATTR_TOKEN_GROUPS };
        String filter = schemaMapping.getAllUsersQuery();
        long startTime = System.nanoTime();

        for (ILdapEntry entry : ldapEntries) {
            String userDn = entry.getDN();
            try (ILdapMessage message = connection.search(userDn, LdapScope.SCOPE_BASE, filter, attributes, false)) {

                if (authStatRecorder != null) {
                    long elapsed = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime);
                    authStatRecorder.add(new LdapQueryStat(filter, userDn, getConnectionString(connection), elapsed, 1));
                }

                ILdapEntry[] entries = message.getEntries();
                if (entries != null && entries.length == 1) {
                    LdapValue[] values = entries[0].getAttributeValues(ATTR_TOKEN_GROUPS);
                    for (LdapValue ldapValue : values) {

                        byte[] groupSID = ldapValue.getValue();
                        SecurityIdentifier sid = SecurityIdentifier.build(groupSID);
                        String groupSidString = sid.toString();

                        boolean excludeWellKnownGroup = excludeGroup(null, groupSidString);
                        if (!excludeWellKnownGroup)
                        {
                            if (!processedGroups.contains(groupSidString))
                            {
                                toProcessGroups.add(groupSidString);
                                addProcessedGroup(groupSidString);
                            }
                        }
                    }
                }
            }
        }
    }

    @Override
    public void addLdapEntriesToProcess(List<ILdapEntry> ldapEntries) {
        ldapEntries.addAll(ldapEntries);
    }
}
