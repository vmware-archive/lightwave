/* **********************************************************************
 * Copyright 2013 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.net.URI;
import java.security.cert.CertPath;
import java.util.Arrays;
import java.util.List;

import com.vmware.vim.sso.admin.LdapIdentitySource.KnownAuthenticationType;
import com.vmware.vim.sso.admin.RoleManagement.NoPrivilege;
import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.ADIDPRegistrationServiceException;
import com.vmware.vim.sso.admin.exception.ADIDSAlreadyExistException;
import com.vmware.vim.sso.admin.exception.DirectoryServiceConnectionException;
import com.vmware.vim.sso.admin.exception.DomainManagerException;
import com.vmware.vim.sso.admin.exception.DomainNotFoundException;
import com.vmware.vim.sso.admin.exception.DuplicateDomainNameException;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.InvalidProviderException;
import com.vmware.vim.sso.admin.exception.LocalOSDomainRegistrationException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;
import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Provides operations for configuring Identity Sources to SSO and listing them
 * and their attached domains.
 * <p>
 * Identity Source provide means to attach one or more domains to SSO. Domain is
 * any store for users and groups the SSO server could use. There are four
 * Identity Source types:
 * <ul>
 * <li>The <b>System</b> Identity Source which brings the system domain, where
 * the principals are stored locally. This is the only domain SSO has write
 * access to. See {@link PrincipalManagementService}.</li>
 * <li>The <b>Local OS</b> Identity Source which brings the localOS domain backed
 * by the local Operating System storage.</li>
 * <li>The <b>Native AD</b> Identity Source which brings all domains from the AD
 * topology.</li>
 * <li><b>LDAP</b> Identity Sources which bring the corresponding domain backed
 * by the LDAP server. The supported LDAP servers are AD and OpenLDAP.</li>
 * </ul>
 * Refer to {@link IdentitySources} and {@link GenericIdentitySource} for more
 * information.
 * <p>
 * Note that the System Identity Source cannot be changed in any way, while all
 * the other could be registered and deleted at any time.
 * <p>
 * Note also that this interface only allows configuring identity sources in
 * SSO. The users, groups and domain configuration cannot be modified. The
 * content of the domains besides the System domain is read-only for the SSO
 * Server.
 */
public interface IdentitySourceManagement {

   /**
    * Represents a user name + password tuple for the SSO Server to use when
    * authenticating against LDAP server or AD server.
    * <p>
    * Neither the user name nor the password are allowed to be {@code null}.
    *
    * @see LdapIdentitySource
    */
   public static final class AuthenticationCredentials {
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
       *           required.
       *           <ul>
       *           <li>For LDAP server, it should be DN format; </li>
       *           <li>For AD Server, it should be in UPN format; </li>
       *           </ul>
       * @param password
       *           user password; {@code not-null} value is required
       */
      public AuthenticationCredentials(String username, char[] password) {
         this(username, password, false, null);      }

      /**
       * Create an AuthenticationCredentails instance using the provided user
       * name and password.
       *
       * @param username
       *           user name; {@code not-null} and not empty string value is
       *           required
       *           <ul>
       *           <li>For LDAP server, it should be DN format; </li>
       *           <li>For AD Server, it should be in UPN format; </li>
       *           </ul>
       * @param password
       *           user password; {@code not-null} value is required
       * @param spn
       *           user service principal name; can be null or empty for LDAP IDS.
       */
      public AuthenticationCredentials(String username, char[] password, boolean useMachineAccount, String spn) {
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
         return Arrays.copyOf(_password, _password.length);
      }

      /** @return The service principal name to obtain TGS with. could be null or empty for LDAP server. */
      public String getSpn() {
         return _spn;
      }

      /** @return Whether the machine account must be used */
      public boolean useMachineAccount() {
          return _useMachineAccount;
      }
   }

   /**************
    *
    * CRUD methods
    *
    **************/

   /**
    * Register new LDAP server as an Identity Source and attaches its
    * corresponding Domain to SSO. The supported LDAP servers are OpenLdap and
    * AD.
    * Precondition:
    * <ul>
    * <li>For registration of AD as LDAP server (AD-over-LDAP), it is required that
    * no native AD Identity Source are currently registered</li>
    * </ul>
    *
    * @param serverType
    *           The type of the server. Required.
    *
    * @param domainName
    *           The name to associate with the created Domain. Same name will be
    *           used for the name of the Identity Source. Required.
    *           <p>
    *           The domain name must be unique in the sense that no other Domain
    *           may have the same name <i>or</i> alias. The association is
    *           permanent, i.e. the name cannot be changed once the Domain is
    *           created.
    *           <p>
    *           When querying, this name shall be returned in the same case it
    *           was entered on registration but it is case-insensitive for the
    *           purpose of all domain operations, e.g. authenticate, find.
    *
    * @param domainAlias
    *           An optional alias to associate with the created Domain.
    *           <p>
    *           Like with the Domain name, the Domain alias (if set) may not be
    *           reused for any other Domain's name or alias and may not be
    *           changed.
    *           <p>
    *           When querying, this alias shall be returned in the same case it
    *           was entered when creating the Domain but it is case-insensitive
    *           for the purpose of all Domain operations.
    *
    * @param details
    *           Details for the Domain. Field constraints: primary and fail-over
    *           URL should be different. Required.
    *
    * @param authenticationType
    *           Specifies how the SSO server should authenticate itself to the
    *           LDAP server when searching for users. Required.
    *
    * @param authnCredentials
    *           Credentials (user name and password) for the SSO Server to use
    *           when authenticating to the LDAP server. Meaningful only if the
    *           {@code authenticationType} is
    *           {@link LdapIdentitySource.KnownAuthenticationType#password}.
    *           Otherwise it must be null.
    *
    * @throws DuplicateDomainNameException
    *            Indicates that the supplied name or alias is already associated
    *            with another Domain or an Identity Source with same name
    *            already exists.
    * @throws ADIDSAlreadyExistException
    *            Indicates that, when trying to register an AD server as LDAP,
    *            a nativeAD IDS is already configured.
    * @throws InvalidProviderException
    *            when the provider configuration specified is invalid, such as due to
    *            invalid users baseDN or invalid groups baseDN
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
    * @see LdapIdentitySource.AuthenticationType
    * @see AuthenticationCredentials
    */
   @Privilege(Role.Administrator)
   void registerLdap(LdapIdentitySource.KnownType type, String domainName,
         String domainAlias, LdapIdentitySourceDetails details,
         LdapIdentitySource.KnownAuthenticationType authenticationType,
         AuthenticationCredentials authnCredentials)
         throws DuplicateDomainNameException, NotAuthenticatedException,
         NoPermissionException, DirectoryServiceConnectionException,
         ADIDSAlreadyExistException, InvalidProviderException;

   /**
    * Register an Identity Source of type 'AD-native' and attaches all visible
    * AD domains to SSO. Preconditions:
    * <ul>
    * <li>no registered native AD Identity Sources</li>
    * <li>no registered LDAP AD Identity Sources</li>
    * <li>the SSO server machine is joined to an AD domain</li>
    * </ul>
    * <p>
    * The AD domains attached are:
    * <ul>
    * <li>the domain in which the SSO Server machine is joined</li>
    * <li>all domains from the same forest</li>
    * <li>all domains from other forests with forest trust relationship (one or
    * two way, transitive or non-transitive) with the local forest</li>
    * <li>all domains from other forests reachable through a chain of transitive
    * forest trusts from the local forest</li>
    * </ul>
    *
    * The domains get automatically discovered. Changes in the AD topology will
    * be dynamically applied here. The DNS domain name become SSO domain name,
    * while the NETBIOS domain name - SSO domain alias.
    *
    * @param name
    *           Fully qualified Active Directory Domain Name the SSO machine joins to.
    *           Required.
    * @param authnCredentials
    *           credentials for Active Directory registration. Required.
    * @param schemaMapping
    *           optional attribute mapping.
    * @throws DuplicateDomainNameException
    *            Indicates that there is a name clash either among domain names
    *            or aliases or with the name of other Identity Source.
    * @throws InvalidPrincipalException
    *            when the connectivity test with the provided credential on
    *            target domain fails.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws ADIDPRegistrationServiceException, which is the base type for the following exceptions:
    *         <li>{@code ADIDSAlreadyExistException}
    *            Indicates that there is already a registered native AD IDS
    *            or LDAP AD IDS
    *         </li>
    *         <li>{@code HostNotJoinedRequiredDomainException}
    *            when host is not joined to the required domain
    *         </li>
    *         <li>{@code DomainManagerException}
    *            when the DomainManager API invocation failed
    *         </li>
    * @see GenericIdentitySource
    * @see Domain
    */
   @Privilege(Role.Administrator)
   void registerActiveDirectory(String domainName,
         AuthenticationCredentials authnCredentials,
         ExternalDomainSchemaDetails schemaMapping)
         throws DuplicateDomainNameException,
         NotAuthenticatedException, NoPermissionException,
         InvalidPrincipalException, ADIDPRegistrationServiceException;

   /**
    * Register the local OS as an Identity Source and attaches its corresponding
    * Domain to SSO.
    *
    * @param name
    *           Identity Source name. Required. The same name will be
    *           associated with the attached domain.
    *           <p>
    *           The name must be unique among Identity Source names and being
    *           also a domain name it should be unique among all domain names
    *           and aliases.
    *           <p>
    *           When querying, this name shall be returned in the same case it
    *           was entered on registration but it is case-insensitive for the
    *           purpose of all domain operations, e.g. authenticate, find.
    *
    * @throws DuplicateDomainNameException
    *            Indicates that the supplied name or alias is already associated
    *            with another Domain or an Identity Source with same name
    *            already exists.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the callered
    * @throws LocalOSDomainRegistrationException
    *            when the local OS domain has already been registered or when
    *            SSO server is installed in mode which does not support local OS
    *            domain
    */
   @Privilege(Role.Administrator)
   void registerLocalOS(String name) throws DuplicateDomainNameException,
      NotAuthenticatedException, NoPermissionException,
      LocalOSDomainRegistrationException;

   /**
    * Provides Information about all registered Identity Sources along with
    * domains they have attached to SSO.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @return all configured Identity Sources with their associated domains,
    *         not empty
    */
   @Privilege(Role.RegularUser)
   IdentitySources get() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Provides information about the Active Directory IDS authentication
    * account.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @return the configured ADIDS authentication account information (except
    *         account password), could be null if ADIDS is not configured.
    */
   @Privilege(Role.RegularUser)
   AuthenticationAccountInfo getActiveDirectoryAuthnAccountInfo()
         throws NotAuthenticatedException, NoPermissionException;

   /**
    * Get the system domain name. The system domain is always present and its
    * identity store is embedded into the SSO server itself. The system domain
    * is special in that its groups can have principals from any other domain.
    *
    * @return system domain name, not empty
    */
   @NoPrivilege
   String getSystemDomainName();

   /**
    * Update the details of an LDAP Identity Source.
    *
    * @param name
    *           The name of the LDAP Identity Source to update. Required.
    *           The lookup is case-insensitive.
    *
    * @param details
    *           The new properties of the LDAP Identity Source. Required.
    *
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            LDAP Identity Source.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws DirectoryServiceConnectionException
    *            when the probe connection test fails
    * @throws InvalidProviderException
    *            when the provider configuration specified is invalid, such as due to
    *            invalid users baseDN or invalid groups baseDN
    */
   @Privilege(Role.Administrator)
   void updateLdap(String name, LdapIdentitySourceDetails details)
      throws DomainNotFoundException, DirectoryServiceConnectionException, NotAuthenticatedException,
      NoPermissionException, InvalidProviderException;

   /**
    * Update an existing identity source of type 'Active Directory'
    * including one of the following cases:
    * <ul>
    * <li>switch between using machine account or specified STS account</li>
    * <li>update STS account credentials</li>
    * </ul>
    * Precondition: Identity source of 'Active Directory' type is already added
    * to the configuration, which also means machine is joined to the same domain.
    * @param domainName
    *           Fully qualified Active Directory Domain Name, cannot be null or empty
    * @param authnCredentials
    *           Credentials including Service Principal Name and Password to
    *           authenticate to the Active Directory domain, or null for using
    *           machine account
    * @param schemaMapping
    *           customized schema mapping. can be null or empty
    * @throws DomainNotFoundException
    *            Indicates that the name of identity source does not match the existing
    *            identity source of 'Active Directory'.
    * @throws DomainManagerException
    *            when the DomainManager API on underlying layer returns error.
    * @throws InvalidPrincipalException
    *            when the attempt to probe connection to AD server fails
    * @throws DuplicateDomainNameException
    *            Indicates that there is a name clash either among domain names
    *            or aliases or with the name of other Identity Source.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void updateActiveDirectory(String domainName,
           AuthenticationCredentials authnCredentials,
           ExternalDomainSchemaDetails schemaMapping)
           throws DomainNotFoundException, DomainManagerException, NotAuthenticatedException, NoPermissionException,
           InvalidPrincipalException, DuplicateDomainNameException;

   /**
    * Updates the way the SSO Server authenticates against an LDAP server.
    *
    * @param name
    *           The name of the LDAP Identity Source to update. Required.
    *           The lookup is case-insensitive.
    *
    * @param authnType
    *           The desired authentication type. Required.
    *
    * @param authnCredentials
    *           A user name + password tuple if the authnType is
    *           {@link LdapIdentitySource.AuthenticationType#password} or
    *           {@code null} for any other authentication type.
    *
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            LDAP Identity Source.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
     * @throws DirectoryServiceConnectionException
    *            when the probe connection test fails
    */
   @Privilege(Role.Administrator)
   void updateLdapAuthnType(String name, KnownAuthenticationType authnType,
      AuthenticationCredentials authnCredentials)
      throws DomainNotFoundException, NotAuthenticatedException,
      NoPermissionException, DirectoryServiceConnectionException;

   /**
    * Remove the Identity Source associated with the given name. The System
    * Identity Source cannot be deleted.
    *
    * @param name
    *           The name of the Identity Source to unregister; the lookup is
    *           case-insensitive.
    *
    * @throws DomainNotFoundException
    *            Indicates that the specified name is not associated with any
    *            Identity Source.
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @throws InvalidArgument
    *            when the System Identity Source is being passed as an argument
    */
   @Privilege(Role.Administrator)
   void delete(String name) throws DomainNotFoundException,
      NotAuthenticatedException, NoPermissionException;

   /***********************************
   *
   * Methods to get/set default domains
   *
   ************************************/

   /**
    * Specify default domains. Users can authenticate against default domain
    * even when a user name is provided without domain name. <br>
    * E.g. if {@code mike@vmware.com} is trying to authenticate providing
    * {@code mike} as a principal name (and correct credentials for
    * {@code mike@vmware.com}), then if:
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
    * Domains from Native AD Identity Source cannot be set as default domains.
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
    * @see #setDefaultDomains(String[])
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

   /*********************************************************
   *
   * Methods to deal with SSL trust for Ldap Identity Sources
   *
   **********************************************************/

   /**
    * Returns the manager of the SSL trust store. SSL trust is relevant only for
    * LDAP Identity Sources.
    * <p>
    * When establishing secure (SSL/TLS) connections with the LDAP server, the
    * SSO server will try to verify the remote certificate with any CA
    * certificate from the SSL trust store. Each certificate in the trust store
    * should be root (self-signed) CA certificate.
    * <p>
    * Note that while technically it is possible with the returned manager to
    * add non-self-signed or leaf certificates to the SSL trust store, doing so
    * has no effect.
    * <p>
    * Also be aware that the certificates in the SSL trust store are only
    * relevant for establishing secure connections with LDAP servers. They are
    * <b>never used</b> for validating tokens or for solution authentication.
    *
    * @return A CertificateManager object wired for managing the SSL trust
    *         store, not null
    *
    */
   @NoPrivilege
   CertificateManagement getSslCertificateManagement();

   /**
    * Probe the connectivity to the specified LDAP service and try to
    * authenticate using the provided credentials.
    *
    * @param serviceUri
    *           The URI of the target service. Required. The scheme must be
    *           {@code ldap} or {@code ldaps}. If the scheme is {@code ldaps}
    *           (LDAP Secure), the remote server's SSL certificate must be
    *           verifiable using the SSO Server's SSL trust store (or the
    *           connection will fail).
    *
    * @param authenticationType
    *           The authentication method to use. Required. If
    *           {@code KnownAuthenticationType#anonymous}, no authentication
    *           will be attempted.
    *
    * @param authenticationCredentials
    *           If {@code authenticationType} is
    *           {@code AuthenticationType#password}, the credentials (username +
    *           password) to use for authentication. Otherwise, it need not be
    *           provided.
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
    *
    * @see #getSslCertificateManager()
    */
   @Privilege(Role.Administrator)
   void probeConnectivity(URI serviceUri,
      KnownAuthenticationType authenticationType,
      AuthenticationCredentials authnCredentials)
      throws DirectoryServiceConnectionException, NotAuthenticatedException,
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
