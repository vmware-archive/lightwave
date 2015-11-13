/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Represents an external SSO Domain. The ExternalDomain objects are immutable.
 *
 * @see DomainManagement
 */
public final class ExternalDomain {

   /**
    * The supported external server types which can be used as Domains by the
    * SSO server.
    */
   public static enum Type {
      /**
       * The external server is a Microsoft Active Directory Server. Versions of
       * Active Directory 2003 and later are supported.
       */
      ActiveDirectory,

      /**
       * The external server is an OpenLDAP server. OpenLDAP version 2.4 and
       * later are supported.
       */
      OpenLdap,

      /**
       * The external server is a NIS server.
       */
      NIS

   }

   /**
    * The supported methods by which the SSO Server may authenticate against the
    * external servers when searching for users and groups.
    */
   public static enum AuthenticationType {
      /**
       * The SSO server will use no authentication.
       */
      anonymous,

      /**
       * A combination of a user name and password will be used for
       * authentication.
       */
      password,

      /**
       * The SSO server will reuse the process session credentials to
       * authenticate against the external server.
       * <p>
       * This method is only supported if the external server is of type
       * "Active Directory" and the SSO server runs as a user already
       * authenticated against the same Windows Domain the external server
       * belongs to (i.e. a Windows Domain user, the Local System account of the
       * Network Service account).
       */
      reuseSession
   }

   /**
    * Details about the authentication method the SSO server is configured to
    * use when talking to an external server.
    */
   public static final class AuthenticationDetails {
      private final AuthenticationType _authenticationType;
      private final String _username;

      /**
       * Create an AuthenticationDetails object.
       *
       * @param authenticationType
       *           The authentication method being used. May not be {@code null}
       *           .
       * @param username
       *           The non-empty username used for authentication in the case
       *           the {@code authenticationType} is
       *           {@link AuthenticationType#password} ; {@code null} otherwise.
       */
      public AuthenticationDetails(AuthenticationType authenticationType,
         String username) {

         ValidateUtil.validateNotNull(authenticationType, "authenticationType");
         if (AuthenticationType.password.equals(authenticationType)) {
            ValidateUtil.validateNotEmpty(username, "username");
         }

         _authenticationType = authenticationType;
         _username = username;
      }

      /**
       * The authentication method being used.
       */
      public AuthenticationType getAuthenticationType() {
         return _authenticationType;
      }

      /**
       * The username used for authentication in the case the {@code
       * authenticationType} is {@link AuthenticationType#password} ; {@code
       * null} otherwise.
       *
       * @see AuthenticationType
       */
      public String getUsername() {
         return _username;
      }

      @Override
      public String toString() {
         StringBuilder objString = new StringBuilder(70);
         objString.append(super.toString());
         objString.append(", type=");
         objString.append(getAuthenticationType());
         objString.append(", userName=");
         objString.append(getUsername());
         return objString.toString();
      }

   }

   private final String _name;
   private final String _alias;
   private final Type _type;
   private final ExternalDomainDetails _details;
   private final AuthenticationDetails _authenticationDetails;

   /**
    * Create an ExternalDomain object.
    *
    * @param name
    *           The name associated with the Domain.
    * @param alias
    *           An optional alias associated with the Domain.
    *
    * @param type
    *           The type of the external server.
    *
    * @param details
    *           Information about the server.
    *
    * @param authenticationDetails
    *           Information about the authentication method used by the SSO when
    *           contacting the server.
    */
   public ExternalDomain(String name, String alias, Type type,
      ExternalDomainDetails details, AuthenticationDetails authenticationDetails) {

      ValidateUtil.validateNotEmpty(name, "name");
      ValidateUtil.validateNotNull(type, "type");
      ValidateUtil.validateNotNull(details, "details");
      ValidateUtil.validateNotNull(authenticationDetails,
         "authenticationDetails");

      _name = name;
      _alias = alias;
      _type = type;
      _details = details;
      _authenticationDetails = authenticationDetails;
   }

   /**
    * Returns the name associated with the external domain.
    * <p>
    * The name is unique in the sense that no other Domain may have the same
    * name <i>or</i> alias and cannot be changed after the Domain is created.
    * The name is also case-insensitive so e.g. "DOMAIN.COM" and "domain.com"
    * refer to the <i>same</i> Domain.
    */
   public String getName() {
      return _name;
   }

   /**
    * Returns an optional alias associated with the external domain. If no alias
    * is associated with the domain, {@code null} is returned.
    * <p>
    * No other Domain may have the same name or alias; the alias cannot be
    * changed and is case-insensitive.
    */
   public String getAlias() {
      return _alias;
   }

   /**
    * Returns the type of the external server which stores this external
    * domain's users and groups.
    */
   public Type getType() {
      return _type;
   }

   /**
    * Returns detailed information about the external domain
    *
    * @see ExternalDomainDetails
    */
   public ExternalDomainDetails getDetails() {
      return _details;
   }

   /**
    * Returns details about the authentication method the SSO server will use
    * with the external server corresponding to this external domain.
    */
   public AuthenticationDetails getAuthenticationDetails() {
      return _authenticationDetails;
   }

   /*
    * (non-Javadoc) Two domain instances are considered equal if their
    * corresponding names are case-insensitively equal.
    *
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(Object obj) {
      if (obj == this) {
         return true;
      }

      if (!(obj instanceof ExternalDomain)) {
         return false;
      }

      return _name.equalsIgnoreCase(((ExternalDomain) obj)._name);
   }

   /*
    * (non-Javadoc) The Domain instances are hashed by their lower-cased name.
    *
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {
      return _name.toLowerCase().hashCode();
   }

   @Override
   public String toString() {
      StringBuilder objString = new StringBuilder(100);
      objString.append(super.toString());
      objString.append(", name=");
      objString.append(getName());
      objString.append(", alias=");
      objString.append(getAlias());
      objString.append(", type=");
      objString.append(getType());
      objString.append(", authenticationDetails=(");
      objString.append(getAuthenticationDetails());
      objString.append("), details=(");
      objString.append(getDetails());
      objString.append(")");
      return objString.toString();
   }
}
