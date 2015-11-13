/* **********************************************************************
 * Copyright 2013 VMware, Inc.  All rights reserved.
 * *********************************************************************/
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
