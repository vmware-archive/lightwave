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

import java.util.List;
import java.util.Map;

import com.vmware.identity.interop.ldap.ILdapEntry;

/**
 * An interface for the strategy for group search. Based on different option it returns the search baseDn, filter, attributes and if connection should be to Global Catalog
 *
 */
interface GroupSearchOptionStrategy
{
    /**
     * Search filters that are constructed by domain and by groups
     *
     */
    public Map<String, List<String>> getFilterByDomain();

    /**
     * Returns true if the connection should be to the Global catalog and false otherwise
     *
     * @return
     */
    public boolean useGCConnection();

    /**
     * Returns the domain used to connect to the server
     * @return
     */
    public String getConnectionDomain();

    /**
     * Attributes that has to be returned by the search
     *
     * @param includeDescription
     * @return
     */
    public List<String> getGroupSearchAttributes(boolean includeDescription);

    /**
     * Adds the parents to be processed
     *
     */
    public void addParents(ILdapEntry entry);

    /**
     * Adds ldapEntries to be processed
     *
     * @param ldapEntries
     */
    public void addLdapEntriesToProcess(List<ILdapEntry> ldapEntries);

    /**
     * Exclude group (eg. exclude well known SIDs)
     * @param groupEntry
     * @param groupObjectSid
     * @return
     */
    public boolean excludeGroup(ILdapEntry groupEntry, String groupObjectSid);

    /**
     * Returns true when there are groups to be processed
     *
     * @return
     * @throws Exception
     */
    public boolean hasGroupsToProcess() throws Exception;

    /**
     * Add dn to the processed groups
     * @param dn
     */
    public void addProcessedGroup(String dn);

    /**
     * Constructs filters by using the clause attribute with clause values.
     * When there are more clauses in one filter then the maximum permitted more filters are constructed
     *
     * @param prefix
     * @param clauseValues
     * @param clauseAttribute
     * @return
     */
    public List<String> getFilters(String prefix, List<String> clauseValues, String clauseAttribute);
}
