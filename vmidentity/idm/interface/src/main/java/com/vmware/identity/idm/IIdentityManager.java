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

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.performanceSupport.IIdmAuthStat;
import com.vmware.identity.performanceSupport.IIdmAuthStatus;

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/8/11
 * Time: 5:46 PM
 * To change this template use File | Settings | File Templates.
 */
public interface IIdentityManager
{
    /*
     *  Tenant
     */
    public void addTenant(Tenant tenant, String adminAccountName, char[] adminPwd, IIdmServiceContext serviceContext) throws  IDMException;

    public void deleteTenant(String name, IIdmServiceContext serviceContext) throws  IDMException;

    public Tenant getTenant(String name, IIdmServiceContext serviceContext)throws  IDMException;

    public String getDefaultTenant(IIdmServiceContext serviceContext) throws  IDMException;

    public String getSystemTenant(IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<String> getAllTenants(IIdmServiceContext serviceContext) throws  IDMException;

    public void setDefaultTenant(String name, IIdmServiceContext serviceContext) throws  IDMException;

    public void setTenant(Tenant tenant, IIdmServiceContext serviceContext)throws  IDMException;

    public String getTenantSignatureAlgorithm(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setTenantSignatureAlgorithm(String tenantName, String signatureAlgorithm, IIdmServiceContext serviceContext) throws  IDMException;

    public List<Certificate> getTenantCertificate(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<List<Certificate>> getTenantCertificates(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setTenantTrustedCertificateChain(String tenantName, Collection<Certificate> tenantCertificates, IIdmServiceContext serviceContext) throws  IDMException;

    public void setTenantCredentials(String tenantName, Collection<Certificate> tenantCertificate, PrivateKey tenantPrivateKey, IIdmServiceContext serviceContext) throws  IDMException;

    public PrivateKey getTenantPrivateKey(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public long getClockTolerance(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setClockTolerance(String tenantName, long milliseconds, IIdmServiceContext serviceContext) throws  IDMException;

    public int getDelegationCount(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setDelegationCount(String tenantName,int delegationCount, IIdmServiceContext serviceContext) throws  IDMException;

    public int getRenewCount(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setRenewCount(String tenantName, int renewCount, IIdmServiceContext serviceContext) throws  IDMException;

    public long getMaximumBearerTokenLifetime(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setMaximumBearerTokenLifetime(String tenantName, long maxLifetime, IIdmServiceContext serviceContext) throws  IDMException;

    public long getMaximumHoKTokenLifetime(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setMaximumHoKTokenLifetime(String tenantName, long maxLifetime, IIdmServiceContext serviceContext) throws  IDMException;

    public long getMaximumBearerRefreshTokenLifetime(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setMaximumBearerRefreshTokenLifetime(String tenantName, long maxLifetime, IIdmServiceContext serviceContext) throws  IDMException;

    public long getMaximumHoKRefreshTokenLifetime(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setMaximumHoKRefreshTokenLifetime(String tenantName, long maxLifetime, IIdmServiceContext serviceContext) throws  IDMException;

    public void setEntityID(String tenantName, String entityID, IIdmServiceContext serviceContext) throws  IDMException;

    public String getEntityID(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public String getOIDCEntityID(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Retrieves the alias associated with the entityId of local identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @return alias, null if alias is not set
     * @throws Exception
     */
    public String getLocalIDPAlias(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException;

    /**
     * Sets the alias associated with the entityId of local identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @param alias alias to be set, can be null or empty.
     * @throws Exception
     */
    public void setLocalIDPAlias(String tenantName, String alias, IIdmServiceContext serviceContext)
            throws  IDMException;

    /**
     * Retrieves the alias associated with the entityId of external identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @param entityId entity id of provider, cannot be null or emtpty.
     * @return alias, null if alias is not set
     * @throws Exception
     */
    public String getExternalIDPAlias(String tenantName, String entityId, IIdmServiceContext serviceContext)
            throws  IDMException;

    /**
     * Sets the alias associated with the entityId of external identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @param entityId entity id of provider, cannot be null or empty.
     * @param alias alias to be set, can be null or empty.
     * @throws Exception
     */
    public void setExternalIDPAlias(String tenantName, String entityId, String alias, IIdmServiceContext serviceContext)
            throws  IDMException;

    /**
     * Retrieves idp selection flag for tenant.
     *
     * @param tenantName name of tenant
     * @return true if idp selection is enabled
     * @throws Exception
     */
    public boolean isTenantIDPSelectionEnabled(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Sets idp selection flag for tenant.
     *
     * @param tenantName name of tenant.
     * @param enableIDPSelection true if enabling idp selection.
     * @throws Exception
     */
    public void setTenantIDPSelectionEnabled(String tenantName, boolean enableIDPSelection, IIdmServiceContext serviceContext) throws  IDMException;

    public PasswordExpiration getPasswordExpirationConfiguration(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void updatePasswordExpirationConfiguration(String tenantName, PasswordExpiration config, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * addCertificate
     * Two types of certificates:
     * (1) LDAP: ssl certificate used to establish ldap session;
     * (2) STS: trusted certificate root used to validate tokens
     * @throws
     */
    public void addCertificate(String tenantName, Certificate idmCert, CertificateType certificateType, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * getAllCertificates
     * Returns all Certificates of type 'certificateType'
     * @throws
     */
    public Collection<Certificate> getAllCertificates(String tenantName, CertificateType certificateType, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * getTrustedCertificates
     * Returns all STS root Certificates
     * @throws
     */
    public Collection<Certificate> getTrustedCertificates(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * getStsIssuersCertificates
     * Returns all STS leaf Certificates
     * @throws
     */
    public Collection<Certificate> getStsIssuersCertificates(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;
    /**
     * deleteCertificate
     * Delete a particular type of certificate based on its finger print
     * @throws not allow deletion attempting to delete STS trusted root certificate that is part of current active signer identity
     */
    public void deleteCertificate(String tenantName, String fingerprint, CertificateType certificateType, IIdmServiceContext serviceContext) throws  IDMException;

    /*
     * SSO Health Statistics
     */

    public SsoHealthStatsData getSsoStatistics(String tenant, IIdmServiceContext serviceContext) throws  IDMException;

    public void incrementGeneratedTokens(String tenant, IIdmServiceContext serviceContext) throws  IDMException;

    public void incrementRenewedTokens(String tenant, IIdmServiceContext serviceContext) throws  IDMException;

    /*
     * RelyingParty
     */
    public void addRelyingParty(String tenantName, RelyingParty rp, IIdmServiceContext serviceContext) throws  IDMException;

    public void deleteRelyingParty(String tenantName, String rpName, IIdmServiceContext serviceContext) throws  IDMException;

    public RelyingParty getRelyingParty(String tenantName, String rpName, IIdmServiceContext serviceContext) throws  IDMException;

    public RelyingParty getRelyingPartyByUrl(String tenantName, String url, IIdmServiceContext serviceContext) throws  IDMException;

    public void setRelyingParty(String tenantName, RelyingParty rp, IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<RelyingParty> getRelyingParties(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /*
     *  OIDC Client
     */
    public void addOIDCClient(
            String tenantName,
            OIDCClient oidcClient,
            IIdmServiceContext serviceContext) throws  IDMException;

    public void deleteOIDCClient(
            String tenantName,
            String clientID,
            IIdmServiceContext serviceContext) throws  IDMException;

    public OIDCClient getOIDCClient(
            String tenantName,
            String clientID,
            IIdmServiceContext serviceContext) throws  IDMException;

    public void setOIDCClient(
            String tenantName,
            OIDCClient oidcClient,
            IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<OIDCClient> getOIDCClients(
            String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException;

    /*
     *  ResourceServer
     */
    public void addResourceServer(
            String tenantName,
            ResourceServer resourceServer,
            IIdmServiceContext serviceContext) throws  IDMException;

    public void deleteResourceServer(
            String tenantName,
            String resourceServerName,
            IIdmServiceContext serviceContext) throws  IDMException;

    public ResourceServer getResourceServer(
            String tenantName,
            String resourceServerName,
            IIdmServiceContext serviceContext) throws  IDMException;

    public void setResourceServer(
            String tenantName,
            ResourceServer resourceServer,
            IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<ResourceServer> getResourceServers(
            String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException;

    /*
     *  IdentityProvider
     */
    public void addProvider(String tenantName, IIdentityStoreData idpData, IIdmServiceContext serviceContext) throws  IDMException;

    public void deleteProvider(String tenantName, String providerName, IIdmServiceContext serviceContext) throws  IDMException;

    public IIdentityStoreData getProvider(String tenantName, String ProviderName, IIdmServiceContext serviceContext) throws  IDMException;

    public IIdentityStoreData getProviderWithInternalInfo(String tenantName, String providerName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setProvider(String tenantName, IIdentityStoreData idpData, IIdmServiceContext serviceContext ) throws  IDMException;

    public void setNativeADProvider(String tenantName, IIdentityStoreData idpData, IIdmServiceContext serviceContext ) throws  IDMException;

    public Collection<IIdentityStoreData> getProviders(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<IIdentityStoreData> getProviders(String tenantName, EnumSet<DomainType> domainTypes, IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<SecurityDomain> getSecurityDomains(String tenantName, String providerName, IIdmServiceContext serviceContext) throws  IDMException;

    public void probeProviderConnectivity(String tenantName, String providerUri,AuthenticationType authType, String userName, String pwd, Collection<X509Certificate> certificates, IIdmServiceContext serviceContext) throws  IDMException;

    public void probeProviderConnectivity(String tenantName, IIdentityStoreData idsData, IIdmServiceContext serviceContext) throws  IDMException;

    public void probeProviderConnectivityWithCertValidation(String tenantName, String providerUri, AuthenticationType authType, String userName, String pwd, Collection<X509Certificate> certificates, IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<String> getDefaultProviders(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setDefaultProviders(String tenantName, Collection<String> defaultProviders, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Authentication
     *
     * @throws IDMLoginException when invalid credentials are given (like wrong
     *            principal, password, domain)
     */
    public PrincipalId authenticate(String tenantName, String principal, String password, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * @throws IDMLoginException when invalid credentials are given
     */
    public GSSResult authenticate(String tenantName, String contextId, byte[] gssTicket, IIdmServiceContext serviceContext) throws  IDMException;

    /** authenticate with user certificate.
     *
     * @param tenantName
     * @param tlsCertChain
     * @param hint
     * @param serviceContext
     * @return PrincipalId at successful authentication
     * @throws RemoteException
     * @throws IDMException
     */
    public PrincipalId authenticate(String tenantName, X509Certificate[] tlsCertChain, String hint, IIdmServiceContext serviceContext) throws IDMException;

    /**
     * authenticate with secure ID
     *
     * @param tenantName
     * @param rsaSessionID
     *            could be null which refers to a fresh login request.
     * @param principal
     * @param passcode
     * @return
     * @throws IDMSecureIDNewPinException
     * @throws IDMLoginException
     */
    public RSAAMResult authenticateRsaSecurId(String tenantName,
            String sessionID, String principal, String passcode,
            IIdmServiceContext serviceContext)
            throws  IDMException;

    public boolean IsActive(String tenantName,PrincipalId principal, IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<Attribute> getAttributeDefinitions(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public Collection<AttributeValuePair> getAttributeValues(String tenantName, PrincipalId principal, Collection<Attribute> attributes, IIdmServiceContext serviceContext) throws  IDMException;

    public byte[] getUserHashedPassword(String tenantName, PrincipalId principal, IIdmServiceContext serviceContext) throws  IDMException;

    public PrincipalId addSolutionUser(String tenantName, String userName, SolutionDetail detail, IIdmServiceContext serviceContext) throws  IDMException;

    public SolutionUser findSolutionUser(String tenantName, String userName, IIdmServiceContext serviceContext) throws  IDMException;

    public SolutionUser findSolutionUserByCertDn(String tenantName, String subjectDN, IIdmServiceContext serviceContext) throws  IDMException;

    public PersonUser findPersonUser(String tenantName, PrincipalId id, IIdmServiceContext serviceContext) throws  IDMException;

    public PersonUser findPersonUserByObjectId(String tenantName, String userObjectId, IIdmServiceContext serviceContext) throws  IDMException;

    public Group findGroupByObjectId(String tenantName, String groupObjectId, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<PersonUser> findPersonUsers(String tenantName, SearchCriteria criteria, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<PersonUser> findPersonUsersByName(String tenantName, SearchCriteria criteria, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<SolutionUser> findSolutionUsers(String tenantName, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<Group> findGroups(String tenantName, SearchCriteria criteria, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<Group> findGroupsByName(String tenantName, SearchCriteria criteria, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<PersonUser> findPersonUsersInGroup(String tenantName, PrincipalId groupId, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<PersonUser> findPersonUsersByNameInGroup(String tenantName, PrincipalId groupId, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<SolutionUser> findSolutionUsersInGroup(String tenantName, String groupName, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<Group> findGroupsInGroup(String tenantName, PrincipalId groupId, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<Group> findGroupsByNameInGroup(String tenantName, PrincipalId groupId, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<Group> findDirectParentGroups(String tenantName, PrincipalId principalId, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Checks whether principal is member of system group. Principals include end
     * users and groups in any back-end identity source (incl. solution users).
     *
     * @param tenantName name of the tenant to search a group for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param principalId
     *           principal identifier, required
     * @param groupName
     *           system group name, required
     * @return
     *         if that group does not exist or the principal is not its member
     * @throws NoSuchTenantException specified tenant could not be found
     * @throws NoSuchIdpException domain/alias cannot be recognized.
     * @throws InvalidPrincipalException
     *            when there is no such principal
     * @throws RemoteException
     * @throws IDMException
     */
    public boolean isMemberOfSystemGroup(String tenantName, PrincipalId principalId, String groupName, IIdmServiceContext serviceContext)
        throws  NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException,  IDMException;

    public Set<Group> findNestedParentGroups(String tenantName, PrincipalId userId, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<PersonUser> findLockedUsers(String tenantName, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<PersonUser> findDisabledPersonUsers(String tenantName, String searchString, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public PrincipalId findActiveUserInSystemDomain(String tenantName, String attributeName, String attributeValue, IIdmServiceContext serviceContext) throws  IDMException;

    public Set<SolutionUser> findDisabledSolutionUsers(String tenantName, String searchString, IIdmServiceContext serviceContext) throws  IDMException;

    public SearchResult find(String tenantName, SearchCriteria criteria, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public SearchResult findByName(String tenantName, SearchCriteria criteria, int limit, IIdmServiceContext serviceContext) throws  IDMException;

    public PrincipalId addUser(String tenantName, String userName, PersonDetail detail, char[] password, IIdmServiceContext serviceContext) throws Exception;

    PrincipalId addUser(String tenantName, String userName, PersonDetail detail, byte[] hashedPassword, String hashingAlgorithm, IIdmServiceContext serviceContext) throws Exception;

    /**
     * Adds a JIT user to the tenant's system domain.
     *
     * @param tenantName Name of tenant. required, non-null, non-empty
     * @param userName Name of JIT user. required, non-null, non-empty
     * @param detail Detailed information about the user. required, non-null, non-empty
     * @param extIdpEntityId Entity Id of the external IDP where the JIT user is from. required, non-null, non-empty
     * @param extUserId JIT user's unique Id in internal identity store. required, non-null, non-empty
     * @param serviceContext required, non-null, non-empty
     * @return non-null principal id of the JIT user after it has been created.
     * @throws IDMException
     */
    public PrincipalId addJitUser(String tenantName, String userName, PersonDetail detail, String extIdpEntityId, String extUserId, IIdmServiceContext serviceContext) throws  IDMException;

    public PrincipalId addGroup(String tenantName, String groupName, GroupDetail groupDetail, IIdmServiceContext serviceContext) throws  IDMException;

    public boolean addUserToGroup(String tenantName, PrincipalId userId, String groupName, IIdmServiceContext serviceContext) throws  IDMException;

    public boolean removeFromLocalGroup(String tenantName, PrincipalId principalId, String groupName, IIdmServiceContext serviceContext) throws  IDMException;

    public boolean addGroupToGroup(String tenantName, PrincipalId groupId, String groupName, IIdmServiceContext serviceContext) throws  IDMException;

    public void deletePrincipal(String tenantName, String principalName, IIdmServiceContext serviceContext) throws  IDMException;

    public boolean enableUserAccount(String tenantName, PrincipalId userId, IIdmServiceContext serviceContext) throws  IDMException;

    public boolean unlockUserAccount(String tenantName, PrincipalId userId, IIdmServiceContext serviceContext) throws  IDMException;

    public boolean disableUserAccount(String tenantName, PrincipalId userId, IIdmServiceContext serviceContext) throws  IDMException;

    public PrincipalId updatePersonUserDetail(String tenantName, String userName, PersonDetail detail, IIdmServiceContext serviceContext) throws  IDMException;

    public PrincipalId updateGroupDetail(String tenantName, String groupName, GroupDetail detail, IIdmServiceContext serviceContext) throws  IDMException;

    public PrincipalId updateSolutionUserDetail(String tenantName, String userName, SolutionDetail detail, IIdmServiceContext serviceContext) throws  IDMException;

    public void setUserPassword(String tenantName, String userName, char[] newPassword, IIdmServiceContext serviceContext) throws  IDMException;

    public void changeUserPassword(String tenantName, String userName, char[] currentPassword, char[] newPassword, IIdmServiceContext serviceContext) throws  IDMException;

    public void updateSystemDomainStorePassword(String tenantName, char[] newPassword, IIdmServiceContext serviceContext) throws  IDMException;

    public PasswordPolicy getPasswordPolicy(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setPasswordPolicy(String tenantName, PasswordPolicy policy, IIdmServiceContext serviceContext) throws Exception;

    public LockoutPolicy getLockoutPolicy(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setLockoutPolicy(String tenantName, LockoutPolicy policy, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Search for a group by principalId.
     * @param tenantName name of the tenant to search a group for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param group principalId
     *        Required, non-null, non-empty.
     * @return Group.
     * @throws NoSuchTenantException specified tenant could not be found
     * @throws NoSuchIdpException domain/alias cannot be recognized.
     * @throws InvalidPrincipalException specified group cannot be found
     * @throws IDMException In case of a failure.
     * @throws RemoteException
     */
    public Group findGroup(String tenantName, PrincipalId groupId, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Search for a group by string based on account name.
     * @param tenantName name of the tenant to search a group for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param group name in one of the formats: <name>@[domain||alias]; [domain||alias]\\<name>;
     *        if default domain is configured, domain part could be omitted which means searching
     *        in the default domain only
     *        Required, non-null, non-empty.
     * @return Group.
     * @throws NoSuchTenantException specified tenant could not be found
     * @throws NoSuchIdpException domain/alias cannot be recognized.
     * @throws InvalidPrincipalException specified group cannot be found
     * @throws IDMException In case of a failure.
     * @throws RemoteException
     */
    public Group findGroup( String tenantName, String group, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, IDMException;

    /**
     * Search for a user by string based on account name.
     * @param tenantName name of the tenant to search a user for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param user user name in one of the formats: <name>@[domain||alias]; [domain||alias]\\<name>;
     *        if default domain is configured, domain part could be omitted which means searching
     *        in the default domain only
     *        Required, non-null, non-empty.
     * @return Principal. This could be a SolutionUser or a PersonUser.
     * @throws NoSuchTenantException specified tenant could not be found
     * @throws NoSuchIdpException domain/alias cannot be recognized.
     * @throws InvalidPrincipalException specified user cannot be found
     * @throws IDMException In case of a failure.
     * @throws RemoteException
     */
    public Principal findUser( String tenantName, String user, IIdmServiceContext serviceContext )
            throws  NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, IDMException;

    /****************************************************
     * 3rd Party (external) IDP support
     ****************************************************/
    /**
     * Create the external IDP configuration for the tenant. If the
     * configuration already exists, it will be updated accordingly.
     * Verify if the group exists in lotus when setting the mappings
     * between external token claims and lotus groups.
     *
     * @param tenantName
     *            tenant name to which the external IDP configuration belongs.
     *            Cannot be null or empty.
     * @param idpConfig
     *            configuration meta data object. Cannot be null.
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public void setExternalIdpForTenant(String tenantName, IDPConfig idpConfig, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException, IDMException;

    /**
     * Remove the externalIDP configuration for the tenant by its entityId.
     *
     * @param tenantName
     *            tenant name to which the external IDP configuration belongs.
     *            Cannot not be null or empty
     * @param configEntityId
     *            Cannot be null or empty
     * @return
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             tenant not found
     * @throws NoSuchExternalIdpConfigException
     *             external IDP configuration per the specified entityId cannot
     *             be found
     */
    public void removeExternalIdpForTenant(String tenantName,
            String configEntityId, IIdmServiceContext serviceContext) throws NoSuchTenantException, NoSuchExternalIdpConfigException, IDMException;

    /**
     * Remove the externalIDP configuration for the tenant by its entityId.
     *
     * @param tenantName
     *            tenant name to which the external IDP configuration belongs.
     *            Cannot not be null or empty
     * @param configEntityId
     *            Cannot be null or empty
     * @param removeJitUsers
     *            true if remove all Jit users created for the external IDP
     * @return
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             tenant not found
     * @throws NoSuchExternalIdpConfigException
     *             external IDP configuration per the specified entityId cannot
     *             be found
     */
    public void removeExternalIdpForTenant(String tenantName,
            String configEntityId, boolean removeJitUsers, IIdmServiceContext serviceContext) throws NoSuchTenantException, NoSuchExternalIdpConfigException, IDMException;

    /**
     * Get all external IDP configurations for the specified tenant
     *
     * @param tenantName
     *            Cannot be null or empty
     * @return Collection of IDPConfiguration objects of the tenant, empty if not found
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public Collection<IDPConfig> getAllExternalIdpsForTenant(String tenantName, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException, IDMException;

    /**
     * Retrieve external IDP configuration per tenant name and entityId.
     *
     * @param tenantName
     *            Cannot be null or empty
     * @param configEntityId
     *            Cannot be null or empty
     * @return IdpConfiguration with the matching entityId, null if not found
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public IDPConfig getExternalIdpForTenant(String tenantName,
            String configEntityId, IIdmServiceContext serviceContext) throws NoSuchTenantException, IDMException;

    /**
     * Retrieve collection of external IDP configurations with matching URL
     *
     * @param tenantName
     *            Cannot be null or empty.
     * @param urlStr
     *            string used to match SSO / SLO locations. Cannot be null or
     *            empty.
     * @return Collection of idpConfiguration with the matching URL location,
     *         empty if not found. Note the result matches either the SSO
     *         service end-points or SLO service end-points
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public Collection<IDPConfig> getExternalIdpForTenantByUrl(String tenantName, String urlStr, IIdmServiceContext serviceContext) throws NoSuchTenantException, IDMException;

    /**
     * Register the third party userId in the tenant's system provider as FSP
     * joining default group
     *
     * @param tenantName
     *            Name of tenant, required non-null, non-empty
     * @param userId
     *            represents a globally unique external IDP user which is to be
     *            assigned default group membership as part of registration.
     *            required non-null.
     * @return true if the external IDP user was successfully registered.
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             when tenant does not exist
     * @throws InvalidArgumentException
     *             invalid input
     * @throws MemberAlreadyExistException
     *             external IDP user already registered
     */
    boolean registerThirdPartyIDPUser(String tenantName, PrincipalId userId, IIdmServiceContext serviceContext)
            throws  IDMException, NoSuchTenantException;

    /**
     * Remove the FSP object registered for external IDP user in the tenant's
     * system provider.
     *
     * @param tenantName
     *            cannot be null or empty
     * @param userId
     *            globally unique external IDP user Id TODO: does FSP object
     *            need to be removed explicitly after being removed from default
     *            group, when it is not member of any groups?
     * @return true if succeeded.
     * @throws RemoteException
     * @throws IDMException
     * @throws InvalidArgumentException
     *             invalid input
     * @throws NoSuchTenantException
     *             when tenant does not exist
     * @throws InvalidPrincipalException
     *             when the external IDP user was not registered
     */
    boolean removeThirdPartyIDPUser(String tenantName, PrincipalId userId, IIdmServiceContext serviceContext)
            throws  IDMException, NoSuchTenantException;

    /**
     * find the external IDP user registered
     * @param tenantName cannot be null or empty
     * @param userId cannot be null.
     * @return PersonUser object for the registered user. {@code null} if not found
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *             when tenant does not exist
     */
   PersonUser findRegisteredExternalIDPUser(String tenantName,
         PrincipalId userId, IIdmServiceContext serviceContext) throws  IDMException,
         NoSuchTenantException;

    /**
     * Retrieve the group name used for registration of externalIDP user.
     * @return the well-known registration group name for externalIDP
     * @throws RemoteException
     * @throws IDMException
     */
    String getExternalIDPRegistrationGroupName(IIdmServiceContext serviceContext) throws  IDMException;


    /**
     * Register an UPN suffix for the specified tenant domain
     *
     * @param tenantName
     *           cannot be null or empty
     * @param domainName
     *           cannot be null or empty
     * @param upnSuffix
     *           UPN suffix to register, such as "@gmail.com". cannot be null or
     *           empty.
     * @return true if value does not exist and added successfully; false if
     *         value is not added due to value already exist or unsuccessful
     *         operation handled gracefully.
     *         <p> Value matching is not case-sensitive.
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *            when tenant does not exist.
     * @throws NoSuchIdpException
     *            domain/alias cannot be recognized.
     */
     boolean registerUpnSuffix(String tenantName, String domainName, String upnSuffix, IIdmServiceContext serviceContext)
          throws  IDMException, NoSuchTenantException, NoSuchIdpException;

    /**
     * Remove an UPN suffix previously registered for the specified tenant domain
     *
     * @param tenantName
     *           cannot be null or empty
     * @param domainName
     *           cannot be null or empty
     * @param upnSuffix
     *           UPN suffix to be removed, such as "@gmail.com". cannot be null
     *           or empty.
     * @return true if found and deleted successfully. false if cannot be found.
     * @throws RemoteException
     * @throws IDMException
     * @throws NoSuchTenantException
     *            when tenant does not exist.
     * @throws NoSuchIdpException
     *            domain/alias cannot be recognized.
     */
     boolean unregisterUpnSuffix(String tenantName, String domainName,
          String upnSuffix, IIdmServiceContext serviceContext) throws  IDMException,
          NoSuchTenantException, NoSuchIdpException;

   /**
    * Query UPN suffixes list for the specific domain
    *
    * @param tenantName
    *           cannot be null or empty
    * @param domainName
    *           cannot be null or empty
    * @return set of strings for the suffixes or {@code null} if none registered
    * @throws RemoteException
    * @throws IDMException
    * @throws NoSuchTenantException
    *            when tenant does not exist.
    * @throws NoSuchIdpException
    *            domain/alias cannot be recognized.
    */
     Set<String> getUpnSuffixes(String tenantName, String domainName, IIdmServiceContext serviceContext)
           throws  IDMException,
           NoSuchTenantException, NoSuchIdpException;

     String getBrandName(String tenantName, IIdmServiceContext serviceContext) throws   IDMException;

    void setBrandName(String tenantName, String brandName, IIdmServiceContext serviceContext)
            throws  IDMException;

    /**
     * Get logon banner title for the tenant.
     *
     * @param tenantName
     *           cannot be null or empty.
     * @return logon banner title string. null if not set.
     * @throws RemoteException
     * @throws IDMException
     */
    String getLogonBannerTitle(String tenantName, IIdmServiceContext serviceContext) throws   IDMException;

    /**
     * Set logon banner title for the tenant.
     *
     * @param tenantName
     *           cannot be null or empty.
     * @param logonBannerTitle
     *           can be null or empty.
     * @throws RemoteException
     * @throws IDMException
     */
    void setLogonBannerTitle(String tenantName, String logonBannerTitle, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Get logon banner content for the tenant.
     *
     * @param tenantName
     *           cannot be null or empty.
     * @return logon banner content. null if not set.
     * @throws RemoteException
     * @throws IDMException
     */
    String getLogonBannerContent(String tenantName, IIdmServiceContext serviceContext) throws   IDMException;

    /**
     * Set logon banner content.
     *
     * @param tenantName
     *           cannot be null or empty.
     * @param logonBannerContent
     *           can be null or empty.
     * @throws RemoteException
     * @throws IDMException
     */
    void setLogonBannerContent(String tenantName, String logonBannerContent, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Check if logon banner checkbox is enabled.
     *
     * @param tenantName
     *           cannot be null or empty.
     * @throws RemoteException
     * @throws IDMException
     */

    /**
     * Set authentication policy for identity provider
     *
     * @param tenantName Name of tenant
     * @param providerName Name of identity provider.
     * @param policy Authentication policy to be set on identity provider
     * @param serviceContext
     */
    public void setAuthnPolicyForProvider(String tenantName, String providerName, AuthnPolicy policy, IIdmServiceContext serviceContext) throws Exception;

    boolean getLogonBannerCheckboxFlag(String tenantName, IIdmServiceContext serviceContext) throws   IDMException;

    /**
     * Set logon banner checkbox.
     *
     * @param tenantName
     *           cannot be null or empty.
     * @param enableLogonBannerCheckbox
     *           true if enabled
     * @param serviceContext
     * @throws RemoteException
     * @throws IDMException
     */
    void setLogonBannerCheckboxFlag(String tenantName, boolean enableLogonBannerCheckbox, IIdmServiceContext serviceContext) throws   IDMException;

    /*
     * System information
     */

    /**
     * Retrieve the Active Directory Domain Join Status
     *
     * @return Information regarding Active Directory Join Status
     */
    ActiveDirectoryJoinInfo
    getActiveDirectoryJoinStatus(IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Query the cluster identifier for this instance
     *
     * @return Cluster Identifier
     */
    String getClusterId(IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Query the deployment identifier for this instance
     *
     * @return Deployment identifier
     */
    String getDeploymentId(IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Retrieve the Domain Trust Info
     *
     * @return List of Trusted Domains
     */
    Collection<DomainTrustsInfo> getDomainTrustInfo(IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * @return Returns host name of the sso box. (IP as a fall-back)
     */
    public String getSsoMachineHostName(IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Retrieve a collection of all joined systems including Domain Controllers and, optionally,
     * Computer accounts.
     *
     * @param tenantName
     *           cannot be null or empty
     * @param getDCOnly
     *           indicates whether the caller is interested in Domain Controller accounts only.
     *           If <tt>false</tt>, retrieves Computer accounts as well.
     *
     * @return list of VmHostData containing information about the joined systems.
     *
     * @throws RemoteException
     * @throws IDMException
     */
    public Collection<VmHostData> getComputers(String tenantName, boolean getDCOnly, IIdmServiceContext serviceContext) throws  IDMException;

   /**
    * Join the SSO server machine to AD domain.
    * @param user
    *           cannot be null or empty
    * @param password
    *           cannot be null
    * @param domain
    *           cannot be null or empty
    * @param orgUnit
    *           can be null or empty
    * @param serviceContext
    * @throws RemoteException
    * @throws IDMException
    * @throws IdmADDomainException
    */
    void joinActiveDirectory(String user, String password, String domain, String orgUnit, IIdmServiceContext serviceContext) throws  IDMException, IdmADDomainException;

   /**
    * Operation for the SSO server to leave the AD domain
    * @param user
    *           cannot be null or empty
    * @param password
    *           cannot be null
    * @throws RemoteException
    * @throws IDMException
    * @throws ADIDSAlreadyExistException
    * @throws IdmADDomainException
    */
    void leaveActiveDirectory(String user, String password, IIdmServiceContext serviceContext) throws  IDMException, ADIDSAlreadyExistException, IdmADDomainException;

    /**
     * @param tenantName
     * @param serviceContext
     * @return non null current AuthnPolicy object.
     * @throws RemoteException
     * @throws IDMException
     */
    public AuthnPolicy getAuthNPolicy(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException;

    /**
     * Operation for retrieving the IDM authentication statistics cache
     *
     * @param tenantName cannot be null or empty
     * @param serviceContext required, non-null
     * @throws RemoteException
     * @throws IDMException
     */
    public List<IIdmAuthStat> getIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Operation for retrieving the status of the IDM authentication statistics cache
     *
     * @param tenantName cannot be null or empty
     * @param serviceContext required, non-null
     * @return the status of the authentication statistics cache
     * @throws RemoteException
     * @throws IDMException
     */
    public IIdmAuthStatus getIdmAuthStatus(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Operation for clearing the IDM authentication statistics cache
     *
     * @param tenantName cannot be null or empty
     * @param serviceContext required, non-null
     * @throws RemoteException
     * @throws IDMException
     */
    public void clearIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Operation for setting the IDM authentication statistics cache size
     *
     * @param tenantName cannot be null or empty
     * @param size size of the authentication stat window
     * @param serviceContext required, non-null
     * @throws RemoteException
     * @throws IDMException
     */
    public void setIdmAuthStatsSize(String tenantName, int size, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Operation for enabling the IDM authentication statistics cache
     *
     * @param tenantName cannot be null or empty
     * @param serviceContext required, non-null
     * @throws RemoteException
     * @throws IDMException
     */
    public void enableIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Operation for disabling the IDM authentication statistics cache
     *
     * @param tenantName cannot be null or empty
     * @param serviceContext required, non-null
     * @throws RemoteException
     * @throws IDMException
     */
    public void disableIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    public void setAuthNPolicy(String tenantName, AuthnPolicy policy, IIdmServiceContext serviceContext)
            throws  IDMException;


        /**
     * Retrieves SPN in the form of HTTP/computername.acme.com in case is
     * AD-joined, or the HTTP/computername if not joined.
     *
     * Use case: WEBSSO server pass this information to logon page script for
     *
     * @throws RemoteException
     * @throws Exception
     */
    String getServerSPN() throws  Exception;

    /**
     * Add rsa secureID agent
     * @param tenantName  non null string name.
     * @param rsaAgentConfig non null RSAAgentConfig
     * @param serviceContext
     * @throws Exception
     */
    public void setRSAConfig(String tenantName, RSAAgentConfig rsaAgentConfig, IIdmServiceContext serviceContext) throws Exception;

    /**
     * Return RSA secureID agent configuration that is associates to the site.
     * @param tenantName
     * @param siteID
     * @param serviceContext
     * @return RSAAgentConfig or null
     * @throws IDMException
     * @throws RemoteException
     */
    public RSAAgentConfig getRSAConfig(String tenantName, IIdmServiceContext serviceContext) throws  IDMException;

    /**
     * Adding a RSA AM instance-specific configuration to tenant
     * @param tenantName
     * @param rsaAgentConfig
     * @param serviceContext
     * @throws Exception
     */
    public void addRSAInstanceInfo(String tenantName, RSAAMInstanceInfo rsaInstInfo, IIdmServiceContext serviceContext) throws Exception;
    /**
     * @param tenantName
     * @param siteID
     * @throws Exception
     */
    public void deleteRSAInstanceInfo(String tenantName, String siteID, IIdmServiceContext serviceContext) throws Exception;
}
