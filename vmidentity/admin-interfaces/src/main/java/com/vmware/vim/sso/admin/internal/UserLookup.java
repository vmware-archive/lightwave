/*
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
 */
package com.vmware.vim.sso.admin.internal;

/**
 * Internal component, extract of the
 * {@link com.vmware.vim.sso.admin.PrincipalDiscovery PrincipalDiscovery} API.
 * Facilitates resolution and id normalization of users. Used between SSO admin
 * server and Lookup Service.
 */
public interface UserLookup {

   /**
    * Lookup for a user and return the normalized user principal name.
    *
    * Normalized user principal name consists of: the canonical user name,
    * followed by the "@" symbol, followed by the user's canonical domain name
    * (not domain alias). The proper name canonical form is the exact name which
    * has been provided on user/group creation in the respective domain. Domain
    * name canonical form is the exact name which has been provided on domain
    * registration in SSO.
    * <p>
    * Search is case insensitive with respect to domain name and domain-specific
    * sensitive with respect to user name. Alternative domain names cannot
    * be used.
    *
    * @param name
    *           User's proper name. Required.
    *
    * @param domain
    *           Domain in which to search for the user. Required.
    *
    * @return Normalized user principal name, if the user is found, or
    *         {@code null} otherwise.
    */
   public String lookup(String name, String domain);
}
