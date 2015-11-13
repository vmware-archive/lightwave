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
package com.vmware.identity.idm;

import java.io.Serializable;

import org.apache.commons.lang.Validate;

public final class SearchCriteria implements Serializable {

    /**
     * Serial version id
     */
    private static final long serialVersionUID = 6614665725412804056L;

    /** Search string */
    private final String searchString;

    /** Domain name ( e.g. VMWARE.COM ) */
    private final String domain;

    /**
     * Create search criteria
     *
     * @param searchString
     *           refer to {@link #getSearchString()}. requires {@code
     *           not-null} value
     * @param domain
     *           domain name as described at {@link PrincipalId#getDomain()};
     *           {@code not-null} and not-empty string value is required
     */
    public SearchCriteria(String searchString, String domain) {

       Validate.notNull(searchString, "searchString");
       Validate.notNull(domain, "domain");

       this.searchString = searchString;
       this.domain = domain;
    }

    /**
     * Retrieve the search string.
     * <p>
     * Search string used to find principals ( users and groups ). Matching
     * principals will be those that have this particular string as a
     * substring either at {@code name} property of their principal ID, or at:
     * <ul>
     * <li>{@code firstName} or {@code lastName} property - for
     * {@link PersonUser}</li>
     * <li>{@code description} property - for {@link Group}</li>
     * </ul>
     *
     * @return the search string; empty string means no restriction; {@code
     *         not-null} value
     *
     * @see PrincipalId#getName() name property
     * @see PersonDetails#getFirstName() firstName property
     * @see PersonDetails#getLastName() lastName property
     * @see GroupDetails#getDescription() description property
     */
    public String getSearchString() {
       return this.searchString;
    }

    /**
     * Retrieve the domain name where to search
     *
     * @return the domain name; {@code not-null} and not-empty string value
     */
    public String getDomain() {
       return this.domain;
    }

    @Override
    public String toString() {
       return String.format("searchString=%s, domain=%s", this.searchString,
          this.domain);
    }
}
