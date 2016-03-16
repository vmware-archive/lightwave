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
package com.vmware.vim.sso.admin;

import java.util.Collections;
import java.util.Set;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;
import com.vmware.vim.sso.admin.util.NamedValue;

/**
 * Represents an Identity Source accessed through LDAP.
 *
 * The schema is implied from the {@link Type}.
 */
public final class LdapIdentitySource extends IdentitySource {

   /**
    * The type of the LDAP IdS, either KnownType or OtherType
    */
   public interface Type {
      String name();
   }

   /**
    * The supported external server types which can be used as LDAP
    * identity sources by SSO server.
    */
   public static enum KnownType implements Type {
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

   }

   /**
    * LDAP IdS type not covered by this interface specification
    *
    * <p>Such instance may be present when an older client receives
    * response from a newer server.
    */
   public static final class OtherType extends NamedValue implements Type {

      private OtherType(String name) {
         super(name);
      }

      public static Type valueOf(String name) {
         return NamedValue.valueOf(name, KnownType.class, new OtherType(name));
      }
   }
   /**
    * The type of the LDAP IdS authentication, either KnownAuthenticationType
    * or OtherAuthType
    */
   public interface AuthenticationType {
      String name();
   }

   /**
    * The supported methods by which the SSO Server may authenticate against the
    * external servers when searching for users and groups.
    */
   public static enum KnownAuthenticationType implements AuthenticationType {
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
       * belongs to (i.e. a Windows Domain user, the Local System account or
       * the Network Service account).
       */
      reuseSession
   }

   /**
    * LDAP IdS authentication type not covered by this interface specification
    *
    * <p>Such instance may be present when an older client receives
    * response from a newer server.
    */
   public static final class OtherAuthType
      extends NamedValue implements AuthenticationType {

      private OtherAuthType(String name) {
         super(name);
      }

      public static AuthenticationType valueOf(String name) {
         return NamedValue.valueOf(name,
            KnownAuthenticationType.class, new OtherAuthType(name));
      }
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
         if (KnownAuthenticationType.password.equals(authenticationType)) {
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

   private final Type _type;
   private final LdapIdentitySourceDetails _details;
   private final AuthenticationDetails _authenticationDetails;

   /**
    * Populate an instance
    *
    * @param domain
    *           The single domain of the IdS. Cannot be {@code null}
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
   public LdapIdentitySource(Domain domain, Type type,
      LdapIdentitySourceDetails details, AuthenticationDetails authenticationDetails) {
      super(getName(domain), Collections.singleton(domain));

      ValidateUtil.validateNotNull(type, "type");
      ValidateUtil.validateNotNull(details, "details");
      ValidateUtil.validateNotNull(authenticationDetails,
         "authenticationDetails");

      _type = type;
      _details = details;
      _authenticationDetails = authenticationDetails;
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
   public LdapIdentitySourceDetails getDetails() {
      return _details;
   }

   /**
    * Returns details about the authentication method the SSO server will use
    * with the external server corresponding to this external domain.
    */
   public AuthenticationDetails getAuthenticationDetails() {
      return _authenticationDetails;
   }

   @Override
   public String toString() {
      StringBuilder objString = new StringBuilder(100);
      objString.append(super.toString());
      objString.append(", type=");
      objString.append(getType());
      objString.append(", authenticationDetails=(");
      objString.append(getAuthenticationDetails());
      objString.append("), details=(");
      objString.append(getDetails());
      objString.append(")");
      return objString.toString();
   }

   // equals and hashcode of the parent are correct for this class as well

   private static String getName(Domain domain) {
      ValidateUtil.validateNotNull(domain, "domain");
      return domain.getName();
   }
}
