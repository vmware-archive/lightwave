/* **********************************************************************
 * Copyright 2013 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.net.URI;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Represents the details of an Ldap Identity Source.
 *
 * @see LdapIdentitySource
 */
public final class LdapIdentitySourceDetails {

   private final String _friendlyName;
   private final String _userBaseDn;
   private final String _groupBaseDn;
   private final URI _primaryUrl;
   private final URI _failoverUrl;
   private final int _searchTimeoutSeconds;

   /**
    * Create an LdapIdentitySourceDetails instance.
    *
    * @param friendlyName
    *           The human-friendly name of the LDAP IdS. This name is also used as
    *           a handle for this IdS for management operations thus it must be
    *           unique for each Domain. Cannot be {@code null}.
    *
    * @param userBaseDn
    *           The base Distinguished Name (DN) the SSO Server will use when
    *           searching for a user in the LDAP IdS.
    *
    * @param groupBaseDn
    *           The base Distinguished Name (DN) the SSO Server will use when
    *           searching for a group in the LDAP IdS.
    *
    * @param primaryUrl
    *           The address of the primary external server the SSO will use as a
    *           user and group store for this LDAP IdS. Cannot be {@code null}.
    *
    * @param failoverUrl
    *           The address of a failover server the SSO Server may use for this
    *           LDAP IdS in the case the primary server is unreachable. May be
    *           {@code null} which indicates no failover url.
    *
    * @param searchTimeoutSeconds
    *           The maximum number of seconds the SSO Server will wait for the
    *           remote server to respond to a search query. Must be positive or
    *           zero (unlimited).
    */
   public LdapIdentitySourceDetails(String friendlyName, String userBaseDn,
      String groupBaseDn, URI primaryUrl, URI failoverUrl,
      int searchTimeoutSeconds) {

      ValidateUtil.validateNotEmpty(friendlyName, "friendlyName");
      ValidateUtil.validateNotNull(primaryUrl, "primaryUrl");
      if (searchTimeoutSeconds < 0) {
         throw new IllegalArgumentException(
            "'searchTimeoutSeconds' value should be positive; actual value is "
               + searchTimeoutSeconds);
      }

      _friendlyName = friendlyName;
      _userBaseDn = userBaseDn;
      _groupBaseDn = groupBaseDn;
      _primaryUrl = primaryUrl;
      _failoverUrl = failoverUrl;
      _searchTimeoutSeconds = searchTimeoutSeconds;
   }

   /**
    * Returns the human-friendly name of the Domain. This name must be unique
    * for each Domain.
    */
   public String getFriendlyName() {
      return _friendlyName;
   }

   /**
    * Returns the base Distinguished Name (DN) the SSO Server will use when
    * searching for a user in the Domain.
    */
   public String getUserBaseDn() {
      return _userBaseDn;
   }

   /**
    * Returns the base Distinguished Name (DN) the SSO Server will use when
    * searching for a group in the Domain.
    */
   public String getGroupBaseDn() {
      return _groupBaseDn;
   }

   /**
    * Returns the address of the primary external server the SSO will use as a
    * user and group store for this Domain.
    */
   public URI getPrimaryUrl() {
      return _primaryUrl;
   }

   /**
    * Returns the address of a failover server the SSO Server may use for this
    * Domain in the case the primary server is unreachable.
    */
   public URI getFailoverUrl() {
      return _failoverUrl;
   }

   /**
    * Returns the maximum number of seconds the SSO Server will wait for the
    * remote server to respond to a search query.
    */
   public int getSearchTimeoutSeconds() {
      return _searchTimeoutSeconds;
   }

   @Override
   public String toString() {
      StringBuilder objString = new StringBuilder(100);
      objString.append(super.toString());

      objString.append(" friendlyName=");
      objString.append(_friendlyName);
      objString.append(", userBaseDn=");
      objString.append(_userBaseDn);
      objString.append(", groupBaseDn=");
      objString.append(_groupBaseDn);
      objString.append(", primaryUrl=");
      objString.append(_primaryUrl);
      objString.append(", failoverUrl=");
      objString.append(_failoverUrl);
      objString.append(", searchTimeoutSeconds=");
      objString.append(_searchTimeoutSeconds);
      return objString.toString();

   }

}
