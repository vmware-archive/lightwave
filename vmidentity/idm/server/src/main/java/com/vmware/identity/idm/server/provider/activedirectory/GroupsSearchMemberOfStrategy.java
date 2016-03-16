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
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;
import java.util.concurrent.TimeUnit;

import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.provider.ILdapSchemaMapping;
import com.vmware.identity.idm.server.provider.NoSuchGroupException;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapConnectionExWithGetConnectionString;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.performanceSupport.LdapQueryStat;

/**
 * Implementation of GroupSearchOptionStrategy that uses memberof attribute to get the groups.
 */
class GroupsSearchMemberOfStrategy extends GroupSearchDefaultStrategy implements GroupSearchOptionStrategy {

    private final List<ILdapEntry> ldapEntries;
    private final IIdmAuthStatRecorder authStatRecorder;
    private final String userDomain;
    private final boolean includePrimaryGroup;
    private final ILdapSchemaMapping schemaMapping;

    private final String ATTR_DESCRIPTION;
    private final String ATTR_GROUP_TYPE;
    private final String ATTR_NAME_SAM_ACCOUNT;
    private final String ATTR_MEMBER_OF;
    private final String ATTR_PRIMARY_GROUP_ID;
    private final String ATTR_OBJECT_SID;
    private Stack<String> toProcessGroups;
    private ILdapConnectionEx connection;

    Set<String> processedGroups = new HashSet<>();

    public GroupsSearchMemberOfStrategy(ILdapConnectionEx connection, List<ILdapEntry> ldapEntries, String userDomain, String joinedDomain, boolean includePrimaryGroup, ILdapSchemaMapping schemaMapping,
            IIdmAuthStatRecorder authStatRecorder) {
        super(schemaMapping, joinedDomain);

        this.ldapEntries = ldapEntries;
        this.userDomain = userDomain;
        this.authStatRecorder = authStatRecorder;
        this.includePrimaryGroup = includePrimaryGroup;
        this.schemaMapping = schemaMapping;
        this.connection = connection;
        this.toProcessGroups = new Stack<>();

        ATTR_NAME_SAM_ACCOUNT = schemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        ATTR_MEMBER_OF = schemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf);
        ATTR_PRIMARY_GROUP_ID = schemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId);
        ATTR_OBJECT_SID = schemaMapping.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
        ATTR_DESCRIPTION = schemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
        ATTR_GROUP_TYPE = schemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupType);
    }

    private void populateGroupsToProcess() throws Exception {
        if (ldapEntries == null || ldapEntries.size() == 0)
            return;

        List<String> userGroups = new ArrayList<String>();
        String primaryGroupDN = null;
        for (ILdapEntry entry : ldapEntries) {
            LdapValue[] memberOfValues = entry.getAttributeValues(ATTR_MEMBER_OF);

            if (memberOfValues != null)
                userGroups.addAll(Arrays.asList(ServerUtils.getMultiStringValue(memberOfValues)));
        }

        if (includePrimaryGroup && ldapEntries.size() == 1) {
            ILdapEntry userLdapEntry = ldapEntries.get(0);
            int primaryGroupRID = ServerUtils.getIntValue(userLdapEntry.getAttributeValues(ATTR_PRIMARY_GROUP_ID));
            byte[] userObjectSID = ServerUtils.getBinaryValue(userLdapEntry.getAttributeValues(ATTR_OBJECT_SID));

            String groupSearchBaseDn = ServerUtils.getDomainDN(userDomain);
            primaryGroupDN = getPrimaryGroupDN(groupSearchBaseDn, userObjectSID, primaryGroupRID,
                    authStatRecorder);

            if (ServerUtils.isNullOrEmpty(primaryGroupDN)) {
                throw new IllegalStateException("Error: Found empty Primary Group DN");
            }

            userGroups.add(primaryGroupDN);
        }

        for (String groupDn : userGroups) {
            if (!processedGroups.contains(groupDn))
            {
                toProcessGroups.push(groupDn);
                addProcessedGroup(groupDn);
            }
        }

        ldapEntries.clear();
    }

    /**
     * This method constructs groups filters by putting together groups with common path. Returned map has common path as key (like cn=Users,dc=ssolabs,dc=com) and a list of filters as value.
     * A filter is like  (&(|(cn=group1)(cn=group2))(objectClass=group))
     * When there are many groups to search there is a limit to the search clauses that can be added to one filter (DEFAULT_FILTER_CLAUSE_COUNT) and in that case more than one filter can be to added the filter list.
     */
    @Override
    public Map<String, List<String>> getFilterByDomain() {
        String suffix = "(objectClass=group)";
        String clauseAttribute = "(";
        Map<String, List<String>> domainFilterClauseValues = new HashMap<>();
        Map<String, List<String>> domainFilter = new HashMap<>();
        if (toProcessGroups != null) {
            while (toProcessGroups.size() > 0) {

                String domainDn = toProcessGroups.pop();

                String[] groupDnParts = domainDn.split(",");
                // expected to have the group DN which should be like CN=,DC=
                if (groupDnParts.length < 2)
                    continue;

                String groupBaseDn = ServerUtils.getDomainDN(ServerUtils.getDomainFromDN(domainDn));
                String cn = groupDnParts[0];

                if (domainFilterClauseValues.containsKey(groupBaseDn)) {
                    domainFilterClauseValues.get(groupBaseDn).add(cn);
                } else {
                    List<String> attributeValues = new ArrayList<String>();
                    attributeValues.add(cn);
                    domainFilterClauseValues.put(groupBaseDn, attributeValues);
                }
            }
        }
        for (String groupDn : domainFilterClauseValues.keySet()) {
            domainFilter.put(groupDn, getFilters(suffix, domainFilterClauseValues.get(groupDn), clauseAttribute));

        }

        return domainFilter;
    }

    @Override
    public boolean useGCConnection() {
        return false;
    }

    @Override
    public String getConnectionDomain() {
        return userDomain;
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

        ldapGroupAttributes.add(ATTR_MEMBER_OF);

        return ldapGroupAttributes;
    }

    @Override
    public void addParents(ILdapEntry entry) {
        String[] parentGroups = ServerUtils.getMultiStringValue(entry.getAttributeValues(ATTR_MEMBER_OF));
        if (parentGroups != null) {
            for (String gDn : parentGroups) {
                if (!processedGroups.contains(gDn) && !ServerUtils.isNullOrEmpty(gDn)) {
                    toProcessGroups.push(gDn);
                    addProcessedGroup(gDn);
                }
            }
        }
    }

    @Override
    public boolean excludeGroup(ILdapEntry groupEntry, String groupObjectSid) {
        //filter out groups that have same name but are from different container
        if (!processedGroups.contains(groupEntry.getDN()))
          return true;
        return super.excludeGroup(groupEntry, groupObjectSid);
    }

    private String getPrimaryGroupDN(String searchBaseDn, byte[] userObjectSID,
            int groupRID, IIdmAuthStatRecorder authStatRecorder) throws NoSuchGroupException {
        SecurityIdentifier sid = SecurityIdentifier.build(userObjectSID);

        sid.setRID(groupRID);

        String groupSidString = sid.toString();

        String[] attrNames = {};

        String filter = String.format(schemaMapping.getGroupQueryByObjectUniqueId(),
                LdapFilterString.encode(groupSidString));

        long startTime = System.nanoTime();

        ILdapMessage message = connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE, filter, attrNames, true);

        if (authStatRecorder != null) {
            long elapsed = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime);
            authStatRecorder.add(new LdapQueryStat(filter, searchBaseDn, getConnectionString(connection), elapsed, 1));
        }

        try {
            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0) {
                throw new NoSuchGroupException(String.format("Group was not found. GroupSID= '%s'.", groupSidString));
            } else if (entries.length != 1) {
                throw new IllegalStateException("Entry length >1");
            }

            return entries[0].getDN();
        } finally {
            if (message != null)
                message.close();
        }
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

    @Override
    public void addLdapEntriesToProcess(List<ILdapEntry> ldapEntries) {
        ldapEntries.addAll(ldapEntries);
    }
}
