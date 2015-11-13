/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.net.URI;
import java.security.cert.CertPath;
import java.util.List;
import java.util.Set;

import com.vmware.vim.sso.admin.RoleManagement.NoPrivilege;
import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.DirectoryServiceConnectionException;
import com.vmware.vim.sso.admin.exception.DomainNotFoundException;
import com.vmware.vim.sso.admin.exception.DuplicateDomainNameException;
import com.vmware.vim.sso.admin.exception.LocalOSDomainRegistrationException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;
import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Provides interface for listing all SSO Domains and creating, updating and
 * deleting External Domains.
 * <p>
 * A "Domain" is any store for users and groups the SSO server uses. There are
 * two Domain types:
 * <ul>
 * <li>The <b>system</b> domain, where the principals are stored locally.</li>
 * <li>The <b>external</b> domains, where the principals are stored on an
 * external server. Refer to the {@link ExternalDomain.Type} enumeration for the
 * list of supported external server types.
 * </ul>
 * <p>
 * Note that the there is always exactly one system domain which cannot be
 * changed in any way, while there may be many (incl. none) external domains
 * attached at the same time; they can be added, changed or removed.
 * <p>
 * Note also that this interface only allows operations with the external
 * domains themselves, but the users and groups stored in these domains cannot
 * be modified. The content of the external domain is read-only for the SSO
 * Server.
 */
public interface DomainManagement {

   /**
    * Represents a user name + password tuple for the SSO Server to use when
    * authenticating against external server.
    * <p>
    * Neither the user name nor the password are allowed to be {@code null}.
    *
    * @see ExternalDomain.AuthenticationType
    */
   public static final class AuthenticationCredentails {
      private final String _username;
      private final char[] _password;
      private final boolean _useMachineAccount;
      private final String _spn;

      /**
       * Create an AuthenticationCredentails instance using the provided user
       * name and password.
       *
       * @param username
       *           user name; {@code not-null} and not empty string value is
       *           required
       * @param password
       *           user password; {@code not-null} value is required
       */
      public AuthenticationCredentails(String username, char[] password) {
         ValidateUtil.validateNotEmpty(username, "username");
         ValidateUtil.validateNotNull(password, "password");

         _username = username;
         _password = password;
         _useMachineAccount = false;
         _spn = null;
      }

      /**
       * Create an AuthenticationCredentails instance using the provided user
       * name and password.
       *
       * @param username
       *           user name; {@code not-null} and not empty string value is
       *           required
       * @param password
       *           user password; {@code not-null} value is required
       * @param spn
       *           user service principal name; {@code not-null} and not empty
       *           string value is value is required
       */
      public AuthenticationCredentails(String username, char[] password, boolean useMachineAccount, String spn) {
         if (!useMachineAccount)
         {
             ValidateUtil.validateNotEmpty(username, "username");
             ValidateUtil.validateNotNull(password, "password");
         }

         _username = username;
         _password = password;
         _useMachineAccount = useMachineAccount;
         _spn = spn;
      }

      /**
       * @return The user name to authenticate with. {@code not-null} and not
       *         empty string value.
       */
      public String getUsername() {
         return _username;
      }

      /** @return The password to authenticate with. {@code not-null} value. */
      public char[] getPassword() {
         return _password;
      }

      /** @return The service principal name to obtain TGS with. could not null. */
      public String getSpn() {
         return _spn;
      }

      /** @return Whether the machine account should be used. */
      public boolean useMachineAccount() {
         return _useMachineAccount;
      }
   }

   /**
    * Probe the connectivity to the specified LDAP/NIS service and try to
    * authenticate using the provided credentials.
    *
    * @param serviceUri
    *           The URI of the target service. The scheme must be {@code ldap},
    *           {@code ldaps} or {@code nis}. If the scheme is {@code ldaps}
    *           (LDAP Secure), the remote server's SSL certificate must be
    *           verifiable using the SSO Server's SSL trust store (or the
    *           connection will fail). Cannot be <code>null</code>.
    *
    * @see #getSslCertificateManagement
    *
    * @param authenticationType
    *           The authentication method to use. If {@code
    *           AuthenticationType#anonymous}, no authentication will be
    *           attempted.
    *
    * @param authenticationCredentials
    *           If {@code authenticationType} is {@code
    *           AuthenticationType#password}, the credentials (username +
    *           password) to use for authentication. Otherwise, should be
    *           {@code null}.
    *
    * @throws DirectoryServiceConnectionException
    *            Indicates the SSO Server either failed to connect or to
    *            authenticate to the service at the specified URI.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void probeConnectivity(URI serviceUri,
      ExternalDomain.AuthenticationType authenticationType,
      AuthenticationCredentails authenticationCredentials)
      throws DirectoryServiceConnectionException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Register new external server as a Domain for the SSO Server.
    *
    * @param serverType
    *           The type of the external server. See the
    *           {@link ExternalDomain.Type} enumeration.
    *
    * @param domainName
    *           The name to associate with the created Domain. The domain name
    *           must be unique in the sense that no other Domain may have the
    *           same name <i>or</i> alias. The association is permanent, i.e.
    *           the name cannot be changed once the Domain is created. When
    *           querying, this name shall be returned in the same case it was
    *           entered when creating the Domain but it is case-insensitive for
    *           the purpose of all Domain operations. Cannot be
    *           <code>null</code> or empty.
    *
    * @param domainAlias
    *           An optional alias to associate with the created Domain. If given
    *           (i.e. non-{@code null}), it must be non-empty string. Like with
    *           the Domain name, the Domain alias (if set) may not be reused for
    *           any other Domain's name or alias and may not be changed. ; When
    *           querying, this alias shall be returned in the same case it was
    *           entered when creating the Domain but it is case-insensitive for
    *           the purpose of all Domain operations.
    *
    * @param details
    *           Details for the Domain. Cannot be <code>null</code>. Field
    *           constraints: primary and failover URL should be different.
    *
    * @param authenticationType
    *           Specifies how the SSO server should authenticate itself to the
    *           external server when searching for users. See the
    *           {@link ExternalDomain.AuthenticationType} enumeration.
    *
    * @param authnCredentials
    *           Credentials (user name and password) for the SSO Server to use
    *           when authenticating to the external server. Meaningful only if
    *           the {@code authenticationType} is
    *           {@link ExternalDomain.AuthenticationType#password}. Otherwise it
    *           must be null.
    *
    * @throws DuplicateDomainNameException
    *            Indicates that the supplied name or alias is already associated
    *            with another Domain.
    * @throws DirectoryServiceConnectionException
    *            Indicates the SSO Server either failed to connect or to
    *            authenticate to the service at the specified URI.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @see ExternalDomain.AuthenticationType
    * @see AuthenticationCredentails
    */
   @Privilege(Role.Administrator)
   void addExternalDomain(ExternalDomain.Type serverType, String domainName,
         String domainAlias, ExternalDomainDetails details,
         ExternalDomain.AuthenticationType authenticationType,
         AuthenticationCredentails authnCredentials)
         throws DirectoryServiceConnectionException,
         DuplicateDomainNameException, NotAuthenticatedException,
         NoPermissionException;

   /**
    * Register the domain managed by the local OS. Local OS domain can be
    * registered at most once.
    *
    * @param domainName
    *           The name to associate with the created Domain. The domain name
    *           must be unique in the sense that no other Domain may have the
    *           same name, alias <i>or</i> friendly name (located at
    *           {@link ExternalDomainDetails}). The association is permanent,
    *           i.e. the name cannot be changed once the Domain is created. When
    *           querying, this name shall be returned in the same case it was
    *           entered when creating the Domain but it is case-insensitive for
    *           the purpose of all Domain operations. Cannot be
    *           <code>null</code> or empty.
    *
    * @throws DuplicateDomainNameException
    *            Indicates that the supplied name or alias is already associated
    *            with another Domain.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws LocalOSDomainRegistrationException
    *            when the local OS domain has already been registered or when
    *            SSO server is installed in mode which does not support local OS
    *            domain
    */
   @Privilege(Role.Administrator)
   void registerLocalOSDomain(String domainName)
      throws DuplicateDomainNameException, NotAuthenticatedException,
      NoPermissionException, LocalOSDomainRegistrationException;

   /**
    * @return Information about all registered Domains.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @see Domains
    */
   @Privilege(Role.RegularUser)
   Domains getDomains() throws NotAuthenticatedException, NoPermissionException;

   /**
    * Get the system domain name where local principals are stored
    *
    * @return domain name; {@code non-null} and not-empty string value
    */
   @NoPrivilege
   String getSystemDomainName();

   /**
    * Get the system tenant name
    *
    * @return system tenant name; {@code non-null} and not-empty string value
    */
   @NoPrivilege
   String getSystemTenantName();

   /**
    * Get the local OS domain name
    *
    * @return domain name or {@code null} if the local OS domain has not been
    *         registered
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   String getLocalOSDomainName() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Find an external domain by its associated name or alias. If no external
    * domain is associated with this name / alias, {@code null} will be
    * returned. The find is case-insensitive.
    *
    * @param filter
    *           The name or alias of the external domain to retrieve. Cannot be
    *           <code>null</code> or empty.
    *
    * @return A data object representing the found Domain (or {@code null} if
    *         none is found).
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   ExternalDomain findExternalDomain(String filter)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Set the brand name for the tenant.
    *
    * @param brandName
    *           can be null or empty
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setBrandName(String brandName) throws NotAuthenticatedException,
         NoPermissionException;

   /**
    * Retrieve the brand name for the tenant
    *
    * @return the brand name or {@code null} if not set
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   String getBrandName() throws NotAuthenticatedException,
         NoPermissionException;

   /**
    * Set the logon banner for the tenant.
    *
    * @param logonBanner
    *           can be null or empty
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setLogonBanner(String logonBanner) throws NotAuthenticatedException, NoPermissionException;

   /**
    * Retrieve the logon banner for the tenant
    *
    * @return the logon banner or {@code null} if not set
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   String getLogonBanner() throws NotAuthenticatedException, NoPermissionException;

   /**
    * Update the details of the Domain associated with the given name.
    *
    * @param name
    *           The name of the Domain to update; the lookup is
    *           case-insensitive. Cannot be <code>null</code> or empty.
    *
    * @param details
    *           The new properties of the Domain. Cannot be <code>null</code>
    *
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            Domain.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void updateExternalDomainDetails(String name, ExternalDomainDetails details)
      throws DomainNotFoundException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Updates the way the SSO Server authenticates against the external server
    * corresponding to a Domain.
    *
    * @param name
    *           The name of the Domain to update; the lookup is
    *           case-insensitive. Cannot be <code>null</code> or empty.
    *
    * @param authnType
    *           The desired authentication type. See the
    *           {@link ExternalDomain.AuthenticationType} enumeration.
    *
    * @param authnCredentials
    *           A user name + password tuple if the authnType is
    *           {@link ExternalDomain.AuthenticationType#password} or {@code
    *           null} for any other authentication type.
    *
    * @throws DomainNotFoundException
    *            Indicates that the specified domain name is not associated with
    *            any Identity Source.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws DirectoryServiceConnectionException
    *            when the probe connectivity test fails
    */
   @Privilege(Role.Administrator)
   void updateExternalDomainAuthnType(String name,
      ExternalDomain.AuthenticationType authnType,
      AuthenticationCredentails authnCredentials)
      throws DomainNotFoundException, NotAuthenticatedException,
      NoPermissionException, DirectoryServiceConnectionException;

   /**
    * Register an UPN suffix for the specific domain
    *
    * @param domainName
    *           domain name the UPN suffix will be applied to. Cannot be null or
    *           empty
    * @param upnSuffix
    *           a suffix to be registered, such as "@gmail.com" in user
    *           "joe@gmail.com" of tenant "cook.com". cannot be null or empty
    * @return whether the state has been changed. In other words, true if the
    *         UPN suffix didn't exist and is added successfully; false if it
    *         already exists or operation was not successful but return
    *         gracefully.
    *         <p>
    *         Registering UpnSuffix for an OpenLdap/ADOverLdap/NativeAD IDP domain
    *         is not allowed, return value {@code false}.
    *         <p>
    *         The UPN suffix is not case-sensitive.
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            Domain.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean registerUpnSuffix(String domainName, String upnSuffix)
         throws DomainNotFoundException, NotAuthenticatedException,
         NoPermissionException;

   /**
    * Un-register an UPN suffix for the specific domain
    *
    * @param domainName
    *           domain name the UPN suffix will be applied to. Cannot be null or
    *           empty
    * @param upnSuffix
    *           a suffix that to be unregistered, such as "@gmail.com" in user
    *           "joe@gmail.com" of tenant "cook.com". cannot be null or empty
    * @return whether the state has been changed. In other words, true if the
    *         UPN suffix exists and is removed successfully; false if it doesn't
    *         exist or operation was not successful but return gracefully.
    *         <p>
    *         The UPN suffix is not case-sensitive.
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            Domain.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean unRegisterUpnSuffix(String domainName, String upnSuffix)
         throws DomainNotFoundException, NotAuthenticatedException,
         NoPermissionException;

   /**
    * Query UPN suffixes list for the specific domain
    *
    * @param domainName
    *           domain name for which UPN suffixes to be retrieves. Cannot be null or
    *           empty
    * @return set of strings for the suffixes or {@code null} if none registered
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            Domain.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Set<String> getUpnSuffixes(String domainName)
         throws DomainNotFoundException, NotAuthenticatedException,
         NoPermissionException;

   /**
    * Remove the Domain associated with the given name.
    *
    * @param name
    *           The name of the Domain to unregister; the lookup is
    *           case-insensitive. Cannot be <code>null</code> or empty.
    *
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            Domain.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void deleteDomain(String name) throws DomainNotFoundException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Returns the interface for management of the SSL trust store.
    * <p>
    * When establishing secure (SSL/TLS) connections with the external domain's
    * server, the SSO server will try to verify the remote certificate with any
    * CA certificate from the SSL trust store. Each certificate in the trust
    * store should be root (self-signed) CA certificate.
    * <p>
    * Note that while technically it is possible with the returned manager to
    * add non-self-signed or leaf certificates to the SSL trust store, doing so
    * has no effect.
    * <p>
    * Also be aware that the certificates in the SSL trust store are only
    * relevant for establishing secure connections with external servers. They
    * are <b>never used</b> for validating tokens or for solution
    * authentication.
    *
    * @return A CertificateManagement instance for managing the SSL trust store.
    */
   @NoPrivilege
   CertificateManagement getSslCertificateManagement();

   /**
    * Specify default domains. Users can authenticate against default domain
    * even when a user name is provided without domain name. <br>
    * E.g. if {@code mike@vmware.com} is trying to authenticate providing
    * {@code mike} as a principal name (and correct credentials for {@code
    * mike@vmware.com}), then if:
    * <ul>
    * <li>{@code vmware.com} is among default domains - authentication will
    * succeed.
    * <li>{@code vmware.com} is not among default domains - authentication will
    * fail.
    * </ul>
    * If a given username is present in more than one of the default domains,
    * authentication will succeed for the first domain which accepts the
    * credentials provided.
    *
    * @param domainNames
    *           Can be <code>null</code>. Domain list ordered by priority. <br>
    *           When a value is not provided the default domains list will be
    *           cleared.
    * @throws DomainNotFoundException
    *            when a provided domain name is not in the list of registered
    *            domains, see {@link #getDomains()}.
    * @throws DuplicateDomainNameException
    *            when the provided list contains an element more than once.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setDefaultDomains(List<String> domainNames)
      throws DomainNotFoundException, DuplicateDomainNameException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * @see #setDefaultDomains
    *
    * @return Default domains ordered by priority, or empty {@link List} if no
    *         default domains are configured.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   List<String> getDefaultDomains() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Retrieve the SSL identity of an LDAPS endpoint located at the specified
    * host ( and optional port ). The SSL chain is retrieved as provided by
    * endpoint with no guarantees to be valid!
    * <p>
    * A {@code null} value will be returned in any of the following cases:
    * <ul>
    * <li>parties cannot agree on which cipher suite to use during SSL handshake
    * </li>
    * <li>the cipher suite being used is not certificate-based, such as Kerberos
    * </li>
    * <li>the domain has no certificate configured</li>
    * </ul>
    *
    * @param host
    *           a valid host name or IP address exposed and accessible via
    *           LDAPS; {@code not-null} and {@code not-empty} string value is
    *           required
    * @param ldapsPort
    *           port number or -1 for the default LDAPS port
    * @return SSL identity or {@code null}
    *
    * @throws DirectoryServiceConnectionException
    *            when I/O exception occurs during SSL socket creation with the
    *            domain's primary URL
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   CertPath getSslIdentity(String host, int ldapsPort)
      throws DirectoryServiceConnectionException, NotAuthenticatedException,
      NoPermissionException;
}
