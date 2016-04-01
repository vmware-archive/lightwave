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
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.provider.ILdapSchemaMapping;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.LdapFilterString;

public abstract class GroupSearchDefaultStrategy implements GroupSearchOptionStrategy{

    private final Set<String> WELL_KNOWN_GROUP_SIDS = Collections.unmodifiableSet(new HashSet<>(Arrays.asList("S-1-1-0", "S-1-2-0", "S-1-2-1", "S-1-3-4", "S-1-5-80-0", "S-1-5-1", "S-1-5-2", "S-1-5-3", "S-1-5-4", "S-1-5-6", "S-1-5-7", "S-1-5-9", 
            "S-1-5-11", "S-1-5-13", "S-1-5-14", "S-1-5-15", "S-1-5-32-544", "S-1-5-32-545", "S-1-5-32-546", "S-1-5-32-547", "S-1-5-32-548", "S-1-5-32-549", "S-1-5-32-550", "S-1-5-32-551", "S-1-5-32-552", "S-1-5-80-0", "S-1-5-83-0")));
    private static final int DOMAIN_LOCAL_GROUP_TYPE = 0x00000004;
    //Specifies a security group.
    private static final int SECURITY_GROUP_TYPE = 0x80000000;
    private static final int DEFAULT_FILTER_CLAUSE_COUNT = 500;

    private final String joinedDomain;
    private final String ATTR_GROUP_TYPE;

    public GroupSearchDefaultStrategy(ILdapSchemaMapping schemaMapping, String joinedDomain)
    {
        this.joinedDomain = joinedDomain;
        ATTR_GROUP_TYPE = schemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupType);
    }

    /**
     * Exclude well known groups, domain local groups and distribution groups
     */
    @Override
    public boolean excludeGroup(ILdapEntry groupEntry, String groupObjectSid) {
        // Exclude well known groups
        if (WELL_KNOWN_GROUP_SIDS.contains(groupObjectSid))
            return true;

        if (groupEntry == null)
            return false;

        int groupTypes = ServerUtils.getIntegerValue(groupEntry.getAttributeValues(ATTR_GROUP_TYPE));
        String groupDomain = ServerUtils.getDomainFromDN(groupEntry.getDN());
        // Exclude domain local groups which are from a different domain than
        // the joined domain
        if (((groupTypes & DOMAIN_LOCAL_GROUP_TYPE) != 0) && (groupDomain != null) && (joinedDomain != null)
                && (!groupDomain.equalsIgnoreCase(joinedDomain)))
            return true;

        //If this flag is not set, then the group is a distribution group.
        if ((groupTypes & SECURITY_GROUP_TYPE) == 0)
            return true;

        return false;
    }

    @Override
    public List<String> getFilters(String suffix, List<String> clauseValues, String clauseAttribute) {
        List<String> filters = new ArrayList<>();
        String prefix = "(&(|";
        StringBuilder filter = new StringBuilder(prefix);

        int i = 0;
        if (clauseValues != null )
        {
            for (String dn : clauseValues) {
                i++;
                filter.append(clauseAttribute);
                filter.append(LdapFilterString.encode(dn));
                filter.append(")");

                if (i > DEFAULT_FILTER_CLAUSE_COUNT)
                {
                    appendSuffix(suffix, filter);
                    filters.add(filter.toString());
                    filter = new StringBuilder(prefix);
                    i = 0;
                }
            }
        }
        if (filter.length() > prefix.length())
        {
            appendSuffix(suffix, filter);
            filters.add(filter.toString());
        }

        return filters;
    }

    private void appendSuffix(String sufix, StringBuilder filter) {
        filter.append(")");
        filter.append(sufix);
        filter.append(")");
    }
}
