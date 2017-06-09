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

package com.vmware.identity.idm.server;

import java.io.IOException;
import java.net.SocketException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.UnknownHostException;
import java.rmi.RemoteException;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.SecureRandom;
import java.security.cert.Certificate;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

import javax.security.auth.login.LoginException;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.SystemUtils;
import org.apache.commons.lang.Validate;

import sun.security.krb5.KrbException;

import com.vmware.af.VmAfClientNativeException;
import com.vmware.af.interop.VmAfAccessDeniedException;
import com.vmware.af.interop.VmAfAlreadyJoinedException;
import com.vmware.af.interop.VmAfBadPacketException;
import com.vmware.af.interop.VmAfInvalidComputerNameException;
import com.vmware.af.interop.VmAfInvalidParameterException;
import com.vmware.af.interop.VmAfLdapNoSuchObjectException;
import com.vmware.af.interop.VmAfNoSuchDomainException;
import com.vmware.af.interop.VmAfNoSuchLogonSessionException;
import com.vmware.af.interop.VmAfNotSupportedException;
import com.vmware.af.interop.VmAfUnknownServerException;
import com.vmware.af.interop.VmAfWrongPasswordException;
import com.vmware.certificate.Request;
import com.vmware.certificate.VMCAClient;
import com.vmware.identity.auth.passcode.spi.AuthenticationResult;
import com.vmware.identity.auth.passcode.spi.AuthenticationSecret;
import com.vmware.identity.auth.passcode.spi.AuthenticationSession;
import com.vmware.identity.auth.passcode.spi.AuthenticationSessionFactory;
import com.vmware.identity.auth.passcode.spi.SessionFactoryProvider;
import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.diagnostics.VmEvent;
import com.vmware.identity.idm.ADIDSAlreadyExistException;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.CertificateRevocationCheckException;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.CommonUtil;
import com.vmware.identity.idm.ContainerAlreadyExistsException;
import com.vmware.identity.idm.DomainManagerException;
import com.vmware.identity.idm.DomainTrustsInfo;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.DuplicateProviderException;
import com.vmware.identity.idm.DuplicateTenantException;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.HostNotJoinedRequiredDomainException;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityManager;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IIdmServiceContext;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.IdmADDomainException;
import com.vmware.identity.idm.IdmADDomainJoinStatusException;
import com.vmware.identity.idm.IdmCertificateRevokedException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPasswordPolicyException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.InvalidProviderException;
import com.vmware.identity.idm.LocalISRegistrationException;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.MemberAlreadyExistException;
import com.vmware.identity.idm.NoSuchExternalIdpConfigException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.PasswordExpiredException;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PasswordPolicyViolationException;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.Principal;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.STSSpnValidator;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.SsoHealthStatsData;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.VmHostData;
import com.vmware.identity.idm.server.clientcert.IdmClientCertificateValidator;
import com.vmware.identity.idm.server.clientcert.IdmCrlCache;
import com.vmware.identity.idm.server.clientcert.TenantCrlCache;
import com.vmware.identity.idm.server.config.ConfigStoreFactory;
import com.vmware.identity.idm.server.config.IConfigStore;
import com.vmware.identity.idm.server.config.IConfigStoreFactory;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.config.directory.DirectoryConfigStore;
import com.vmware.identity.idm.server.config.directory.TenantAttributes;
import com.vmware.identity.idm.server.config.directory.TokenPolicy;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.IdmAuthStatCache;
import com.vmware.identity.idm.server.performance.IdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.NoopIdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.PerformanceMonitor;
import com.vmware.identity.idm.server.performance.PerformanceMonitorFactory;
import com.vmware.identity.idm.server.provider.BaseLdapProvider;
import com.vmware.identity.idm.server.provider.GSSAuthProvider;
import com.vmware.identity.idm.server.provider.GSSAuthResult;
import com.vmware.identity.idm.server.provider.IGssAuthIdentityProvider;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.IProviderFactory;
import com.vmware.identity.idm.server.provider.ISystemDomainIdentityProvider;
import com.vmware.identity.idm.server.provider.LdapConnectionPool;
import com.vmware.identity.idm.server.provider.NoSuchUserException;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.ProviderFactory;
import com.vmware.identity.idm.server.provider.activedirectory.ActiveDirectoryProvider;
import com.vmware.identity.idm.server.provider.activedirectory.ServerKrbUtils;
import com.vmware.identity.idm.server.provider.localos.LocalOsIdentityProvider;
import com.vmware.identity.idm.server.vmaf.VmafClientUtil;
import com.vmware.identity.interop.IdmUtils;
import com.vmware.identity.interop.accountmanager.AccountLockedOutException;
import com.vmware.identity.interop.accountmanager.AccountPasswordExpiredException;
import com.vmware.identity.interop.directory.Directory;
import com.vmware.identity.interop.domainmanager.DomainControllerInfo;
import com.vmware.identity.interop.domainmanager.DomainTrustInfo;
import com.vmware.identity.interop.idm.IIdmClientLibrary;
import com.vmware.identity.interop.idm.IdmClientLibraryFactory;
import com.vmware.identity.interop.ldap.AlreadyExistsLdapException;
import com.vmware.identity.interop.ldap.ConstraintViolationLdapException;
import com.vmware.identity.interop.ldap.DirectoryStoreProtocol;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ServerDownLdapException;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;
import com.vmware.identity.performanceSupport.IIdmAuthStat;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.IIdmAuthStat.EventLevel;
import com.vmware.identity.performanceSupport.IIdmAuthStatus;
import com.vmware.identity.performanceSupport.IdmAuthStatus;
import com.vmware.identity.performanceSupport.PerfBucketKey;
import com.vmware.identity.performanceSupport.PerfDataSinkFactory;
import com.vmware.identity.performanceSupport.PerfMeasurementPoint;
/**
 * User: snambakam
 * Date: 12/23/11
 * Time: 1:56 PM
 */
public class IdentityManager implements IIdentityManager {
    class IdmCachePeriodicChecker extends Thread
    {

        @Override
        public void run()
        {
            try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", "bba76607-42b4-4a15-a3c2-7542f427d12c", "IdmCachePeriodicChecker") )
            {
            while(true)
            {
                try
                {
                    long startTime = System.nanoTime();

                    IdentityManager.this.refreshTenantCache();

                    if (PerfDataSinkFactory.getPerfDataSinkInstance() != null)
                    {
                        PerfDataSinkFactory.getPerfDataSinkInstance().addMeasurement(
                                new PerfBucketKey(
                                        PerfMeasurementPoint.IDMPeriodicRefreshTenantCertificates),
                                        TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime));
                    }
                }
                catch (Throwable t)
                {
                    logger.error(String.format("IdmCachePeriodicChecker refreshTenantCredential failed : %s",
                            t.getMessage()), t);
                    t.printStackTrace();
                }

                try
                {
                    // refresh tenant certificates every 15 seconds
                    Thread.sleep(15000);
                }
                catch (InterruptedException e)
                {
                    logger.error("IdmCachePeriodicChecker Thread is interrupted!");
                    e.printStackTrace();
                }
            }
        }
    }

    }


    /**
     * CRL checker thread refreshing at interval of 1 hour (3600000 milliseconds)
     *
     */
    class IdmCrlCachePeriodicChecker extends Thread
    {
        private final Integer CheckInterval = 3600000;
        @Override
        public void run()
        {
            try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", "IdmCrlCachePeriodicChecker", "IdmCrlCachePeriodicChecker") )
            {
                while(_crlCacheChecker == Thread.currentThread())
                {
                    try
                    {
                        IdentityManager.this.refreshTenantCrlCache();
                    }
                    catch (Throwable t)
                    {
                        logger.error(String.format("IdmCrlCachePeriodicChecker refreshTenantCrl failed : %s",
                                t.getMessage()), t);
                        t.printStackTrace();
                    }

                    try
                    {
                        Thread.sleep(CheckInterval);
                    }
                    catch (InterruptedException e)
                    {
                        logger.error("IdmCrlCachePeriodicChecker Thread is interrupted!");
                        e.printStackTrace();
                    }
                }
            }
        }
    }


    private class ProvidersInfo {
        Collection<IIdentityProvider> _providers;
        Collection<IIdentityStoreData> _idsStores; // internal provider data store information
        IIdentityProvider _adProvider;
        Collection<String> _defaultProviders;

        private ProvidersInfo(Collection<IIdentityProvider> providers,
                              Collection<IIdentityStoreData> idsStores,
                              IIdentityProvider adProvider,
                              Collection<String> defaultProviders)
        {
            this._providers = providers;
            this._idsStores = idsStores;
            this._adProvider = adProvider;
            this._defaultProviders = defaultProviders;
        }
    }

    private static final long serialVersionUID = -7719567998332472234L;
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdentityManager.class);
    private static final Object identityManagerLock = new Object();

    // CONSTANTS
    private static final String DEFAULT_HTTPS_PORT = "443";
    private static final String HOSTNAME_MACRO = "<HOSTNAME>"; // to be susbstituted with getHostIPAddress()
    private static final String PORT_MACRO = "<PORT>"; // to be substituted with StsTomcat ipaddress
    private static final String LOCAL_OS_STATIC_ALIAS = "localos";
    private static final String PROVIDER_TYPE_RSA_SECURID = "RsaSecureID";
    private static final String PASSCODE_PROVIDER_TYPE = "RSA";

    public static final String THIRD_PARTY_IDP_USER_DEFAULT_GROUP_NAME = "Users";
    public static final String INTERNAL_ATTR_GROUP_OBJECTIDS = "userMemberOfGroupIds"; // This will include both user's own sid as well as the sids of groups user is a member of
    public static final char[] INVALID_CHARS_FOR_USER_DETAIL = "^<>&%`".toCharArray();
    public static final char[] VALID_ACCOUNT_NAME_SEPARATORS = {ServerUtils.UPN_SEPARATOR, ServerUtils.NETBIOS_SEPARATOR };     // we support either 'user@domain', 'domain\\user', or default domain formats
    public static final char[] INVALID_CHARS_FOR_USER_ID = (String.valueOf(INVALID_CHARS_FOR_USER_DETAIL) + ServerUtils.UPN_SEPARATOR + ServerUtils.NETBIOS_SEPARATOR).toCharArray();
    public static final String WELLKNOWN_SOLUTIONUSERS_GROUP_NAME = "SolutionUsers";
    public static final String WELLKNOWN_SOLUTIONUSERS_GROUP_DESCRIPTION = "Well-known solution users' group, which contains all solution users as members.";
    public static final String WELLKNOWN_CONFIGURATIONUSERS_GROUP_NAME = "SystemConfiguration.Administrators";
    public static final String WELLKNOWN_CONFIGURATIONUSERS_GROUP_DESCRIPTION = "Well-known configuration users' group which contains all configuration users as members.";
    public static final String WELLKNOWN_ACT_AS_USERS_GROUP_NAME = "ActAsUsers";
    public static final String WELLKNOWN_ACT_AS_USERS_GROUP_DESCRIPTION = "Well-known act-as users' group which contains all solution users that are allowed to act on behalf of person users.";
    public static final String WELLKNOWN_TRUSTED_USERS_GROUP_NAME = "TrustedUsers";
    public static final String WELLKNOWN_TRUSTED_USERS_GROUP_DESCRIPTION = "Well-known trusted users' group which contains all users with privileges just below administrator.";
    public static final String WELLKNOWN_EXTERNALIDP_USERS_GROUP_NAME =  "ExternalIDPUsers";
    public static final String WELLKNOWN_EXTERNALIDP_USERS_GROUP_DESCRIPTION = "Well-known external IDP users' group, which registers external IDP users as guests.";
    public static final String WELLKNOWN_CONTAINER_SERVICE_PRINCIPALS = "ServicePrincipals";

    /**
     * A singleton IDM instance intend to be used across all other webapps.
     */
    private static IdentityManager idmInstance = null;

    private IProviderFactory providerFactory;
    private IConfigStoreFactory configStoreFactory;
    private AuthSessionFactoryCache _rsaSessionFactoryCache;
    private final static RsaAuthSessionCache _rsaSessionCache = new RsaAuthSessionCache();;
    private IConfigStore _configStore;
    private Collection<Attribute> _defaultAttributes;
    private TenantCache _tenantCache;
    private TenantCrlCache _tenantCrlCache;
    private volatile Thread _crlCacheChecker = null;
    private static final PersonDetail EXTERNAL_USER_SAMPLE_PERSON_DETAIL = new PersonDetail.Builder().build();
    private static SsoHealthStatistics ssoHealthStatistics = new SsoHealthStatistics();

    /**
     * Performs IDM bootstrap actions and instantiates tenant cache. Along with, Starts two caching threads for refreshing tenant information CRL info
     */
    protected IdentityManager() throws IDMException {
        try {
            this.configStoreFactory = new ConfigStoreFactory();
            this.providerFactory = new ProviderFactory();
            this._tenantCache = new TenantCache();
            this._tenantCrlCache = new TenantCrlCache();
            this._configStore = this.configStoreFactory.getConfigStore();

            IdmServerConfig settings = IdmServerConfig.getInstance();
            this._defaultAttributes = settings.getDefaultAttributesList();
            if (this._defaultAttributes == null) {
                this._defaultAttributes = Collections.emptyList();
            }

            this._rsaSessionFactoryCache = new AuthSessionFactoryCache();
            ssoHealthStatistics.setUpTimeIDMService();

            // Auxiliary IDM bootstrap actions
            IdmDomainState.getInstance();
            String systemTenant = registerServiceProviderAsTenant(); // Set up system tenant
            initializeTenantCache(); // prepare tenant cache
            ensureValidTenant(systemTenant);
            ensureWellKnownGroupExists(systemTenant, WELLKNOWN_CONFIGURATIONUSERS_GROUP_NAME,
                    WELLKNOWN_CONFIGURATIONUSERS_GROUP_DESCRIPTION);

            // Start the Tenant Cache thread
            Thread idmCacheThread = new IdmCachePeriodicChecker();
            idmCacheThread.start();

            // Start the CRL Cache thread
            ManageCrlCacheChecker();

            logger.info("Setting thread local system properties...");
            System.setProperties(new ThreadLocalProperties(System.getProperties()));

            logger.info("Identity Manager initialized successfully");

        } catch(Exception ex) {
            logger.error("IDM bootstrap failed", ex);
            throw ServerUtils.getRemoteException(ex);
        }
    }

    /**
     * Check if any tenant enabled smartcard authentication
     * @return true if at least one tenant enables smart card authentication.
     * @throws Exception if unable to check tenants.
     */
    private boolean smartCardAuthnEnabled() throws Exception {
        Collection<String> allTenantNames = this.getAllTenants();
        assert(allTenantNames != null && allTenantNames.size() > 0);

        boolean smartCardAuthnEnabled = false;
        for (String tenantName : allTenantNames)
        {
            try
            {
                AuthnPolicy policy = getTenantInfo(tenantName).getAuthnPolicy();

                if (policy.IsTLSClientCertAuthnEnabled()) {
                    smartCardAuthnEnabled = true;
                    break;
                }
            }
            catch(Exception ex)
            {
                logger.error(
                        String.format(
                                "Failed to retrieve Authentication policy for tenant %s. ",
                                tenantName));
                throw ex;
            }
        }
        return smartCardAuthnEnabled;
    }

    private void ensureValidTenant(String tenantName) throws Exception {
        // create 'SolutionUsers' well-known group
        // (each solution user is automatically member of such group upon creation)
        ensureWellKnownGroupExists(tenantName, WELLKNOWN_SOLUTIONUSERS_GROUP_NAME,
                WELLKNOWN_SOLUTIONUSERS_GROUP_DESCRIPTION);

        // create registration group for external IDP users
        ensureWellKnownGroupExists(tenantName, WELLKNOWN_EXTERNALIDP_USERS_GROUP_NAME,
                WELLKNOWN_EXTERNALIDP_USERS_GROUP_DESCRIPTION);

        ensureWellKnownGroupExists(tenantName, WELLKNOWN_ACT_AS_USERS_GROUP_NAME,
                WELLKNOWN_ACT_AS_USERS_GROUP_DESCRIPTION);

        ensureWellKnownGroupExists(tenantName, WELLKNOWN_TRUSTED_USERS_GROUP_NAME,
                WELLKNOWN_TRUSTED_USERS_GROUP_DESCRIPTION);

        // Make sure we create ServicePrincipal containers to place solution users
        ensureContainerExists(tenantName, WELLKNOWN_CONTAINER_SERVICE_PRINCIPALS);
    }

    private
    void addTenant(Tenant tenant, String adminAccountName, char[] adminPwd) throws Exception
    {
        try
        {
            ValidateUtil.validateNotNull(tenant, "Tenant");
            ValidateUtil.validateNotEmpty(adminAccountName, "adminAccountName");
            ValidateUtil.validateNotEmpty(adminPwd, "adminPwd");

            registerTenant(tenant, adminAccountName, adminPwd);

            Directory.createInstance(
                  tenant.getName(),
                  adminAccountName,
                  String.valueOf(adminPwd));

            //Add Tenant admin to System Admins group
            Collection<IIdentityStoreData> stores =
                getProviders(tenant.getName(), EnumSet.of(DomainType.SYSTEM_DOMAIN), true/*internal*/);
            if (stores.size() != 1)
            {
                throw new IllegalStateException("Failed to find tenant's system domain");
            }
            stores.iterator().next().getExtendedIdentityStoreData();

            ensureValidTenant(tenant.getName());

            // load tenant information to cache when adding tenant
            TenantInformation tenantInfo = loadTenant(tenant.getName());
            assert(tenantInfo != null);

            logger.info(String.format(
                    "Tenant [%s] added successfully",
                    tenant.getName()));
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to add tenant [%s]",
                    tenant.getName()));

            if (ex instanceof AlreadyExistsLdapException)
            {
                throw new DuplicateTenantException(ex.getMessage());
            }
            else
            {
                throw ex;
            }
        }
    }

    private
    void deleteTenant(String name) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(name, "Tenant name");

            TenantInformation tenantInfo = findTenant(name);
            ServerUtils.validateNotNullTenant(tenantInfo, name);

            // verify that we are not deleting the system tenant
            if (name.equalsIgnoreCase(getSystemTenant()))
            {
                String errMsg = String.format(
                        "Failed to delete tenant [%s], this is system tenant",
                        name);
                logger.error(errMsg);

                throw new IllegalArgumentException(errMsg);
            }

            // Need to use the system tenant administrator to delete tenant instances...
            Collection<IIdentityStoreData> stores = _configStore.getProviders(
                    getSystemTenant(),
                    EnumSet.of(DomainType.SYSTEM_DOMAIN),
                    true);

            if (stores.isEmpty())
            {
                logger.error("Unable to retrieve the system domain for system tenant");
                throw new IllegalStateException("Unable to retrieve system domain for system tenant");
            }

            IIdentityStoreData systemDomainData = stores.iterator().next();
            String username = systemDomainData.getExtendedIdentityStoreData().getUserName();
            String password = systemDomainData.getExtendedIdentityStoreData().getPassword();

            unregisterTenant(name);

            Directory.deleteInstance(name, username, password);

            _tenantCache.deleteTenant(name);
            ssoHealthStatistics.removeTenantStats(name);
            LdapConnectionPool.getInstance().cleanPool(name);

            PerformanceMonitor.getInstance().deleteCache(name);

            logger.info(String.format(
                    "Tenant [%s] deleted successfully",
                    name));
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to delete tenant [%s]",
                    name));
            throw ex;
        }
    }

    private
    Tenant getTenant(String name) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(name, "Tenant name");

            TenantInformation tenantInfo = findTenant(name);
            ServerUtils.validateNotNullTenant(tenantInfo, name);

            return tenantInfo.getTenant();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get tenant [%s]",
                    name));

            throw ex;
        }
    }

    private
    void setTenant(Tenant tenant) throws Exception
    {
        try
        {
            ValidateUtil.validateNotNull(tenant, "Tenant");

            TenantInformation tenantInfo = findTenant(tenant.getName());
            ServerUtils.validateNotNullTenant(tenantInfo, tenant.getName());

            _configStore.setTenant(tenant);

            _tenantCache.deleteTenant(tenant.getName());

            logger.info(String.format("Tenant [%s] set successfully",
                    tenant.getName()));
        }
        catch(Exception ex)
        {
            logger.error(String.format("Failed to set tenant [%s]",
                    tenant.getName()));

            throw ex;
        }
    }

    private
    List<Certificate>
    getTenantCertificate(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getTenantCertificate();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get certificate for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    Collection<List<Certificate>>
    getTenantCertificates(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getTenantCertificates();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get certificate for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    /**
     * Retrieves the currently configured signature algorithm for the tenant.
     *
     * @param tenantName
     * @return algorithm name.
     * @throws IDMException
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name is
     *                                  null or empty
     */
    private
    String getTenantSignatureAlgorithm(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getSignatureAlgorithm();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get signature algorithm for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

     /**
     * Sets the signature algorithm used for signing tokens from STS for that tenant.
     *
     * @param tenantName
     * @return
     * @throws IDMException
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name or
     *                                 signatureAlgorithm is null or empty
     */
    private
    void
    setTenantSignatureAlgorithm(
            String tenantName,
            String signatureAlgorithm
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            logger.debug(String.format(
                    "Signature algorithm [%s] will be set for tenant [%s]",
                    signatureAlgorithm,
                    tenantName));

            _configStore.setSignatureAlgorithm(tenantName, signatureAlgorithm);

            logger.info(String.format(
                    "Signature algorithm successfully set for tenant [%s]",
                    tenantName));

            _tenantCache.deleteTenant(tenantName);
            logger.debug(String.format(
                    "tenant cache removed for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set signature algorithm for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }
    /**
     * Sets the optional brand name for the tenant. This string is used on login page.
     * If the attribute is not set or set to null/empty string, it will use vCenter login page with vCenter logo
     * If a non-empty value is set, the brand name string is displayed on the login page.
     *
     * @param tenantName
     * @param brandname
     * @return
     * @throws IDMException
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name or
     *                                 signatureAlgorithm is null or empty
     */
    private
    void
    setBrandName(
            String tenantName,
            String brandName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            logger.debug(String.format(
                    "Band name [%s] will be set for tenant [%s]",
                    brandName,
                    tenantName));

            _configStore.setBrandName(tenantName, brandName);

            logger.info(String.format(
                    "Brand name successfully set for tenant [%s]",
                    tenantName));

            _tenantCache.deleteTenant(tenantName);
            logger.debug(String.format(
                    "tenant cache removed for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set brand name for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }
    /**
     * Retrieves configured brand name for the tenant.
     *
     * @param tenantName
     * @return brand name.
     * @throws IDMException
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name is
     *                                  null or empty
     */
    private
    String
    getBrandName(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getBrandName();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get brand name for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private void setLogonBannerContent(String tenantName, String logonBannerContent)
            throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            logger.debug(String.format("Logon banner will be set for tenant [%s]", tenantName));

            _configStore.setLogonBannerContent(tenantName, logonBannerContent);
            logger.info(String.format("Logon banner successfully set for tenant [%s]", tenantName));

            _tenantCache.deleteTenant(tenantName);
            logger.debug(String.format("tenant cache removed for tenant [%s]", tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format("Failed to set logon banner for tenant [%s]", tenantName));
            throw ex;
        }
    }

    /**
     * Set authentication policies per identity source. This also takes care of synchronizing authentication types for tenant
     */
    @Override
    public void setAuthnPolicyForProvider(String tenantName, String providerName, AuthnPolicy policy, IIdmServiceContext serviceContext) throws Exception {

        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setAuthnPolicyForProvider")) {

            // Validate if provider exists
            ValidateUtil.validateNotEmpty(providerName, "Identity provider name");
            IIdentityStoreData provider = getProviderWithInternalInfo(tenantName, providerName);
            if (provider == null) {
                String errMessage = String.format("Failed to find provider:'%s' on tenant :'%s'", providerName, tenantName);
                logger.error(errMessage);
                throw new NoSuchIdpException(errMessage);
            }

            if (policy.IsRsaSecureIDAuthnEnabled()) {
                checkPasscodeAuthProviderConfigured();
            }

            // Set authentication policy for identity source
            _configStore.setAuthnTypesForProvider(tenantName, providerName,
                    policy.IsPasswordAuthEnabled(),
                    policy.IsWindowsAuthEnabled(),
                    policy.IsTLSClientCertAuthnEnabled(),
                    policy.IsRsaSecureIDAuthnEnabled());

            // Delete tenant data from cache such that it will load fresh data on next request
            _tenantCache.deleteTenant(tenantName);

            // Synchronize tenant authentication policy
            synchronizeTenantAuthenticationPolicy(tenantName);

        } catch (Exception ex) {
            logger.error("Failed to set authentication policy for provider : " + providerName, ex);
            throw ServerUtils.getRemoteException(ex);
        }
    }

    /**
     *
     * This ensure the authentication policy set on tenant is always a super set of authentication policies of each identity source
     * attached to the same tenant.
     *
     * @param tenantName name of tenant
     */
    private void synchronizeTenantAuthenticationPolicy(String tenantName) throws Exception{

        boolean passwordAuthEnabled = false;
        boolean windowsAuthEnabled = false;
        boolean certAuthEnabled = false;
        boolean rsaSecureIDAuthnEnabled = false;

        Collection<IIdentityStoreData> providers = getProviders(tenantName);
        Set<Integer> tenantPermissibleAuthnTypes = new HashSet<Integer>();
        for(IIdentityStoreData provider : providers) {
            int[] authnTypes = provider.getExtendedIdentityStoreData().getAuthnTypes();
            for(int authnType : authnTypes) {
                tenantPermissibleAuthnTypes.add(Integer.valueOf(authnType));
            }
        }

        // Sync password authentication
        if(tenantPermissibleAuthnTypes.contains(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_PASSWORD)) {
            passwordAuthEnabled = true;
        }

        // Sync windows based authentication
        if(tenantPermissibleAuthnTypes.contains(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_WINDOWS)) {
            windowsAuthEnabled = true;
        }

        // Sync CAC based authentication
        if(tenantPermissibleAuthnTypes.contains(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE)) {
            certAuthEnabled = true;
        }

        // Sync RSA securID authentication
        if(tenantPermissibleAuthnTypes.contains(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID)) {
            rsaSecureIDAuthnEnabled = true;
        }
        _configStore.setAuthnTypes(tenantName, passwordAuthEnabled, windowsAuthEnabled, certAuthEnabled, rsaSecureIDAuthnEnabled);
    }

    private String getLogonBannerContent(String tenantName)
            throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getLogonBannerContent();
        }
        catch(Exception ex)
        {
            logger.error(String.format("Failed to get logon banner for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private String getLogonBannerTitle(String tenantName)
            throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getLogonBannerTitle();
        }
        catch(Exception ex)
        {
            logger.error(String.format("Failed to get logon banner title for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private boolean getLogonBannerCheckboxFlag(String tenantName)
            throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getLogonBannerCheckboxFlag();
        }
        catch(Exception ex)
        {
            logger.error(String.format("Failed to get logon banner title for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private void setLogonBannerTitle(String tenantName, String logonBannerTitle)
            throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            logger.debug(String.format("Logon banner title [%s] will be set for tenant [%s]", logonBannerTitle, tenantName));

            _configStore.setLogonBannerTitle(tenantName, logonBannerTitle);
            logger.info(String.format("Logon banner successfully set for tenant [%s]", tenantName));

            _tenantCache.deleteTenant(tenantName);
            logger.debug(String.format("tenant cache removed for tenant [%s]", tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format("Failed to set logon banner for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private void setLogonBannerCheckbox(String tenantName, boolean enableLogonBannerCheckbox) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            logger.debug(String.format("Logon banner checkbox [%s] will be set for tenant [%s]", enableLogonBannerCheckbox, tenantName));

            _configStore.setLogonBannerCheckboxFlag(tenantName, enableLogonBannerCheckbox);
            logger.info(String.format("Logon banner successfully set for tenant [%s]", tenantName));

            _tenantCache.deleteTenant(tenantName);
            logger.debug(String.format("tenant cache removed for tenant [%s]", tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format("Failed to set logon banner for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private
    List<Certificate>
    validateCertificateChain(
            String tenantName, /* optional */
            Collection<Certificate> tenantCertificates,
            PrivateKey tenantPrivateKey
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantCertificates, "Certificate chain");

            if (tenantCertificates.size() <= 1)
            {
                throw new IllegalArgumentException("Invalid certificate chain");
            }

            // MEMO: IDM also can simply throw exception
            // telling client certChain is invalid with duplicate certificates instead

            // Make sure certificate chain does not have duplicate certs
            // OR ldap operation will fail due to dup values for one attribute
            List<Certificate> certChainWithNoDup = new ArrayList<Certificate>();
            for (Certificate cert : tenantCertificates)
            {
                if (!certChainWithNoDup.contains(cert))
                {
                    certChainWithNoDup.add(cert);
                }
            }

            // Validate the certChain with no duplicate certificates while preserving the original order

            // (1) length validation
            if (certChainWithNoDup.size() <= 1)
            {
                throw new IllegalArgumentException("Invalid certificate chain");
            }

            if (tenantPrivateKey != null)
            {
                // (2) The certChain must be ordered and
                // contain a Certificate at index 0 corresponding to the private key
                Certificate cert = certChainWithNoDup.get(0);
                new PrivateKeyWithCertificateSignatureValidator().validate(tenantPrivateKey, cert);
            }

            return certChainWithNoDup;
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to validate tenant credentials for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    setTenantCredentials(
            String                  tenantName,
            Collection<Certificate> tenantCertificates,
            PrivateKey              tenantPrivateKey
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            // check whether tenant exists, if not, throw NoSucnTenantException.
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            // Validate 'tenantCertificates' and "tenantPrivateKey"
            ValidateUtil.validateNotNull(tenantPrivateKey, "Tenant privatekey");

            List<Certificate> certChainWithNoDup = validateCertificateChain(tenantName,
                    tenantCertificates,
                    tenantPrivateKey);

            _configStore.setTenantCredentials(
                        tenantName,
                        certChainWithNoDup,
                        tenantPrivateKey);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Credentials successfully set for tenant [%s]",
                    tenantName));
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to set credentials for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    setTenantTrustedCertificateChain(
            String                  tenantName,
            Collection<Certificate> tenantCertificates
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            // check whether tenant exists, if not, throw NoSucnTenantException.
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            List<Certificate> certChainWithNoDup = validateCertificateChain(tenantName,
                    tenantCertificates,
                    null);

            _configStore.setTenantTrustedCertificateChain(
                        tenantName,
                        certChainWithNoDup);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Trusted certificate chain successfully set for tenant [%s]",
                    tenantName));
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to set credentials for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    PrivateKey
    getTenantPrivateKey(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getPrivateKey();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get private key for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    private boolean isTenantIDPSelectionEnabled(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.isIDPSelectionEnabled();
        } catch(Exception ex)
        {
            logger.error(String.format("Failed to get IDP selection flag for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private void setTenantIDPSelectionEnabled(String tenantName, boolean enableIDPSelection) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            logger.debug(String.format("IDP selection flag [%s] will be set for tenant [%s]", enableIDPSelection, tenantName));

            _configStore.setTenantIDPSelectionEnabled(tenantName, enableIDPSelection);
            logger.info(String.format("IDP selection flag successfully set for tenant [%s]", tenantName));

            _tenantCache.deleteTenant(tenantName);
            logger.debug(String.format("tenant cache removed for tenant [%s]", tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format("Failed to set IDP selection flag for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private
    long
    getClockTolerance(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getClockTolerance();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get clock tolerance for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    private
    void
    setClockTolerance(
            String tenantName,
            long   milliseconds
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            _configStore.setClockTolerance(tenantName, milliseconds);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Clock tolerance successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set clock tolerance for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    /**
     * Retrieves the configuration setting in the tenant pertaining to the
     * maximum number of times a SAML token may be delegated.
     *
     * @param tenantName Name of tenant
     * @throws IDMException
     * @throws RemoteException
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     */
    private
    int
    getDelegationCount(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getDelegationCount();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get delegation count for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    /**
     * Sets the maximum number of times a SAML token generated by the tenant
     * may be delegated.
     *
     * @param tenantName      Name of tenant, required, non-null, non-empty.
     * @param delegationCount A positive integer, required, non-negative
     * @throws IDMException
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     */
    private
    void
    setDelegationCount(
            String tenantName,
            int    delegationCount
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            _configStore.setDelegationCount(tenantName, delegationCount);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Delegation count successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set delegation count for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    int getRenewCount(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getRenewCount();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get renew count for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    /**
     * Sets the maximum number of times a SAML token generated by the tenant may
     * be renewed.
     *
     * @param tenantName Name of tenant, required, non-null, non-empty
     * @param renewCount Maximum number of time. Positive integer
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     */
    private
    void
    setRenewCount(
            String tenantName,
            int    renewCount
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            _configStore.setRenewCount(tenantName, renewCount);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Renew count successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set renew count for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    SsoHealthStatsData
    getSsoStatistics(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            return ssoHealthStatistics.getSsoStatistics(tenantName);

        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to get Sso Health Statistics for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    private
    void
    incrementGeneratedTokens(
            String tenantName) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            ssoHealthStatistics.incrementGeneratedTokens(tenantName);

        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to increment generated tokens for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    incrementRenewedTokens(
            String tenantName) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            ssoHealthStatistics.incrementRenewedTokens(tenantName);

        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to increment renewed tokens for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }
    private
    long
    getMaximumBearerTokenLifetime(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getMaxBearerTokenLifetime();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get maximum bearer lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    setMaximumBearerTokenLifetime(
            String tenantName,
            long   maxLifetime
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            _configStore.setMaximumBearerTokenLifetime(tenantName, maxLifetime);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Maximum Bearer token lifetime successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set maximum bearer lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    long
    getMaximumHoKTokenLifetime(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getMaxHOKLifetime();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get maximum holder of key lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    setMaximumHoKTokenLifetime(
            String tenantName,
            long   maxLifetime
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            _configStore.setMaximumHoKTokenLifetime(tenantName, maxLifetime);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Maximum HoK token lifetime successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set maximum holder of key lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    long
    getMaximumBearerRefreshTokenLifetime(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getMaxBearerRefreshTokenLifetime();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get maximum bearer refresh token lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    setMaximumBearerRefreshTokenLifetime(
            String tenantName,
            long   maxLifetime
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            this._configStore.setMaximumBearerRefreshTokenLifetime(tenantName, maxLifetime);

            this._tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Maximum bearer refresh token lifetime successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set maximum bearer refresh token lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    long
    getMaximumHoKRefreshTokenLifetime(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getMaxHoKRefreshTokenLifetime();
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get maximum holder of key refresh token lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    setMaximumHoKRefreshTokenLifetime(
            String tenantName,
            long   maxLifetime
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            this._configStore.setMaximumHoKRefreshTokenLifetime(tenantName, maxLifetime);

            this._tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Maximum HoK refresh token lifetime successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to set maximum holder of key refresh token lifetime for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    PasswordExpiration
    getPasswordExpirationConfiguration(
            String tenantName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            return _configStore.getPasswordExpirationConfiguration(tenantName);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to get password expiration configuration for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    updatePasswordExpirationConfiguration(
            String             tenantName,
            PasswordExpiration config
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(
                    config,
                    "Password expiration configuration");

            _configStore.updatePasswordExpirationConfiguration(
                    tenantName,
                    config);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to update password expiration configuration for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    addCertificate(
            String      tenantName,
            Certificate idmCert,
            CertificateType certificateType
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(idmCert, "Certificate");
            ValidateUtil.validateCertType(certificateType, "Certificate type");

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                _configStore.addCertificateForSystemTenant(tenantName, idmCert, certificateType);
            }
            else
            {
                _configStore.addCertificateForNonSystemTenant(tenantName, idmCert, certificateType);
            }

            _tenantCache.deleteTenant(tenantName);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to add certificate for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    Collection<Certificate>
    getAllCertificates(
            String tenantName,
            CertificateType certificateType
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateCertType(certificateType, "Certificate type");

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                return _configStore.getAllCertificatesForSystemTenant(tenantName, certificateType);
            }
            else
            {
                return _configStore.getAllCertificatesForNonSystemTenant(tenantName, certificateType);
            }
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to get all certificates for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    Collection<Certificate>
    getTrustedCertificates(
        String tenantName
        ) throws Exception
    {
        try
    {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            return _configStore.getTrustedCertificatesForTenant(tenantName);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to get all certificates for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    private
    Collection<Certificate>
    getStsIssuersCertificates(
        String tenantName
        ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            return _configStore.getStsIssuersCertificatesForTenant(tenantName);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to get all certificates for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    private
    void
    deleteCertificate(
            String tenantName,
            String fingerprint,
            CertificateType certificateType
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(fingerprint, "Fingerprint");
            ValidateUtil.validateCertType(certificateType, "Certificate type");

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                _configStore.deleteCertificateForSystemTenant(tenantName, fingerprint.toLowerCase(), certificateType);
            }
            else
            {
                _configStore.deleteCertificateForNonSystemTenant(tenantName, fingerprint.toLowerCase(), certificateType);
            }
            _tenantCache.deleteTenant(tenantName);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to delete certificates for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    addRelyingParty(
            String       tenantName,
            RelyingParty rp
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(rp, "Relying party");

            _configStore.addRelyingParty(tenantName, rp);
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to add relying party for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    void
    deleteRelyingParty(
            String tenantName,
            String rpName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(rpName, "Replying party name");

            _configStore.deleteRelyingParty(tenantName, rpName);
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to delete relying party [%s] for tenant [%s]",
                    rpName,
                    tenantName));

            throw ex;
        }
            }

    private
    RelyingParty
    getRelyingParty(
            String tenantName,
            String rpName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(rpName, "Replying party name");

            return _configStore.getRelyingParty(tenantName, rpName);
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get relying party [%s] for tenant [%s]",
                    rpName,
                    tenantName));

            throw ex;
        }
            }

    private
    RelyingParty
    getRelyingPartyByUrl(
            String tenantName,
            String url
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(url, "Replying party URL");

            return _configStore.getRelyingPartyByUrl(tenantName, url);
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get relying party by url [%s] for tenant [%s]",
                    url,
                    tenantName));

            throw ex;
        }
            }

    private
    void
    setRelyingParty(
            String       tenantName,
            RelyingParty rp
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(rp, "Replying party");

            _configStore.setRelyingParty(tenantName, rp);

            logger.info(String.format(
                    "Relying party successfully set for tenant [%s]",
                    tenantName));
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to set relying party for tenant [%s]",
                    tenantName));

            throw ex;
        }
            }

    private
    Collection<RelyingParty>
    getRelyingParties(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            return _configStore.getRelyingParties(tenantName);
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get relying parties for tenant [%s]",
                    tenantName));

            throw ex;
         }
      }

    private void addOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(oidcClient, "oidcClient");

            _configStore.addOIDCClient(tenantName, oidcClient);
        } catch (Exception ex) {
            logger.error(String.format("Failed to add OIDC client for tenant [%s]", tenantName));

            throw ex;
        }
    }

    private void deleteOIDCClient(String tenantName, String clientID) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(clientID, "clientID");

            _configStore.deleteOIDCClient(tenantName, clientID);
        } catch (Exception ex) {
            logger.error(String.format("Failed to delete OIDC client [%s] for tenant [%s]", clientID, tenantName));

            throw ex;
        }
    }

    private OIDCClient getOIDCClient(String tenantName, String clientID) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(clientID, "clientID");

            return _configStore.getOIDCClient(tenantName, clientID);
        } catch (Exception ex) {
            logger.error(String.format("Failed to get OIDC client [%s] for tenant [%s]", clientID, tenantName));

            throw ex;
        }
    }

    private void setOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(oidcClient, "oidcClient");

            _configStore.setOIDCClient(tenantName, oidcClient);

            logger.info(String.format("OIDC client successfully set for tenant [%s]", tenantName));
        } catch (Exception ex) {
            logger.error(String.format("Failed to set OIDC client for tenant [%s]", tenantName));

            throw ex;
        }
    }

    private Collection<OIDCClient> getOIDCClients(String tenantName) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");

            return _configStore.getOIDCClients(tenantName);
        } catch (Exception ex) {
            logger.error(String.format("Failed to get OIDC clients for tenant [%s]", tenantName));

            throw ex;
        }
    }

    private void addResourceServer(String tenantName, ResourceServer resourceServer) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(resourceServer, "resourceServer");
            _configStore.addResourceServer(tenantName, resourceServer);
        } catch (Exception ex) {
            logger.error(String.format("Failed to add resource server for tenant [%s]", tenantName), ex);
            throw ex;
        }
    }

    private void deleteResourceServer(String tenantName, String resourceServerName) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotEmpty(resourceServerName, "resourceServerName");
            _configStore.deleteResourceServer(tenantName, resourceServerName);
        } catch (Exception ex) {
            logger.error(String.format("Failed to delete resource server [%s] for tenant [%s]", resourceServerName, tenantName), ex);
            throw ex;
        }
    }

    private ResourceServer getResourceServer(String tenantName, String resourceServerName) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotEmpty(resourceServerName, "resourceServerName");
            return _configStore.getResourceServer(tenantName, resourceServerName);
        } catch (Exception ex) {
            logger.error(String.format("Failed to get resource server [%s] for tenant [%s]", resourceServerName, tenantName), ex);
            throw ex;
        }
    }

    private void setResourceServer(String tenantName, ResourceServer resourceServer) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(resourceServer, "resourceServer");
            _configStore.setResourceServer(tenantName, resourceServer);
        } catch (Exception ex) {
            logger.error(String.format("Failed to set resource server for tenant [%s]", tenantName), ex);
            throw ex;
        }
    }

    private Collection<ResourceServer> getResourceServers(String tenantName) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            return _configStore.getResourceServers(tenantName);
        } catch (Exception ex) {
            logger.error(String.format("Failed to get resource servers for tenant [%s]", tenantName), ex);
            throw ex;
        }
    }

    /**
     * Go through the IDPs of the tenant and check whether IDP with specified type already exists.
     * Return name of the IDP or null if not found
     */
    private String findIdpTypeRegistered(String tenantName, IdentityStoreType type) throws Exception
    {
       Collection<IIdentityStoreData> extIDPs =
          _configStore.getProviders(tenantName, EnumSet.of(DomainType.EXTERNAL_DOMAIN), true);
       for (IIdentityStoreData idp : extIDPs)
       {
          if (idp.getExtendedIdentityStoreData().getProviderType()
                == type)
          {
             return idp.getName();
          }
       }
       return null;
    }

    /**
     * Go through the IDPs of the tenant and check whether IDP with specified type and specified name already exists.
     * Return name of the IDP or null if not found
     */
    private String findIdpTypeRegisteredWithName(String tenantName, IdentityStoreType type, String providerName) throws Exception
    {
       ValidateUtil.validateNotEmpty(providerName, "Provider name");
       Collection<IIdentityStoreData> extIDPs =
       _configStore.getProviders(tenantName, EnumSet.of(DomainType.EXTERNAL_DOMAIN), true);
       for (IIdentityStoreData idp : extIDPs)
       {
          if (idp.getExtendedIdentityStoreData().getProviderType()
                == type && idp.getName().equalsIgnoreCase(providerName))
          {
              return idp.getName();
          }
       }
       return null;
    }

    // idsData.getExtendedIdentityStoreData() is non-null
    private IIdentityStoreData checkAndNormalizeAdIdStore(IIdentityStoreData idsData)
            throws IDMException
    {
        String adDomainName = idsData.getName();
        ActiveDirectoryJoinInfo adJoinInfo = null;
        boolean useMachineAccount = idsData.getExtendedIdentityStoreData().useMachineAccount();

        try
        {
            adJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
            if (adJoinInfo == null || !adJoinInfo.getName().equalsIgnoreCase(adDomainName))
            {
               throw new HostNotJoinedRequiredDomainException(adDomainName,
                     adJoinInfo == null ?"" : adJoinInfo.getName());
            }
        }
        catch (com.vmware.identity.interop.domainmanager.HostNotJoinedException e)
        {
            throw new HostNotJoinedRequiredDomainException(adDomainName, null);
        }
        catch(com.vmware.identity.interop.domainmanager.DomainManagerNativeException e)
        {
            throw new DomainManagerException(adDomainName, e.getErrCode());
        }
        catch(com.vmware.identity.interop.domainmanager.DomainManagerException e)
        {
            throw new DomainManagerException(adDomainName);
        }

        // if use machine account, by pass spn check
        if (!useMachineAccount)
        {
            // make sure SPN name is prefixed with 'STS/'
            String spn = idsData.getExtendedIdentityStoreData().getServicePrincipalName();
            STSSpnValidator.validate(spn);
            ActiveDirectoryProvider.probeAdConnectivity(idsData);
        }

        // For native AD, we only need persist username, spn, password, bUserMachineAccount and flags
        return getADIdsToStore(idsData);
    }

    /**
     * Adds an identity provider to the tenant's configuration
     *
     * @param tenantName Name of tenant. Required, non-null, non-empty, case insensitive.
     * @param idp        Identity Provider information, required.
     * @throws Exception
     *              overall -- when finally addProvider() fails;
     * @throws InvalidArgumentException     -- when the data store has improper config;
     * @throws IDMLoginException            -- failed generic probe connectivity test;
     * @throws DuplicateProviderException   -- when we have duplicate tenant name;
     * @throws InvalidProviderException     -- when the userBaseDN or groupBaseDN is invalid for
     *                                         adding non-nativeAD IDS.
     * @throws ADIDSAlreadyExistException   -- nativeAD or ADLdap already exist when adding nativeAD IDS;
     *                                         Or, nativeAD already exist when adding ADLdap.
     * @throws LocalISRegistrationException -- when trying to add localOS provider is already exist
     * @throws RemoteException        - if unable to connect the IDM server
     */
    private synchronized
    void
    addProvider(
            String             tenantName,
            IIdentityStoreData idsData
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(idsData, "Identity store configuration");

            if (idsData.getDomainType() == DomainType.EXTERNAL_DOMAIN &&
                idsData.getExtendedIdentityStoreData() != null)
            {
                if(idsData.getExtendedIdentityStoreData().getAuthenticationType() == AuthenticationType.SRP){
                    throw new InvalidArgumentException(
                            "AuthenticationType.SRP is not allowed for EXTERNAL_DOMAIN.");
                }

                // Check whether we have Native AD configured
                String adIDP = findIdpTypeRegistered(tenantName, IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY);
                ActiveDirectoryJoinInfo machineJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
                if (adIDP != null && machineJoinInfo == null)
                {
                    throw new IllegalStateException(
                            "A native active directory is configued with SSO, however, machine is not currently joined.");
                }

                // Native AD, make sure host is joined to this AD domain
                if (idsData.getExtendedIdentityStoreData().getProviderType() == IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY &&
                    idsData.getExtendedIdentityStoreData().getAuthenticationType() == AuthenticationType.USE_KERBEROS)
                {
                    if (adIDP != null && machineJoinInfo != null)
                    {
                        logger.error("There is already one nativeAD [%s] registered", adIDP);
                        throw new ADIDSAlreadyExistException(machineJoinInfo.getName());
                    }

                    if (machineJoinInfo != null) {
                        String adLdapIDPName = findIdpTypeRegisteredWithName(tenantName, IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
                                                                             machineJoinInfo.getName());
                        if (adLdapIDPName != null)
                        {
                            logger.error(String.format("There is already one AD-Over-LDAP [%s] registered", adLdapIDPName));
                            throw new ADIDSAlreadyExistException(adLdapIDPName);
                        }
                    }

                    idsData = checkAndNormalizeAdIdStore(idsData);
                }
                // Non-native AD
                else
                {
                    ValidateUtil.validateIdsDomainNameNAlias(idsData);
                    // Do not add AD Over Ldap provider if the name is the same as the currently joined domain (if native AD is already configured)
                    if (adIDP != null && machineJoinInfo != null &&
                        idsData.getName().equalsIgnoreCase(machineJoinInfo.getName()))
                    {
                        logger.error("There is already one nativeAD [%s] registered", adIDP);
                        throw new ADIDSAlreadyExistException(machineJoinInfo.getName());
                    }

                    ValidateUtil.validateIdsUserNameAndBaseDN(idsData);

                    ValidateUtil.validateNotNull(
                            idsData.getExtendedIdentityStoreData(), "idsData.getExtendedIdentityStoreData()");
                    ValidateUtil.validateNotNull(
                            idsData.getExtendedIdentityStoreData().getConnectionStrings(),
                            "idsData.getExtendedIdentityStoreData().getConnectionStrings()");

                    if( idsData.getExtendedIdentityStoreData().getConnectionStrings().size() < 1 )
                    {
                        throw new InvalidArgumentException("There must be at least 1 connection string provided.");
                    }

                    // use simple bind
                    try
                    {
                       probeProviderConnectivity(tenantName, idsData);
                    }
                    catch(Exception ex)
                    {
                        logger.error(String.format(
                            "Failed to pass connection test for [%s]",
                            tenantName));

                        throw ex;
                    }
                }
            }

            _configStore.addProvider(tenantName, idsData);

            _tenantCache.deleteTenant(tenantName);
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to add identity provider for tenant [%s]",
                    tenantName));

            throw ex;
        }
    }

    private void setNativeADProvider(String tenantName, IIdentityStoreData idsData) throws Exception
    {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(idsData, "idsData");
            ValidateUtil.validateNotNull(idsData.getExtendedIdentityStoreData(), "extenededStoreData");
            assert idsData.getExtendedIdentityStoreData().getProviderType() == IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY;
            assert idsData.getExtendedIdentityStoreData().getAuthenticationType() == AuthenticationType.USE_KERBEROS;

            //make sure host is joined to this AD domain
            idsData = checkAndNormalizeAdIdStore(idsData);

            ValidateUtil.validateIdsDomainNameNAlias(idsData);
            ValidateUtil.validateIdsUserNameAndBaseDN(idsData);
            ValidateUtil.validateNotEmpty(idsData.getExtendedIdentityStoreData().getConnectionStrings(), "connectionStrings");

            if (!idsData.getExtendedIdentityStoreData().useMachineAccount())
            {
                ActiveDirectoryProvider.probeAdConnectivity(idsData);
            }

            _configStore.setProvider(tenantName, idsData);
            _tenantCache.deleteTenant(tenantName);
        } catch (Exception ex) {
            logger.error(String.format("Failed to set native AD provider for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private
    void
    deleteProvider(
            String tenantName,
            String providerName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(
                    providerName,
                    "Identity provider name");

            IIdentityStoreData provider = this.getProvider(tenantName, providerName);
            if ( provider != null )
            {
                if ( provider.getDomainType() == DomainType.SYSTEM_DOMAIN )
                {
                    throw new InvalidArgumentException(
                        String.format("Cannot delete a system domain provider [%s]", providerName));
                }

                _configStore.deleteProvider(tenantName, provider.getName());
                _tenantCache.deleteTenant(tenantName);
            }
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to delete identity provider [%s] for tenant [%s]",
                    providerName,
                    tenantName));

            throw ex;
        }
            }

    /**
     * Retrieves an identity provider from the tenant's configuration
     *
     * @param tenantName   Name of tenant. Required, non-null, non-empty, case insensitive.
     * @param ProviderName Name of Identity Provider. Required.
     * @return             Identity Provider information, null if not found.
     * @throws IDMException
     *  @ NoSuchTenantException
     * @throws InvalidArgumentException - one or more input argument is invalid.
     */
    private
    IIdentityStoreData
    getProvider(
            String tenantName,
            String providerName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(
                    providerName,
                    "Identity provider name");

            return _configStore.getProvider(
                        tenantName,
                        providerName,
                        false /* exclude internal info */);
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to get identity provider [%s] for tenant [%s]",
                    providerName,
                    tenantName), ex);

            throw ex;
        }
   }

    /**
     * Retrieves an identity provider from the tenant's configuration along with its internal info
     *
     * @param tenantName   Name of tenant. Required, non-null, non-empty, case insensitive.
     * @param ProviderName Name of Identity Provider. Required.
     * @return             Identity Provider information, null if not found.
     * @throws IDMException On IDM server errors
     * @throws NoSuchTenantException If tenant doesn't exist
     * @throws InvalidArgumentException - one or more input argument is invalid.
     */
    private IIdentityStoreData getProviderWithInternalInfo(String tenantName, String providerName) throws Exception {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(providerName, "Identity provider name");
            return _configStore.getProvider(tenantName, providerName, true);
        }
        catch(Exception ex)
        {
            logger.error(String.format("Failed to get identity provider [%s] for tenant [%s]", providerName, tenantName), ex);
            throw ex;
        }
   }

    private
    void
    setProvider(
            String             tenantName,
            IIdentityStoreData idsData
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(idsData, "Identity store configuration");

         IIdentityStoreData provider = this.getProviderWithInternalInfo(tenantName, idsData.getName());
         if ( provider != null && provider.getDomainType() == DomainType.SYSTEM_DOMAIN)
         {
             throw new InvalidArgumentException(
                 String.format("Cannot update a system domain provider [%s] ", idsData.getName()));
         }

         AuthenticationType authType = idsData.getExtendedIdentityStoreData().getAuthenticationType();
         if(authType == AuthenticationType.SRP){
             throw new InvalidArgumentException(
                     "AuthenticationType.SRP is not allowed for EXTERNAL_DOMAIN.");
         }
         // if we are not native ad
         if (idsData.getExtendedIdentityStoreData().getProviderType() !=
                 IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY ||
                 authType != AuthenticationType.USE_KERBEROS)
         {
             probeProviderConnectivity(tenantName, idsData);
         }
         // native AD
         else
         {
             String adIDP = findIdpTypeRegistered(tenantName, IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY);
             ActiveDirectoryJoinInfo machineJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
             if (adIDP != null && machineJoinInfo == null)
             {
                 throw new IllegalStateException(
                         "A native active directory is configued with SSO, however, machine is not currently joined.");
             }
             idsData = checkAndNormalizeAdIdStore(idsData);
         }

         _configStore.setProvider(tenantName, idsData);

         _tenantCache.deleteTenant(tenantName);

         logger.info(String.format(
               "Provider [%s] successfully set for tenant [%s]",
               idsData.getName(), tenantName));
      }
      catch (Exception ex) {
         logger.error(String.format(
               "Failed to set Ldap provider for tenant [%s]", tenantName));

            throw ex;
        }
    }

    private void checkDn(IIdentityStoreData idsData, URI serverUri) throws Exception
    {
        ILdapConnectionEx connection = null;
        try {
            IIdentityStoreDataEx idsDataEx = idsData.getExtendedIdentityStoreData();

            connection = ServerUtils.getLdapConnectionByURIs(
                Collections.singleton(serverUri),
                idsDataEx.getUserName(),
                idsDataEx.getPassword(),
                AuthenticationType.PASSWORD, false,
                new LdapCertificateValidationSettings(idsData.getExtendedIdentityStoreData().getCertificates()));

            String userBaseDn = idsDataEx.getUserBaseDn();
            String groupBaseDn = idsDataEx.getGroupBaseDn();
            if (!ServerUtils.isValidDN(connection, userBaseDn))
            {
                String msg = String.format("DN is invalid: [%s]", userBaseDn);
                logger.error(msg);
                throw new InvalidProviderException(msg, "userBaseDN", userBaseDn);
            }
            if (!ServerUtils.isValidDN(connection, groupBaseDn))
            {
                String msg = String.format("DN is invalid: [%s]", groupBaseDn);
                logger.error(msg);
                throw new InvalidProviderException(msg, "groupBaseDN", groupBaseDn);
            }
        } finally {
            if (null != connection)
            {
                connection.close();
            }
        }
    }

    /**
     * Retrieves all providers configured for a tenant
     *
     * @param tenantName Name of tenant, non-null, non-empty, required.
     * @return           Collection of identity providers, Empty collect if no provider found
     * @throws IDMException
     * @throws InvalidArgumentException
     * @ NoSuchTenantException
     */
    private
    Collection<IIdentityStoreData>
    getProviders(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            return getProviders(
                    tenantName,
                    EnumSet.of(
                            DomainType.EXTERNAL_DOMAIN,
                            DomainType.LOCAL_OS_DOMAIN,
                            DomainType.SYSTEM_DOMAIN));
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get all identity providers for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    /**
     * Retrieves all the providers matching the set of specified domain types
     * in a tenant's configuration
     *
     * @param tenantName  Name of tenant, non-null non-empty, required
     * @param domains    Set of domain types. Required.
     * @return           Collection of identity providers, Empty collect if no provider found
     * @throws Exception
     * @throws InvalidArgumentException
     *  @ NoSuchTenantException
     * @throws RemoteException        - if unable to connect the IDM server
     * @see    DomainType
     */
    private
    Collection<IIdentityStoreData>
    getProviders(
        String              tenantName,
        EnumSet<DomainType> domainTypes
        ) throws Exception
    {
        try
        {
            return this.getProviders(tenantName, domainTypes, false);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get identity providers by domain types for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    /**
     * Retrieves the security domains supported by a provider
     *
     * @param tenantName    Name of tenant, non-null non-empty, required
     * @param providerName  Name of identity provider
     * @return Collection of domains, Empty collection if no provider found
     * @throws Exception
     */
    private
    Collection<SecurityDomain>
    getSecurityDomains(
            String tenantName,
            String providerName
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(providerName, "Provider name");

            TenantInformation tenantInfo = findTenant(tenantName);

            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            // The provider name is the fully qualified domain name
            IIdentityProvider provider =
                                tenantInfo.findProviderByName(providerName);
            if (provider != null)
            {
                return Collections.unmodifiableCollection(
                        provider.getDomains());
            }
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get security domains for provider [%s] in tenant [%s]",
                            providerName,
                            tenantName));

            throw ex;
        }

        return Collections.emptySet();
    }

    /**
     * Checks the connectivity to an identity provider
     *
     * @param tenantName  Name of tenant, non-null non-empty, required
     * @param providerUri Location of identity provider. non-null non-empty, required
     * @param authType    Type of authentication. required.
     *                 currently supports AuthenticationType.PASSWORD only.
     * @param userName    Login identifier. non-null, required
     * @param pwd         Password    non-null non-empty, required
     * @param certs       Trusted certificates used for SSL connection
     * @throws InvalidPrincipalException    user name is invalid (empty)
     * @throws IDMLoginException. If one or more of the input argument is illegal.
     *                                 Or URI syntax is incorrect.
     * @throws IDMException
     * @see AuthenticationType
     */
    private
    void
    probeProviderConnectivity(
        String             tenantName,
        String             providerUri,
        AuthenticationType authType,
        String             userName,
        String             pwd,
        LdapCertificateValidationSettings certValidationSettings
        ) throws Exception
    {

        URI uri = null;
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotEmpty( providerUri, "providerUri" );

            // providerUri for now has to start with ldap||ldaps
            try {
                uri = new URI(providerUri);
            }catch (URISyntaxException e)
            {
               throw new IllegalArgumentException(e.getMessage());
            }

            if ( ( !uri.getScheme().toLowerCase().equals("ldap") )
                    && ( !uri.getScheme().toLowerCase().equals( "ldaps" ) ) )
            {
                throw new IllegalArgumentException(
                        String.format(
                                "Unsupported providerUri='%s'.",
                                providerUri ));
            }

            if( AuthenticationType.PASSWORD != authType )
            {
                throw new IllegalArgumentException(
                        String.format(
                                "AuthenticationType='%s' is not supported.",
                                authType.toString() )
                        );
            }
            ServerUtils.validateNotEmptyUsername(userName);
            ValidateUtil.validateNotNull(pwd, "pwd");
        } catch (Exception ex)
        {
           logger.error(ex.getMessage());
           throw ex;
        }//Validation done.

        try {
            Collection<URI> uris = Collections.unmodifiableList(Arrays.asList(new URI(providerUri)));
            // connection must be closed if succeeded ....
            try(ILdapConnectionEx connection =
                    ServerUtils.getLdapConnectionByURIs(uris, userName, pwd, AuthenticationType.PASSWORD, false, certValidationSettings))
            {}
            return;
        }
        catch (Exception ex)
        {
            String msg = String.format(
                    "Failed to probe provider connectivity [URI: %s]; tenantName [%s], userName [%s]",
                    providerUri,
                    tenantName,
                    userName);

            logger.warn(msg);

            throw new IDMLoginException(msg, uri, ex);
        }
    }

    private void probeProviderConnectivity(String tenantName, IIdentityStoreData idsData) throws Exception {

        ValidateUtil.validateNotNull(idsData.getExtendedIdentityStoreData(), "idsData details");
        if( AuthenticationType.PASSWORD != idsData.getExtendedIdentityStoreData().getAuthenticationType() )
        {
             throw new IllegalArgumentException(String.format("AuthenticationType='%s' is not supported.",
                            idsData.getExtendedIdentityStoreData().getAuthenticationType().toString()));
        }
        ServerUtils.validateNotEmptyUsername(idsData.getExtendedIdentityStoreData().getUserName());
        ValidateUtil.validateNotNull(idsData.getExtendedIdentityStoreData().getPassword(), "pwd");

        IIdentityProvider provider = providerFactory.buildProvider(tenantName, idsData, idsData.getExtendedIdentityStoreData().getCertificates());
        if (!(provider instanceof BaseLdapProvider))
            throw new IllegalArgumentException(String.format("Supported provider type is %s, %s provider is not of supported type.", BaseLdapProvider.class.toString(),
                    provider.getClass().toString()));

        StringBuilder connections = new StringBuilder();
        for (String connectionStr : idsData.getExtendedIdentityStoreData().getConnectionStrings()) {
            ValidateUtil.validateNotEmpty(connectionStr, "connectionString");

            try {
                URI connectionUri = new URI(connectionStr);
                DirectoryStoreProtocol protocol = DirectoryStoreProtocol.getValue(connectionUri.getScheme().toString());
                if (protocol  == null)
                {
                    throw new IllegalArgumentException(String.format("Unsupported providerUri='%s'.", connectionUri ));
                }
                else if (protocol == DirectoryStoreProtocol.LDAPS)
                {
                    ValidateUtil.validateNotEmpty(idsData.getExtendedIdentityStoreData().getCertificates(), "IdentityStore certificates");
                    for (X509Certificate cert : idsData.getExtendedIdentityStoreData().getCertificates()) {
                        ValidateUtil.validateCertificate(cert);
                    }
                }
                connections.append(connectionStr + " ");
            } catch (URISyntaxException e) {
                throw new IllegalArgumentException(e.getMessage(), e);
            }
        }

        try
        {
            //probe connectivity and DN check
            ((BaseLdapProvider)provider).probeConnectionSettings();
        }
        catch (Exception ex)
        {
            String msg = String.format("Failed to probe provider connectivity [URI: %s]; tenantName [%s], userName [%s]",
                    connections.toString(),
                    tenantName,
                    idsData.getExtendedIdentityStoreData().getUserName());

            logger.warn(msg);

            throw new IDMLoginException(msg, null, ex);
        }
    }

    private
    Collection<String>
    getDefaultProviders(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getDefaultProviders();
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get default providers for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    private
    void
    setDefaultProviders(
            String             tenantName,
            Collection<String> defaultProviders
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            // per admin interface setting null is allowed meaning erase the default provider settings.

            _configStore.setDefaultProviders(tenantName, defaultProviders);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "Default Providers successfully set for tenant [%s]",
                    tenantName));
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to set default providers for tenant [%s]",
                            tenantName));

            throw ex;
        }
            }


    /**
     * Authenticates a principal using the password specified
     *
     * The principal is expected to be managed by one of the Identity providers
     * configured in the tenant.
     *
     * @param tenantName Name of tenant. non-null non-empty, required
     * @param principal  User principal to be authenticated. non-null non-empty, required
     * @param password   Password.  non-null non-empty, required
     * @return Normalized principal if successfully authenticated.
     * @throws IDMLoginException when authentication failed.
     * @throws PasswordExpiredException when authentication failed due to
     *         expired password.
     * @throws UserAccountLockedException when authentication failed due to
     *         locked user account.
     * @throws Exception
     */
    private
    PrincipalId
    authenticate(
            String    tenantName,
            String    principal,
            String    password
            ) throws Exception
            {
        long startTime = System.nanoTime();
        boolean authFailed = false;

        PrincipalId userPrincipal = null;
        IIdentityProvider provider = null;

        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ServerUtils.validateNotEmptyUsername(principal);
            ValidateUtil.validateNotNull(password, "Password");

            TenantInformation tenantInfo = findTenant(tenantName);

            if (tenantInfo == null)
            {
                throw new IDMLoginException("Access denied");
            }

            userPrincipal = ServerUtils.getUserPrincipal(tenantInfo, principal);
            if( userPrincipal == null )
            {
                throw new IDMLoginException(
                        String.format( "Invalid user principal '%s'.", principal )
                        );
            }

            provider = tenantInfo.findProviderADAsFallBack(userPrincipal.getDomain());

            if (provider == null)
            {
                throw new IDMLoginException("Access denied");
            }

            String identityProviderName = provider.getName();

            validateProviderAllowedAuthnTypes(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_PASSWORD,
                                              identityProviderName,
                                              tenantInfo);

            return provider.authenticate(userPrincipal, password);
        }
        catch (InvalidPrincipalException e) {
            authFailed = true;
            logger.error(
                    VmEvent.USER_NAME_PWD_AUTH_FAILED,
                    "Failed to authenticate principal [{}]. Invalid principal.",
                     (principal != null ? principal : "null")
                );
            throw e;
        }
        catch(Exception ex)
        {
            authFailed = true;

            logger.error(
                    String.format(
                            "Failed to authenticate principal [%s] for tenant [%s]",
                            principal != null ? principal : "null",
                                    tenantName != null ? tenantName : "null"), ex);

            if (provider != null && userPrincipal != null)
            {
                try
                {
                    provider.checkUserAccountFlags(userPrincipal);
                }
                catch(UserAccountLockedException ex1)
                {
                    logger.error(
                        VmEvent.USER_NAME_PWD_AUTH_FAILED,
                        "Failed to authenticate principal [{}]. User account locked.",
                         (principal != null ? principal : "null")
                    );

                    throw ex1;
                }
                catch( PasswordExpiredException ex1 )
                {
                    logger.error(
                        VmEvent.USER_NAME_PWD_AUTH_FAILED,
                        "Failed to authenticate principal [{}]. User password expired.",
                         (principal != null ? principal : "null")
                    );
                    throw ex1;
                }
                catch( Exception ex2 )
                {
                    logger.error(
                            String.format(
                                    "Failed to checkUserAccountFlags principal [%s] for tenant [%s]",
                                    principal != null ? principal : "null",
                                            tenantName != null ? tenantName : "null"));
                    // we are ignoring this exception here, because we want to propagate the original
                    // authenticate failure, and not this failure
                }
            }

            if (ex instanceof LoginException)
            {   //Kerberos exception needs to be handled here
                Throwable t = ex.getCause();
                if (t instanceof KrbException)
                {  // get the Kerberos return code only when the cause if KrbException
                    int returnCode = ((KrbException)t).returnCode();
                    switch (returnCode)
                    {
                    case ServerKrbUtils.KDC_ERR_CLIENT_REVOKED:
                        {
                            logger.error(
                                VmEvent.USER_NAME_PWD_AUTH_FAILED,
                                "Failed to authenticate principal [{}]. User account locked.",
                                 (principal != null ? principal : "null")
                            );

                        throw new UserAccountLockedException(ex.getMessage());
                       }
                    case ServerKrbUtils.KDC_ERR_KEY_EXPIRED:
                        {
                            logger.error(
                                VmEvent.USER_NAME_PWD_AUTH_FAILED,
                                "Failed to authenticate principal [{}]. User password expired.",
                                 (principal != null ? principal : "null")
                            );
                        throw new PasswordExpiredException(ex.getMessage());
                        }
                    default://no op for others
                        break;
                    }
                }
            }

            logger.error(
                VmEvent.USER_NAME_PWD_AUTH_FAILED,
                String.format(
                     "Failed to authenticate principal [%s]. %s",
                     principal != null ? principal : "null",
                     ex.getMessage()
                ),
                ex
            );

            if (ex instanceof AccountLockedOutException)
            {
                throw new UserAccountLockedException(ex.getMessage());
            }
            else if (ex instanceof AccountPasswordExpiredException)
            {
                throw new PasswordExpiredException(ex.getMessage());
            }

            throw new IDMLoginException(ex.getMessage());
        }
        finally
        {
            long delta = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime);

            if (logger.isInfoEnabled())
            {
                if (provider == null)
                {
                    logger.info(
                        String.format(
                                     "Authentication %s for user [%s] in tenant [%s] in [%d] milliseconds because the provider is not registered",
                                     authFailed ? "failed" : "succeeded",
                                             principal,
                                             tenantName,
                                             delta));
                }
                else
                {
                    logger.info(
                        String.format(
                                "Authentication %s for user [%s] in tenant [%s] in [%d] milliseconds with provider [%s] of type [%s]",
                                authFailed ? "failed" : "succeeded",
                                        principal,
                                        tenantName,
                                        delta,
                                        provider.getName(),
                                        provider.getClass().getName()));
                }
            }

            PerfDataSinkFactory.getPerfDataSinkInstance().addMeasurement(
                    new PerfBucketKey(
                            PerfMeasurementPoint.IDMAuthenticate,
                            principal),
                            delta);
        }
            }

    /**
     * A helper method that will check if requested authentication type is supported based on a provider (user belonging to)
     * @param requestedAuthnType authentication type to check if it is associated with provider
     * @param requestedProvider Name of identity source
     * @param tenantInfo tenant information
     */
    public static void validateProviderAllowedAuthnTypes(int requestedAuthnType, String requestedProvider, TenantInformation tenantInfo) throws IDMLoginException {
        boolean authenticationAllowed = false;

        Collection<IIdentityStoreData> idsStores = tenantInfo.getIdsStores(); // All identity sources on tenant
        IIdentityStoreData identitySource = null;

        // Retrieve information of requested identity source
        for(IIdentityStoreData identityStore : idsStores ) {
            if(identityStore.getName().equalsIgnoreCase(requestedProvider)) {
               identitySource = identityStore;
               if(identitySource != null){
                   IIdentityStoreDataEx extendedData = identitySource.getExtendedIdentityStoreData();
                   if(extendedData.getAuthnTypes() != null) {
                       int[] providerAuthnTypes = extendedData.getAuthnTypes();
                       if(ArrayUtils.contains(providerAuthnTypes, requestedAuthnType)){
                           authenticationAllowed = true;
                       }

                       if(!authenticationAllowed) {
                           String errMessage = String.format("Authentication type : '%s' is not allowed for requested identity provider : '%s'", requestedAuthnType, requestedProvider);
                           throw new IDMLoginException(errMessage);
                       }
                   }
               }
               break;
            }
        }
    }

    private
    GSSResult
    authenticate(
            String tenantName,
            String contextId,
            byte[] gssTicket
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(gssTicket, "GSSAPI Token");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IGssAuthIdentityProvider provider = GSSAuthProvider.getInstance();

            if (provider == null)
            {
                logger.error(
                        VmEvent.GSS_AUTH_FAILED,
                        "Failed to authenticate gss token. Unable to find an ActiveDirectoryProvider."
                    );
                String message = String.format(
                        "Unable to find an GSS Provider for tenant [%s].",
                        (tenantName != null ? tenantName : "null"));
                logger.error(message);

                throw new IDMLoginException(message);
            }

            GSSAuthResult result = provider.authenticate(contextId, gssTicket);

            if (result.getUserInfo() != null)
            {
                IIdentityProvider adIdentityProvider = tenantInfo.findProviderADAsFallBack(result.getUserInfo().getDomain());
                if (adIdentityProvider == null) {
                    throw new NoSuchIdpException("Native AD Provider does not exist.");
                }
                //save PAC info to ActiveDirectoryProvider if the user domain is from AD provider
                if (adIdentityProvider instanceof ActiveDirectoryProvider)
                {
                    ActiveDirectoryProvider adProvider = ((ActiveDirectoryProvider)adIdentityProvider);
                    adProvider.saveUserInfo(result.getUserInfo());
                }

                return new GSSResult(
                    contextId,
                    new PrincipalId(result.getUserInfo().getName(), result.getUserInfo().getDomain()));
            }
            else
            {
                return new GSSResult(contextId, result.getServerLeg());
            }
        }
        catch(Exception ex)
        {
            logger.error(
                    VmEvent.GSS_AUTH_FAILED,
                    "Failed to authenticate gss token",
                    ex
                );
            logger.error(
                    String.format(
                            "Failed to authenticate gss token for tenant [%s]",
                            (tenantName != null ? tenantName : "null")));

            throw ex;
        }
    }

    /**
     * TLS Client Certificate (or smartcard) authentication.
     *
     * This function does the following Certificate path validation, revocation
     * check Subject validation OID filtering
     *
     * @param tenantName
     * @param tlsCertChain
     *            certificate chain may or may not provide full client cert
     *            chain.
     * @return principal matched
     * @throws IDMLoginException
     *             certificate not provided or can not find a matching user in
     *             directory
     * @throws IdmCertificateRevokedException
     *             certificate revoked
     * @throws InvalidArgumentException
     *             parameter was incorrectly set
     * @throws CertificateRevocationCheckException
     *             revocation check fails to determine the certificate status
     * @throws IDMException
     *             any other exceptions
     */
    private PrincipalId authenticate(String tenantName,
                X509Certificate[] tlsCertChain, String hint) throws IDMLoginException,CertificateRevocationCheckException,
                InvalidArgumentException, IdmCertificateRevokedException, IDMException{

        TenantInformation info;

        try {
            info = findTenant(tenantName);
        }
        catch (Exception e)
        {
            throw new IDMLoginException("Error in retrieve tenantInfo");
        }

        if (tlsCertChain == null || tlsCertChain.length < 1) {
            logger.error("Certificate chain is empty or null");
            throw new IDMLoginException("Certificate chain is empty or null");
        }

        if (logger.isDebugEnabled()) {
            for (int i = 0; i < tlsCertChain.length; i++) {
                logger.debug("Client Certificate [" + i + "] = "
                                + tlsCertChain[i].toString());
            }
        }

        X509Certificate targetCert = tlsCertChain[0];

        String subjectDn = targetCert.getSubjectDN() != null?
                targetCert.getSubjectDN().toString() : "";

        IIdmAuthStatRecorder recorder = PerformanceMonitorFactory.createIdmAuthStatRecorderInstance(
                tenantName,
                "CertificateAuthentication",
                "IDM",
                0,
                IIdmAuthStat.ActivityKind.AUTHENTICATE,
                IIdmAuthStat.EventLevel.INFO,
                subjectDn);
        recorder.start();
        long startTime = System.nanoTime();

        AuthnPolicy aPolicy = info.getAuthnPolicy();

        Validate.notNull(aPolicy, "AuthnPolicy can not be null.");
        Validate.isTrue(aPolicy.IsTLSClientCertAuthnEnabled(),
                "TLSClient authn is not enabled.");

        ClientCertPolicy certPolicy = aPolicy.getClientCertPolicy();
        Validate.notNull(certPolicy,
                "Client Certificate Policy can not be null.");

        IdmClientCertificateValidator certValidator = new IdmClientCertificateValidator(certPolicy, info);

        Map<String, String> authStatsExtension = new HashMap<String, String> ();
        recorder.add(authStatsExtension);
        String clusterID;

        //Step1 validate certificate
        try {
            clusterID = this.getClusterId();
        } catch (Exception e1) {
            throw new IDMException("Failed to retrieve PSC cluster ID.");
        }
        certValidator.validateCertificatePath(targetCert, clusterID, authStatsExtension);

        //Principal account mapping and validation.
        PrincipalId principal = certValidator.certificateAccountMapping(targetCert, hint);

        authStatsExtension.put("Account mapping", String.format("%d Ms", TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime)));
        recorder.end();

        return principal;
    }


    /**
     * authenticate with secure ID
     *
     * @param tenantName
     * @param principal
     * @param passcode
     * @return
     * @throw IDMLoginExceptin  if credential is denied
     * @throw IDMRsaSecurIDNewPinException  if user need to setup new pin.
     * @throws IDMException all other errors.
     */
    private RSAAMResult authenticateRsaSecurId(String tenantName, String sessionId,
            String userName, String passcode)
            throws IDMException

    {
        long startTime = System.nanoTime();
        boolean authFailed = false;
        RSAAMResult authResult = null;

        logger.debug("Authenticating with RSA securID ..");

        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(userName, "User Principal");

            TenantInformation info = findTenant(tenantName);
            AuthnPolicy aPolicy = info.getAuthnPolicy();
            Validate.notNull(aPolicy, "AuthnPolicy can not be null.");
            Validate.isTrue(aPolicy.IsRsaSecureIDAuthnEnabled(), "SecureID authentication is not turned on for this tenant.");

            RSAAgentConfig rsaConfig = aPolicy.get_rsaAgentConfig();
            Validate.notNull(rsaConfig, "RSAAgentConfig is not defined");

            HashMap<String, String> userIDAttrMap = rsaConfig.get_idsUserIDAttributeMap();

            // we should not need to create api all the time; but different tenant should have different api
            AuthenticationSessionFactory api = null;

            String[] userInfo = ServerUtils.separateUserIDAndDomain(userName);
            IIdentityProvider provider = info.findProviderADAsFallBack(userInfo[1]);

            if (null == provider) {
                throw new IDMLoginException(String.format(
                        "Identity source was not defined for user: %s.", userName));
            }
            validateProviderAllowedAuthnTypes(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID,
                                                provider.getName(),
                                                info);

            api = this._rsaSessionFactoryCache.getAuthnFactory(info);

            String userID = extractRsaUserID(info, userName, userIDAttrMap);
            AuthenticationSession session = null;
            String cachedSessionId = sessionId;
            PrincipalId pId = getPrincipalIDFromUserName(info, userName, userIDAttrMap);
            try
            {
                // Retrieve session if this it is provided
                if (cachedSessionId != null) {
                    session = IdentityManager._rsaSessionCache.getSession(tenantName,cachedSessionId);
                }
                // generate a rsa session if not found in rsa session cache
                if (session == null) {
                    logger.debug("Using new AuthSession ...");
                    session = api.createSession();
                    String newSessionId = createSessionId();

                    int status = session.authenticate(userID, new AuthenticationSecret(passcode)).getStatusCode();

                    if (status == AuthenticationResult.NEXT_CODE_REQUIRED) {
                        IdentityManager._rsaSessionCache.addSession(tenantName, newSessionId, session);
                    }

                    authResult = afterProcessRSAStatus(status,
                            newSessionId, userName, pId);
                } else {
                    logger.debug("Using cached AuthSession, in second leg of NEXT_CODE_REQUIRED mode ...");

                    //It must be in nextcode mode if the session is found
                    int prevStatus = session.getAuthenticationStatus().getStatusCode();
                    if (prevStatus != AuthenticationResult.NEXT_CODE_REQUIRED) {
                        throw new IDMLoginException(String.format(
                                "Unexpected status in a cached rsa session: %s.", prevStatus));
                    }

                    int status = session.nextAuthenticationStep(new AuthenticationSecret(passcode)).getStatusCode();
                    authResult = afterProcessRSAStatus(status,
                            cachedSessionId, userName, pId);

                    if (status != AuthenticationResult.NEXT_CODE_REQUIRED) {
                        IdentityManager._rsaSessionCache.removeSession(tenantName, cachedSessionId);
                    }
                }
            } finally
            {
                if (session.getAuthenticationStatus().getStatusCode() != AuthenticationResult.NEXT_CODE_REQUIRED) {
                    session.closeSession();
                }
            }
        } catch (IDMLoginException ex) {
            authFailed = true;
            throw ex;
        } catch (IDMException ex) {
            // don't wrap it.
            authFailed = true;
            throw ex;
        } catch (Exception ex)
        {
            authFailed = true;
            logger.error(
                    String.format(
                            "Failed to authenticate principal [%s] by passcode",
                            userName != null ? userName : "null"), ex);

            throw new IDMException(ex.getMessage());
        } finally
        {
            long delta = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime);

            if (logger.isInfoEnabled())
            {
                logger.info(
                        String.format(
                                "Authentication %s for user [%s] in tenant [%s] in [%d] milliseconds with rsa secureID",
                                authFailed ? "failed" : "succeeded or entered \"NEXT_CODE_REQUIRED\" mode",
                                userName,
                                tenantName,
                                delta));

            }

            PerfDataSinkFactory.getPerfDataSinkInstance().addMeasurement(
                    new PerfBucketKey(
                            PerfMeasurementPoint.IDMAuthenticate,
                            userName),
                    delta);
        }
        return authResult;
    }

    private String createSessionId() {
        SecureRandom randomGenerator = new SecureRandom();
        int sessionId = randomGenerator.nextInt(1000000);
        return String.valueOf(sessionId);
    }

    /**
     *
     * Handle rsa session status.
     *
     * Throw at NewPin mode. User need to setup new pin via rsa tool rather than our login page.
     *
     * For each status do the following 1. log the event 2. update rsa session
     * cache. 3. throw or return RSAAMResult.
     *
     * @param status
     *            Normal return conditions:
     *            ACCESS_OK NEW_PIN_REQUIRED NEXT_CODE_REQUIRED,PIN_ACCEPTED
     *            Throwing conditions:
     *            ACCESS_DENIED, NEXT_CODE_BAD,PIN_REJECTED,
     * @param cachedSessionId
     * @param userName
     *            userName used at login
     *            -need in status of succeed.
     * @return RSAAMResult
     * @throws IDMLoginException
     * @throws IDMSecureIDNewPinException if the status is AuthSession.NEW_PIN_REQUIRED.
     */
    private RSAAMResult afterProcessRSAStatus(int status,
            String cachedSessionId, String userName, PrincipalId pId)
            throws IDMLoginException, IDMSecureIDNewPinException
    {
        Validate.notEmpty(userName, "Empty userName");
        RSAAMResult result;

        switch (status) {
            case AuthenticationResult.ACCESS_OK:
                logger.info(String
                        .format("Successfully authenticating principal [%s] by passcode.",
                                userName));
                result = new RSAAMResult(pId);
                break;
            case AuthenticationResult.ACCESS_DENIED:
                logger.error(String
                        .format("Failed authenticating principal [%s] by passcode.",
                                userName));
                throw new IDMLoginException(String.format(
                        "RSA status: ACCESS_DENIED."));
            case AuthenticationResult.NEXT_CODE_BAD:
                //Next passcode failed to pass authentication.
                logger.error(String
                        .format("Failed authenticating principal [%s] by passcode.",
                                userName));
                throw new IDMLoginException(String.format(
                        "RSA status: NEXT_CODE_BAD."));
            case AuthenticationResult.NEXT_CODE_REQUIRED:
                logger.info(String
                        .format("Next code required authenticating principal [%s] by passcode.RSA SessionID [%s]",
                                userName, cachedSessionId));
                result = new RSAAMResult(cachedSessionId);
                break;
            case AuthenticationResult.NEW_PIN_REQUIRED:
                logger.info(String
                        .format("New pin required authenticate principal [%s] by passcode.",
                                userName));
                throw new IDMSecureIDNewPinException(String.format(
                        "RSA status: PIN_REJECTED."));
            default:
                throw new IDMLoginException(
                        String.format(
                                "Unexpected RSA AM status:  %d",
                                status));
        }

        return result;
    }

    private String extractRsaUserID(TenantInformation info, String userName, HashMap<String, String> userIDAttrMap) throws Exception {

        Validate.notNull(info, "info");

        String[] userInfo = ServerUtils.separateUserIDAndDomain(userName);
        if (userInfo == null) {
            throw new IDMLoginException(String.format(
                    "User name %s does not contain the domain - expected in format of name@domain",
                    userName));
        }

        String userID;
        if (userIDAttrMap == null || userIDAttrMap.isEmpty()) {
            userID = userName;
        } else {
            IIdentityProvider provider = info.findProviderADAsFallBack(userInfo[1]);
            String ldapAttr = userIDAttrMap.get(provider.getName());

            if (ldapAttr == null || ldapAttr.equals("userPrincipalName")) {
                userID = userName;
            } else {
                userID = userInfo[0];
            }
        }
        return userID;
    }

    /**
     * UserName is could be UPN or userID+domain. UserID here is the ldap attribute value used to identity the user.
     * @param userName
     *            user name for securID login
     * @return
     * @throws Exception
     */
    private PrincipalId getPrincipalIDFromUserName(TenantInformation info, String userName, HashMap<String, String> userIDAttrMap) throws Exception {

        String[] userInfo = ServerUtils.separateUserIDAndDomain(userName);
        if (userInfo == null) {
            throw new IDMLoginException(String.format(
                    "User name %s does not contain the domain - expected in format of name@domain",
                    userName));
        }

        if (userIDAttrMap == null || userIDAttrMap.isEmpty()) {
            return new PrincipalId(userInfo[0], userInfo[1]);
        }

        IIdentityProvider provider = info.findProviderADAsFallBack(userInfo[1]);
        String ldapAttrName = userIDAttrMap.get(provider.getName());

        if (ldapAttrName == null || ldapAttrName.equals("userPrincipalName") ) {
            return new PrincipalId(userInfo[0], userInfo[1]);
        } else {
            // find the user with the ldap attribute
            PrincipalId pID = provider.findActiveUser(ldapAttrName, userInfo[0]);
            return pID;
        }
    }

    public static
    boolean
    IsActive(
                    TenantInformation tenantInfo, String tenantName,
            PrincipalId principal
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(principal, "User Principal");
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    principal.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, principal.getDomain());


            return provider.IsActive(principal);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to check if principal [%s@%s] is active for tenant [%s]",
                            principal != null ? principal.getName() : "null",
                                    principal != null ? principal.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
    }

    private
    Collection<Attribute>
    getAttributeDefinitions(
            String tenantName
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            return tenantInfo.getAttributeDefinitions();
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get attribute definitions in tenant [%s].",
                            tenantName));

            throw ex;
        }
    }

    private
    Collection<AttributeValuePair>
    getAttributeValues(
            String      tenantName,
            PrincipalId principal,
            Collection<Attribute> attributes
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(principal, "User Principal");
            ValidateUtil.validateNotNull(attributes, "Attributes");
            ValidateUtil.validatePositiveNumber(attributes.size(), "Attribute count");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    principal.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, principal.getDomain());

            Collection<AttributeValuePair> attributeValues =
                    new HashSet<AttributeValuePair>();

            long startTime = System.nanoTime();

            String samlGroupAttrName = tenantInfo.findSystemProvider()
                    .getMappingSamlAttributeForGroupMembership();
            ISystemDomainIdentityProvider systemProvider = tenantInfo.findSystemProvider();

            Collection<AttributeValuePair> retAttrs = provider.getAttributes(principal, attributes);

            AttributeValuePair sids = null;
            AttributeValuePair groupNames = null;

            for (AttributeValuePair attr : retAttrs)
            {
                if (attr.getAttrDefinition().getName().equalsIgnoreCase(INTERNAL_ATTR_GROUP_OBJECTIDS))
                {
                    sids = attr;
                }
                else if (attr.getAttrDefinition().getName().equalsIgnoreCase(samlGroupAttrName))
                {
                    groupNames = attr;
                }
                else
                {
                    attributeValues.add(attr);
                }
            }

            if (!(provider instanceof ISystemDomainIdentityProvider))
            {
                List<String> groupsInSp = new ArrayList<String>();
                if ((sids != null) && (sids.getValues() != null) && (!sids.getValues().isEmpty()))
                {
                    try
                    {
                        groupsInSp = systemProvider.findGroupsForFsps(sids.getValues());
                    }
                    catch(Exception e)
                    {
                        logger.trace(
                                String.format(
                                        "Failed to determine FSP for principal [%s] or any of its groups in tenant [%s]",
                                                        principal != null ? principal.getUPN() : "null",
                                                        tenantName));
                    }

                    if (groupNames != null && groupsInSp != null && groupsInSp.size() > 0)
                    {
                        for (String groupName : groupsInSp)
                        {
                            groupNames.getValues().add(groupName);
                        }
                    }
                }
           }
           if (groupNames != null)
           {
               attributeValues.add(groupNames);
           }

            addEveryoneGroupTo(attributeValues, samlGroupAttrName, systemProvider);

            //only measure time of successful results
            PerfDataSinkFactory.getPerfDataSinkInstance().addMeasurement(
                    new PerfBucketKey(
                            PerfMeasurementPoint.IDMGetAttributeValues,
                            principal.getDomain()),
                            TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime));

            return attributeValues;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get attributes for principal [%s@%s] in tenant [%s]",
                            principal != null ? principal.getName() : "null",
                                    principal != null ? principal.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
    }

    /**
     * Add Everyone@<systemProviderDomain> to attribute values without duplicates
     *
     * @param attributeValues
     *           collection of {@code AttributeValuePair}. The Everyone
     *           group is going to be added if it does not exist.
     * @param systemProviderDomain
     *           domain name of the system provider
     */
    private void addEveryoneGroupTo(
          Collection<AttributeValuePair> attributeValues, String samlGroupAttr,
          ISystemDomainIdentityProvider systemProvider) throws Exception
    {
       //If system provider does not provide groupMembership info ==> no-op
       if (samlGroupAttr != null)
       {
           for (AttributeValuePair currentPair : attributeValues)
           {
              if (currentPair.getAttrDefinition().getName().equalsIgnoreCase(samlGroupAttr))
              {
                  Group g = systemProvider.getEveryoneGroup();
                 String everyoneGroupNetbios = g.getNetbios();
                 String everyoneAliasNetbios = ServerUtils.getGroupAliasNetbios(g);
                 if (currentPair.getValues() == null)
                 {
                    throw new IllegalStateException(
                          "SAML attribute definition Group is found but associated value list is null");
                 }
                 if (!currentPair.getValues().contains(everyoneGroupNetbios))
                 {//don't add duplicates
                    currentPair.getValues().add(everyoneGroupNetbios);
                 }
                 if (
                        (ServerUtils.isNullOrEmpty(everyoneAliasNetbios) == false)
                        &&
                        (!currentPair.getValues().contains(everyoneAliasNetbios))
                    )
                 {
                    currentPair.getValues().add(everyoneAliasNetbios);
                 }
                 break;
              }
           }
       }
    }

    private
    PrincipalId
    addSolutionUser(
        String         tenantName,
        String         userName,
        SolutionDetail detail
        ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(userName, "User name");
            ValidateUtil.validateNotNull(detail, "Solution user detail");
            // Validate solution user detail: make sure its certificate is valid
            ValidateUtil.validateSolutionDetail(detail, "Solution user detail",
                    this.getClockTolerance(tenantName));

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.addServicePrincipal(userName, detail);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to add solution user [%s] in tenant [%s]",
                            userName,
                            tenantName));

            throw ex;
        }
    }

    private
    SolutionUser
    findSolutionUser(
            String tenantName,
            String userAccount
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ServerUtils.validateNotEmptyUsername(userAccount);

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                return provider.findServicePrincipal(userAccount);
            }
            else
            {
                return provider.findServicePrincipalInExternalTenant(userAccount);
            }
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find solution user [%s] in tenant [%s]",
                            userAccount,
                            tenantName));

            throw ex;
        }
    }

    private
    SolutionUser
    findSolutionUserByCertDn(
            String tenantName,
            String subjectDN
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(subjectDN, "Subject DN");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                return provider.findServicePrincipalByCertDn(subjectDN);
            }
            else
            {
                return provider.findServicePrincipalByCertDnInExternalTenant(subjectDN);
            }
        }
        catch(NoSuchUserException ex)
        {
            logger.info(
                    String.format(
                            "Failed to find solution user by subject DN [%s] in tenant [%s]",
                            subjectDN,
                            tenantName));

            return null;
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find solution user by subject DN [%s] in tenant [%s]",
                            subjectDN,
                            tenantName));

            throw ex;
        }
            }

    /**
     * Retrieve user hashed password blob
     *
     * @param tenantName Name of tenant
     * @param Principal id of the user
     * @return User's hashed password blob
     * @throws IDMException
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws NoSuchIdpException   - system tenant is not set up.
     * @throws InvalidArgumentException    -- if the tenant name is null or empty
     * @throws InValidPrincipleException    - if user does not exist or multiple ones are found
     */
    private
    byte[]
    getUserHashedPassword(
        String tenantName,
        PrincipalId principal
    ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(principal, "User principal");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    principal.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, principal.getDomain());

            ISystemDomainIdentityProvider systemProvider = null;
            if( provider instanceof ISystemDomainIdentityProvider )
            {
                systemProvider = (ISystemDomainIdentityProvider)provider;
            }

            if( systemProvider != null )
            {
                return systemProvider.getUserHashedPassword(principal);
            }
            else
            {
                PersonUser user = provider.findUser(principal);

                if (user == null)
                {
                    throw new InvalidPrincipalException(String.format("User %s is invalid in domain %s",
                            principal.getName(), principal.getDomain()), principal.getUPN());
                }

                // return null password
                return null;
            }
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get hashsed password for person user [%s@%s] in tenant [%s]",
                            principal != null ? principal.getName() : "null",
                                    principal != null ? principal.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
                    }

    private PersonUser
    findPersonUser(
            String      tenantName,
            PrincipalId id
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(id, "User principal");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(id.getDomain());

            if (null == provider)
            {
                String msg = String.format(
                        "PrincipalId [%s] does not match any providers on primary system.",
                        id.toString());
                logger.info(msg);
                return null;  // caller will try to resolve it as external IDP registered user.
            }
            else
            {
                return provider.findUser(id);   //return null if not found
            }
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find person user [%s@%s] in tenant [%s]",
                            id != null ? id.getName() : "null",
                                    id != null ? id.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
            }

    private
    PersonUser
    findPersonUserByObjectId(
            String    tenantName,
            String    userObjectId
            ) throws Exception
            {
        PersonUser user = null;

        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(userObjectId, "User objectId");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            // Look in Tenant's SP first
            ISystemDomainIdentityProvider systemProvider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(systemProvider, tenantName);

            try
            {
                user = systemProvider.findUserByObjectId(userObjectId);
            }
            catch(Exception ex)
            {// not necessary an error, just log info
                logger.info(
                        String.format(
                                "cannot find person user with objectId [%s] in Service Provider [%s]",
                                userObjectId != null ? userObjectId : "null",
                                        systemProvider.getDomain()));
            }

            if (user != null)
                return user;

            // Look in Tenant's ID store iteratively until found
            Collection<IIdentityProvider> providers = tenantInfo.getProviders();

            for (IIdentityProvider provider : providers)
            {
                try
                {
                    assert(provider != null);
                    user = provider.findUserByObjectId(userObjectId);
                }
                catch(Exception ex)
                {// not necessary an error, just log info
                    logger.info(
                            String.format(
                                    "cannot find person user with objectId [%s] in provider [%s]",
                                    userObjectId != null ? userObjectId : "null",
                                            provider.getDomain()));
                    continue;
                }

                if (user != null)
                    break;
            }

            return user;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find person user with objectId [%s] in tenant [%s]",
                            userObjectId != null ? userObjectId : "null",
                                    tenantName));

            throw ex;
        }
            }

    private
    Set<SolutionUser>
    findSolutionUsers(
            String tenantName,
            String searchString,
            int limit
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(searchString, "Search string");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                return provider.findServicePrincipals(searchString);
            }
            else
            {
                return provider.findServicePrincipalsInExternalTenant(searchString);
            }
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find solution users [criteria : %s] in tenant [%s]",
                            searchString,
                            tenantName));

            throw ex;
        }
            }
    /**
     * Finds a security group defined in one of the tenant's identity providers
     *
     * @param tenantName Name of tenant
     * @param id         Group to be found
     * @return Principal id of the security group. null if no such group
     * @throws IDMException
     * @throws RemoteException
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     *         IDMEception              - any other exceptions could be returned.
     */
    private
    Group
    findGroup(
            String      tenantName,
            PrincipalId groupId
            ) throws Exception
            {
        Group candidate = null;

        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(groupId, "Group name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    groupId.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, groupId.getDomain());

            candidate = provider.findGroup(groupId);
            // Provider fulfills the contract return null in case group does not exist
            if (candidate == null)
            {
                logger.info(
                        String.format(
                                "Failed to find group [%s@%s] in tenant [%s]",
                                groupId.getName(),
                                groupId.getDomain(),
                                tenantName));

                throw new InvalidPrincipalException(
                        String.format("Group [%s] could not be found for tenant [%s]",
                                groupId.getName(), tenantName),
                        ServerUtils.getUpn(groupId)
                        );
            }

            return candidate;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find group [%s@%s] in tenant [%s]",
                            groupId != null ? groupId.getName() : "null",
                                    groupId != null ? groupId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
            }

    /**
     * Finds a security group defined in one of the tenant's identity providers
     *
     * @param tenantName Name of tenant
     * @param id         Group to be found
     * @return Group object of the security group. null if no such group
     * @throws Exception
     * @throws RemoteException
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     *         IDMEception              - any other exceptions could be returned.
     */
    private
    Group
    findGroupByObjectId(
            String tenantName,
            String groupObjectId
            ) throws Exception
            {
        Group group = null;

        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(groupObjectId, "Group objectId");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            // Look in Tenant's SP first
            ISystemDomainIdentityProvider systemProvider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(systemProvider, tenantName);

            try
            {
                group = systemProvider.findGroupByObjectId(groupObjectId);
            }
            catch(Exception ex)
            {
                logger.error(
                        String.format(
                                "Failed to find group with objectId [%s] in Service Provider [%s]",
                                groupObjectId != null ? groupObjectId : "null",
                                        systemProvider.getDomain()));
            }

            if (group != null)
                return group;

            // Look in Tenant's ID store iteratively until found
            Collection<IIdentityProvider> providers = tenantInfo.getProviders();

            for (IIdentityProvider provider : providers)
            {
                try
                {
                    assert (provider != null);
                    group = provider.findGroupByObjectId(groupObjectId);
                }
                catch(Exception ex)
                {
                    logger.error(
                            String.format(
                                    "Failed to find group with objectId [%s] in provider [%s]",
                                    groupObjectId != null ? groupObjectId : "null",
                                            provider.getDomain()));
                    continue;
                }

                if (group != null)
                    break;
            }

            return group;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find group with objectId [%s] in tenant [%s]",
                            groupObjectId != null ? groupObjectId : "null",
                                    tenantName));

            throw ex;
        }
            }

    /**
     * Finds regular users in one of the identity providers configured for the
     * tenant.
     *
     * The search criteria defines the identity domain to be searched.
     *
     * A user account is chosen for the search results if the search string is
     * part of either the account name, first name, last name, or description
     * of the account.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of the tenant
     * @param criteria   Search criteria, non negative
     * @param limit  a positive integer for the maximum number of items to return
     * @return Set of regular users found. Empty set when no user found.
     * @throws IDMException
     * @throws RemoteException
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     * @throws IDMException              - any other exceptions could be returned.
     */
    private
    Set<PersonUser>
    findPersonUsers(
            String tenantName,
            SearchCriteria criteria,
            int limit
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(criteria, "Search criteria");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    criteria.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, criteria.getDomain());

            return provider.findUsers(criteria.getSearchString(), criteria.getDomain(), limit);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find person users [Criteria : %s] in tenant [%s]",
                            criteria,
                            tenantName));

            throw ex;
        }
    }

    /**
     * Finds regular users in one of the identity providers configured for the
     * tenant.
     *
     * The search criteria defines the identity domain to be searched.
     *
     * A user account is chosen for the search results if the search string is
     * part of the account name, i.e. in AD, samAccountName is used
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of the tenant
     * @param criteria   Search criteria, non negative, this search only targets searching on accountName
     *        for users (i.e. in AD it uses samAccountName)
     * @param limit  a positive integer for the maximum number of items to return
     * @return Set of regular users found. Empty set when no user found.
     * @throws IDMException
     * @throws RemoteException
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     * @throws IDMException              - any other exceptions could be returned.
     */
    private
    Set<PersonUser>
    findPersonUsersByName(
        String tenantName,
        SearchCriteria criteria,
        int limit
        ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(criteria, "Search criteria");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    criteria.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, criteria.getDomain());

            return provider.findUsersByName(criteria.getSearchString(), criteria.getDomain(), limit);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find person users [Criteria : %s] in tenant [%s]",
                            criteria,
                            tenantName));

            throw ex;
        }
    }

    private
    Set<Group>
    findGroups(
            String         tenantName,
            SearchCriteria criteria,
            int            limit
            ) throws Exception
   {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(criteria, "Search criteria");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    criteria.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, criteria.getDomain());

            return provider.findGroups(criteria.getSearchString(), criteria.getDomain(), limit);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find groups [Criteria : %s] in tenant [%s]",
                            criteria,
                            tenantName));

            throw ex;
        }
    }

    // the search is limited to search only on accountNames, i.e. for AD, samAccountName is searched
    private
    Set<Group>
    findGroupsByName(
        String         tenantName,
        SearchCriteria criteria,
        int            limit
        ) throws Exception
   {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(criteria, "Search criteria");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    criteria.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, criteria.getDomain());

            return provider.findGroupsByName(criteria.getSearchString(), criteria.getDomain(), limit);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find groups [Criteria : %s] in tenant [%s]",
                            criteria,
                            tenantName));

            throw ex;
        }
    }

    private
    Set<PersonUser>
    findPersonUsersInGroup(
            String      tenantName,
            PrincipalId groupId,
            String      searchString,
            int         limit
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(groupId, "Group");
            ValidateUtil.validateNotNull(searchString, "Search string");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    groupId.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, groupId.getDomain());
            ISystemDomainIdentityProvider systemProvider = null;
            if( provider instanceof ISystemDomainIdentityProvider )
            {
                systemProvider = (ISystemDomainIdentityProvider)provider;
            }

            Set<PersonUser> members = provider.findUsersInGroup(groupId, searchString, limit);
            Set<PersonUser> modifiedMembers = new HashSet<PersonUser>();

            for( PersonUser user : members )
            {
                String userId = user.getId().getName();

                if( (systemProvider != null) && (systemProvider.isObjectIdCandidate(userId)))
                {
                    String userObjectId = systemProvider.getObjectId(userId);

                    PersonUser resolvedUser = findPersonUserByObjectId(tenantName, userObjectId);

                    if (resolvedUser != null)
                    {
                        modifiedMembers.add(resolvedUser);
                    }
                }
                else
                {
                    modifiedMembers.add(user);
                }
            }

            return modifiedMembers;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find person users [Criteria : %s] in group [%s@%s] in tenant [%s]",
                            searchString,
                            groupId != null ? groupId.getName() : "null",
                                    groupId != null ? groupId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
            }

    private Set<PersonUser> findPersonUsersByNameInGroup(String tenantName, PrincipalId groupId, String searchString,
            int limit) throws Exception
    {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(groupId, "Group");
            ValidateUtil.validateNotNull(searchString, "Search string");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(groupId.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, groupId.getDomain());
            ISystemDomainIdentityProvider systemProvider = null;

            if (provider instanceof ISystemDomainIdentityProvider) {
                systemProvider = (ISystemDomainIdentityProvider) provider;
            }

            Set<PersonUser> members = provider.findUsersByNameInGroup(groupId, searchString, limit);
            Set<PersonUser> modifiedMembers = new HashSet<PersonUser>();

            for (PersonUser user : members) {
                String userId = user.getId().getName();

                if ((systemProvider != null) && (systemProvider.isObjectIdCandidate(userId))) {
                    String userObjectId = systemProvider.getObjectId(userId);

                    PersonUser resolvedUser = findPersonUserByObjectId(tenantName, userObjectId);

                    if (resolvedUser != null) {
                        modifiedMembers.add(resolvedUser);
                    }
                } else {
                    modifiedMembers.add(user);
                }
            }

            return modifiedMembers;
        } catch (Exception ex) {
            logger.error(String.format("Failed to find person users [Criteria : %s] in group [%s@%s] in tenant [%s]",
                    searchString, groupId != null ? groupId.getName() : "null", groupId != null ? groupId.getDomain()
                            : "null", tenantName));

            throw ex;
        }
    }

    private
    Set<SolutionUser>
    findSolutionUsersInGroup(
            String tenantName,
            String groupName,
            String searchString,
            int    limit
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(groupName, "Group name");
            ValidateUtil.validateNotNull(searchString, "Search string");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.findServicePrincipalsInGroup(
                    groupName,
                    searchString);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find solution users [Criteria : %s] in group [%s] in tenant [%s]",
                            searchString,
                            groupName,
                            tenantName));

            throw ex;
        }
            }

    private
    Set<Group>
    findGroupsInGroup(
            String      tenantName,
            PrincipalId groupId,
            String      searchString,
            int         limit
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(groupId, "Group");
            ValidateUtil.validateNotNull(searchString, "Search string");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    groupId.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, groupId.getDomain());

            ISystemDomainIdentityProvider systemProvider = null;
            if( provider instanceof ISystemDomainIdentityProvider )
            {
                systemProvider = (ISystemDomainIdentityProvider)provider;
            }

            Set<Group> members = provider.findGroupsInGroup(groupId, searchString, limit);
            Set<Group> modifiedMembers = new HashSet<Group>();

            for( Group group : members )
            {
                String subGroupName = group.getId().getName();

                if( (systemProvider != null) && ( systemProvider.isObjectIdCandidate(subGroupName)))
                {
                    String subgroupObjectId = systemProvider.getObjectId(subGroupName);

                    Group resolvedGroup = findGroupByObjectId(tenantName, subgroupObjectId);

                    if (resolvedGroup != null)
                    {
                        modifiedMembers.add(resolvedGroup);
                    }
                }
                else
                {
                    modifiedMembers.add(group);
                }
            }

            return modifiedMembers;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find groups [Criteria : %s] in group [%s@%s] in tenant [%s]",
                            searchString,
                            groupId != null ? groupId.getName() : "null",
                                    groupId != null ? groupId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
    }

    private Set<Group> findGroupsByNameInGroup(String tenantName, PrincipalId groupId, String searchString, int limit)
            throws Exception
    {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(groupId, "Group");
            ValidateUtil.validateNotNull(searchString, "Search string");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(groupId.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, groupId.getDomain());

            ISystemDomainIdentityProvider systemProvider = null;
            if (provider instanceof ISystemDomainIdentityProvider) {
                systemProvider = (ISystemDomainIdentityProvider) provider;
            }

            Set<Group> members = provider.findGroupsByNameInGroup(groupId, searchString, limit);
            Set<Group> modifiedMembers = new HashSet<Group>();

            for (Group group : members) {
                String subGroupName = group.getId().getName();

                if ((systemProvider != null) && (systemProvider.isObjectIdCandidate(subGroupName))) {
                    String subgroupObjectId = systemProvider.getObjectId(subGroupName);

                    Group resolvedGroup = findGroupByObjectId(tenantName, subgroupObjectId);

                    if (resolvedGroup != null) {
                        modifiedMembers.add(resolvedGroup);
                    }
                } else {
                    modifiedMembers.add(group);
                }
            }

            return modifiedMembers;
        } catch (Exception ex) {
            logger.error(String.format("Failed to find groups [Criteria : %s] in group [%s@%s] in tenant [%s]",
                    searchString, groupId != null ? groupId.getName() : "null", groupId != null ? groupId.getDomain()
                            : "null", tenantName));

            throw ex;
        }
    }


    private boolean isMemberOfSystemGroup(String tenantName, PrincipalId principalId, String groupName)
            throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(principalId, "User principal");
            ValidateUtil.validateNotEmpty(groupName, "groupName");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            ISystemDomainIdentityProvider systemProvider = tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(systemProvider, tenantName);

            // we can optimize this by introducing the groups check api in system domain provider
            // at the moment keeping existing behavior to minimize changes ...
            boolean isMember = false;
            Set<Group> directParentGroups = this.findDirectParentGroups(tenantName, principalId);

            if ( ( directParentGroups != null ) && (directParentGroups.isEmpty() == false) )
            {
                for (Group group : directParentGroups)
                {
                    if ( ( group != null ) &&
                         ( group.getDomain().equalsIgnoreCase(systemProvider.getDomain() ) ) &&
                         ( group.getName().equalsIgnoreCase(groupName)) )
                    {
                        isMember = true;
                        break;
                    }
                }
            }

            return isMember;
        }
        catch(Exception ex)
        {
            logger.error(
                String.format(
                    "Failed to check System Group Membership: tenant name=[%s], principalId=[%s], groupName=[%s]",
                    (tenantName != null) ? tenantName : "(NULL)",
                    (principalId != null) ? principalId.getUPN() : "(NULL)",
                    (groupName != null) ? groupName : "(NULL)"
                ),
                ex);
            throw ex;
        }
    }

    /**
     * Finds the set of groups that contain the specified security principal in
     * the tenant.
     *
     * The principal whose immediate parents are desired, may be a user or group
     *
     * @param tenantName  Name of tenant, required.non-null non-empty

     * @param principalId Security principal id, required, non-null.
     * @return Set of immediate parent groups found.
     * @throws IDMException
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws NoSuchIdpException   - system tenant is not set up.
     * @throws InvalidArgumentException    -- if the tenant name is null or empty
     * @throws InValidPrincipleException    - principal ID is invalid.
     * @throws IDMException         - wrapping exception for any other exceptions
     *                              from down the stack.
     */
    private
    Set<Group>
    findDirectParentGroups(
            String      tenantName,
            PrincipalId principalId
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(principalId, "User principal");

            long startedTime = System.nanoTime();

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            // (1) Find from system domain
            ISystemDomainIdentityProvider systemProvider = tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(systemProvider, tenantName);

            Set<Group> groups = new HashSet<Group>();
            PrincipalGroupLookupInfo idpGroups = null;
            List<Group> sysGroups = null;
            String fspId = null;
            // direct parent groups is a union of:
            // - set of direct parent groups from specific identity source provider
            // - set of direct parent groups from system provider for specified principal's object id
            // [if prinicipal's identity source != system provider]

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    principalId.getDomain());


            // TODO: external registered Idp support needs to be re-considered for more straightforward handling...
            if ( provider == null ) // this could be external idp registered user
            {
                fspId = getRegisteredExternalIDPUserObjectId(tenantName, principalId);
            }
            else
            {
                try
                {
                    idpGroups = provider.findDirectParentGroups(principalId);
                    if ( ( idpGroups != null ) && ( provider.getName().equalsIgnoreCase(systemProvider.getName()) == false ) )

                    {
                        fspId = idpGroups.getPrincipalObjectId();

                    }
                }
                catch(InvalidPrincipalException ex)

                {
                    // this could be external idp registered user
                    fspId = getRegisteredExternalIDPUserObjectId(tenantName, principalId);
                }
             }

            if ( ServerUtils.isNullOrEmpty(fspId) == false)

            {

                try
                {
                    sysGroups = systemProvider.findGroupObjectsForFsps(Collections.<String>singletonList(fspId));
                }
                catch(InvalidPrincipalException ex)

                {
                    logger.trace(
                        String.format(
                            "Failed to find principal [%s@%s] as FSP principal in tenant [%s]",
                            principalId != null ? principalId.getName() : "null",
                            principalId != null ? principalId.getDomain() : "null",
                            tenantName));
                    sysGroups = null;
                }
            }

            if ( (idpGroups != null) && (idpGroups.getGroups() != null) && (idpGroups.getGroups().isEmpty() == false) )

            {
                groups.addAll(idpGroups.getGroups());
            }

            if (sysGroups != null && !sysGroups.isEmpty())
            {
                groups.addAll(sysGroups);
            }

            // TODO: ideally everyone group should be added within system domain provider
            groups.add(systemProvider.getEveryoneGroup());
            PerfDataSinkFactory.getPerfDataSinkInstance().addMeasurement(
                    new PerfBucketKey(
                            PerfMeasurementPoint.IDMFindDirectParentGroups,
                            principalId.getDomain()),
                            TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedTime));
            return groups;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find direct parent groups of principal [%s@%s] in tenant [%s]",
                            principalId != null ? principalId.getName() : "null",
                                    principalId != null ? principalId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
    }

    private
    Set<Group>
    findNestedParentGroups(
        String      tenantName,
        PrincipalId principalId
        ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(principalId, "Principal Id");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider systemProvider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(systemProvider, tenantName);

            Set<Group> groups = new HashSet<Group>();
            PrincipalGroupLookupInfo idpGroups = null;
            List<Group> sysGroups = null;
            ArrayList<String> fspIds = new ArrayList<String>();

            // nested parent groups is a union of:
            // - set of nested parent groups from specific identity source provider
            // - set of fsp parent groups from system provider for { specified principal's object id + all of the nested groups ids}.
            //   [if principal's identity source != system provider]
            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    principalId.getDomain());

            // TODO: external registered Idp support needs to be re-considered for more straightforward handling...
            if ( provider == null ) // this could be external idp registered user
            {
                String externalUserId = getRegisteredExternalIDPUserObjectId(tenantName, principalId);
                if (ServerUtils.isNullOrEmpty(externalUserId) == false )
                {
                    fspIds.add(externalUserId);
                }
            }
            else
            {
                try
                {
                    idpGroups = provider.findNestedParentGroups(principalId);
                    if  ( ( idpGroups != null ) &&
                          ( provider.getName().equalsIgnoreCase(systemProvider.getName()) == false ) )
                    {
                        if ((ServerUtils.isNullOrEmpty(idpGroups.getPrincipalObjectId()) == false ))
                        {
                            fspIds.add(idpGroups.getPrincipalObjectId());
                        }

                        if ( ( idpGroups.getGroups() != null) && (idpGroups.getGroups().isEmpty() == false) )
                        {
                            for(Group g : idpGroups.getGroups())

                            {
                                if ( (g != null) && (ServerUtils.isNullOrEmpty(g.getObjectId()) == false ) )
                                {
                                    fspIds.add(g.getObjectId());

                                }
                            }
                        }
                    }
                }
                catch(InvalidPrincipalException ex)
                {
                    // this could be external idp registered user
                    String externalUserId = getRegisteredExternalIDPUserObjectId(tenantName, principalId);
                    if (ServerUtils.isNullOrEmpty(externalUserId) == false )
                    {
                        fspIds.add(externalUserId);
                    }
                }
            }

            if ( (fspIds != null) && (fspIds.isEmpty() == false) )

            {
                try
                {
                    sysGroups = systemProvider.findGroupObjectsForFsps(fspIds);
                }
                catch(InvalidPrincipalException ex)
                {
                    logger.trace(
                        String.format(
                            "Failed to find principal [%s@%s] as FSP principal in tenant [%s]",
                            principalId != null ? principalId.getName() : "null",
                            principalId != null ? principalId.getDomain() : "null",
                            tenantName));
                    sysGroups = null;
                }
            }

            if (idpGroups != null && idpGroups.getGroups() != null && !idpGroups.getGroups().isEmpty())

            {
                groups.addAll(idpGroups.getGroups());
            }

            if (sysGroups != null && !sysGroups.isEmpty())
            {
                groups.addAll(sysGroups);
            }

            groups.add(systemProvider.getEveryoneGroup());
            return groups;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find nested parent groups of principal [%s@%s] in tenant [%s]",
                            principalId != null ? principalId.getName() : "null",
                                    principalId != null ? principalId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
    }

    private String getRegisteredExternalIDPUserObjectId(String tenantName, PrincipalId principalId)
        throws Exception
    {
        String objectId = null;
        PersonUser user = findRegisteredExternalIDPUser(tenantName, principalId);
        if (null != user)
        {
            objectId = user.getObjectId();
        }
        else
        {
            throw new InvalidPrincipalException("Principal cannot be found.", principalId.getUPN() );
        }

        return objectId;
    }

    private
    SearchResult
    find(
        String         tenantName,
        SearchCriteria criteria,
        int            limit
        ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(criteria, "Search criteria");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    criteria.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, criteria.getDomain());

            return provider.find(criteria.getSearchString(), criteria.getDomain(), limit);

        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find objects [Criteria : %s] in tenant [%s]",
                            criteria,
                            tenantName));

            throw ex;
        }
    }

    private
    SearchResult
    findByName(
        String         tenantName,
        SearchCriteria criteria,
        int            limit
        ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(criteria, "Search criteria");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            IIdentityProvider provider = tenantInfo.findProviderADAsFallBack(
                    criteria.getDomain());
            ServerUtils.validateNotNullIdp(provider, tenantName, criteria.getDomain());

            return provider.findByName(criteria.getSearchString(), criteria.getDomain(), limit);
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find objects [Criteria : %s] in tenant [%s]",
                            criteria,
                            tenantName));

            throw ex;
        }
    }

    private
    Set<PersonUser>
    findLockedUsers(
            String tenantName,
            String searchString,
            int    limit
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(searchString, "Search string");
            Set<PersonUser> lockedUsers = new HashSet<PersonUser>();

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            Set<PersonUser> lockedUsers_in_sp = provider.findLockedUsers(searchString, limit);

            if (lockedUsers_in_sp != null && lockedUsers_in_sp.size() > 0)
            {
                lockedUsers.addAll(lockedUsers_in_sp);
            }

            Collection<IIdentityProvider> providers = tenantInfo.getProviders();

            if (providers!= null && providers.size() > 0)
            {
                for (IIdentityProvider idp : providers)
                {
                    Set<PersonUser> lockedUsers_in_idp = idp.findLockedUsers(searchString, limit);

                    if (lockedUsers_in_idp != null && lockedUsers_in_idp.size() > 0)
                    {
                        lockedUsers.addAll(lockedUsers_in_idp);
                    }
                }
            }

            return lockedUsers;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find locked users [Criteria : %s] in tenant [%s]",
                            searchString,
                            tenantName));

            throw ex;
        }
    }

    private
    Set<PersonUser>
    findDisabledPersonUsers(
            String tenantName,
            String searchString,
            int    limit
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(searchString, "Search string");
            Set<PersonUser> disabledUsers = new HashSet<PersonUser>();

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            Set<PersonUser> disabledUsers_in_sp = provider.findDisabledUsers(searchString, limit);

            if (disabledUsers_in_sp != null && disabledUsers_in_sp.size() > 0)
            {
                disabledUsers.addAll(disabledUsers_in_sp);
            }

            Collection<IIdentityProvider> providers = tenantInfo.getProviders();

            if (providers!= null && providers.size() > 0)
            {
                for (IIdentityProvider idp : providers)
                {
                    Set<PersonUser> disabledUsers_in_idp = idp.findDisabledUsers(searchString, limit);

                    if (disabledUsers_in_idp != null && disabledUsers_in_idp.size() > 0)
                    {
                        disabledUsers.addAll(disabledUsers_in_idp);
                    }
                }
            }

            return disabledUsers;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find disabled users [Criteria : %s] in tenant [%s]",
                            searchString,
                            tenantName));

            throw ex;
        }
    }

    private
    Set<SolutionUser>
    findDisabledSolutionUsers(
            String tenantName,
            String searchString
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(searchString, "Search string");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                return provider.findDisabledServicePrincipals(searchString);
            }
            else
            {
                return provider.findDisabledServicePrincipalsInExternalTenant(searchString);
            }
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find disabled solution users [search string : %s] in tenant [%s]",
                            searchString,
                            tenantName));

            throw ex;
        }
            }

    /**
     * Adds a regular user to the tenant's system domain
     *
     * @param tenantName Name of tenant. required, non-null, non-empty
     * @param userName   Name of regular user. required, non-null,
     * @param detail     Detailed information about the user. required, non-null,
     * @param hashedPassword   User's hashed password. required, non-null,
     * @param hashingAlgorithm  The algorithm used to generate the password hash, this is mandatory,
     *        The valid values are defined in enum 'HashingAlgorithmType', currently vmware-directory
     *        only supports 'SSO-v1-1'. required, non-null,
     * @return Principal id of the regular user after it has been created.
     * @throws IDMException
     * @throws RemoteException
     * @throws NoSuchTenantException    - when tenant does not exist
     * @throws InvalidArguementException  - invalid input
     * @throws InValidPrincipleException    - user already exist, or username is empty,
     *                                        or invalid username format
     */
    private
    PrincipalId
    addUser(
            String       tenantName,
            String       userName,
            PersonDetail detail,
            byte[]       hasedPassword,
            String       hashingAlgorithm
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");

            ValidateUtil.validateNotNull(userName, "user name");
            checkInvalidCharForUserData(userName, INVALID_CHARS_FOR_USER_ID);

            ValidateUtil.validateNotNull(detail, "user detail");
            checkInvalidCharForUserData(detail.getFirstName(), INVALID_CHARS_FOR_USER_DETAIL);
            checkInvalidCharForUserData(detail.getLastName(), INVALID_CHARS_FOR_USER_DETAIL);
            checkInvalidCharForUserData(detail.getDescription(), INVALID_CHARS_FOR_USER_DETAIL);

            ValidateUtil.validateNotNull(hasedPassword, "Hashed Password");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.addUser(userName, detail, hasedPassword, hashingAlgorithm);
        }
        catch (ConstraintViolationLdapException e)
        {
            logger.warn(String.format(
                    "provided password for user [%s] violates password policy constraint for tenant [%s]",
                    userName, tenantName), e);
            throw new PasswordPolicyViolationException(e.getMessage(), e);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to add user [%s] in tenant [%s]",
                            userName,
                            tenantName));

            throw ex;
        }
            }

    /**
     * Adds a regular/jit user to the tenant's system domain.
     * If extIdpEntityId is non-null, this user is a jit user.
     * extUserId must be provided for jit user provisioning.
     *
     * @param tenantName Name of tenant. required, non-null, non-empty
     * @param userName   Name of regular/jit user. required, non-null.
     * @param detail     Detailed information about the user. required, non-null.
     * @param extIdpEntityId ExternalIDP entity ID. If it is non-null, add jit user.
     * @param extUserId      External User's ID. Required attribute for jit user.
     * @param password   User's password
     * @return Principal id of the regular user after it has been created.
     * @throws IDMException
     * @throws RemoteException
     * @throws NoSuchTenantException    - when tenant does not exist
     * @throws InvalidArguementException  - invalid input
     * @throws InValidPrincipleException    - user already exist, or invalid user name format
     */
    private
    PrincipalId
    addUser(
            String       tenantName,
            String       userName,
            PersonDetail detail,
            String       extIdpEntityId,
            String       extUserId,
            char[]       password
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");

            ValidateUtil.validateNotNull(userName, "user name");
            checkInvalidCharForUserData(userName, INVALID_CHARS_FOR_USER_ID);

            ValidateUtil.validateNotNull(detail, "user detail");
            checkInvalidCharForUserData(detail.getFirstName(), INVALID_CHARS_FOR_USER_DETAIL);
            checkInvalidCharForUserData(detail.getLastName(), INVALID_CHARS_FOR_USER_DETAIL);
            checkInvalidCharForUserData(detail.getDescription(), INVALID_CHARS_FOR_USER_DETAIL);

            ValidateUtil.validateNotNull(detail, "user detail");

            if (!ValidateUtil.isEmpty(extIdpEntityId)) {
                ValidateUtil.validateNotEmpty(extUserId, "external user id.");
            }

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.addUser(userName, detail, password, extIdpEntityId, extUserId);
        }
        catch (ConstraintViolationLdapException e)
        {
            logger.warn(String.format(
                    "provided password for user [%s] violates password policy constraint for tenant [%s]",
                    userName, tenantName), e);
            throw new PasswordPolicyViolationException(e.getMessage(), e);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to add user [%s] in tenant [%s]",
                            userName,
                            tenantName));

            throw ex;
        }
            }

    private
    boolean
    enableUserAccount(
            String      tenantName,
            PrincipalId userId
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(userId, "user id");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.enableUserAccount(userId);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to enable user [%s@%s] in tenant [%s]",
                            userId != null ? userId.getName() : "null",
                                    userId != null ? userId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
            }

    private
    boolean
    disableUserAccount(
            String      tenantName,
            PrincipalId userId
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(userId, "user id");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.disableUserAccount(userId);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to disable user [%s@%s] in tenant [%s]",
                            userId != null ? userId.getName() : "null",
                                    userId != null ? userId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
            }

    private
    boolean
    unlockUserAccount(
            String      tenantName,
            PrincipalId userId
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(userId, "user id");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.unlockUserAccount(userId);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to unlock user [%s@%s] in tenant [%s]",
                            userId != null ? userId.getName() : "null",
                                    userId != null ? userId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
            }

    /**
     * Adds a security group to the tenant's system domain
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param groupName   Name of security group, required non-null, non-empty
     * @param groupDetail Detailed information about the group. required, non-null.
     * @return Principal id of the group after it has been created.
     * @throws IDMException
     * @throws Exception
     * @throws InvalidArgumentException    - illegal input valid was passed.
     * @throws NoSuchTenantException
     * @throws NoSuchIdpException        when system provider is missing
     */
    private
    PrincipalId
    addGroup(
            String      tenantName,
            String      groupName,
            GroupDetail detail
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotEmpty(groupName, "group name");
            ValidateUtil.validateNotNull(detail, "group detail");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.addGroup(groupName, detail);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to add group [%s] in tenant [%s]",
                            groupName,
                            tenantName));

            throw ex;
        }
            }

    private
    boolean
    lookupPrincipalIdServicePrincipal(
            String    tenantName,
            ISystemDomainIdentityProvider provider,
            PrincipalId principalId
            ) throws Exception
            {
        boolean bIsSystemTenant = ServerUtils.isEquals(tenantName, this.getSystemTenant());

        if (
             (!principalId.getDomain().equalsIgnoreCase(provider.getDomain()))
             &&
             (!principalId.getDomain().equalsIgnoreCase(provider.getAlias()))
           )
        {//the subsequent processing is only relying on the name of the principalId.
            //check here first and return if domain mismatch.
            return true;  //not found
        }
        return ((bIsSystemTenant && provider.findServicePrincipal(principalId.getName()) == null) ||
                (!bIsSystemTenant && provider.findServicePrincipalInExternalTenant(principalId.getName()) == null));
            }

    /**
     * Adds a regular user to a security group in the tenant
     *
     * @param tenantName Name of tenant,  required non-null, non-empty
     * @param userId     Name of regular user to be assigned group
     *                     membership.  required non-null.
     * @param groupName  Name of security group.  required non-null, non-empty
     * @return true if the user's group membership was successfully assigned,
     *         false otherwise.
     * @throws IDMException
     * @throws NoSuchTenantException    - when tenant does not exist
     * @throws InvalidArguementException  - invalid input
     * @throws InValidPrincipleException  - unable to find the group
     * @throws MemberAlreadyExistException    - user already exist
     *
     */
    private
    boolean
    addUserToGroup(
            String      tenantName,
            PrincipalId userId,
            String      groupName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(userId, "userId");
            ValidateUtil.validateNotNull(groupName, "groupName");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (provider.findUser(userId) == null &&
                    lookupPrincipalIdServicePrincipal(tenantName, provider, userId))
            {
                PersonUser user = findPersonUser(tenantName, userId);

                if (user != null)
                {
                    validateObjectIdNotNull(user);
                    PrincipalId newUserId = getFspIdForSystemDomain(provider, user);
                    return provider.addUserToGroup(newUserId, groupName);
                }
                else
                {
                    user = findRegisteredExternalIDPUser(tenantName, userId);
                    if (user != null)
                    {
                        PrincipalId newFspId =  getFspIdForSystemDomain(provider, user);
                        return provider.addUserToGroup(newFspId, groupName);
                    }
                }
            }

            return provider.addUserToGroup(userId, groupName);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to add user [%s@%s] to group [%s] in tenant [%s]",
                            userId != null ? userId.getName() : "null",
                                    userId != null ? userId.getDomain() : "null",
                                            groupName,
                                            tenantName));

            throw ex;
        }
            }

    private
    boolean
    removeFromLocalGroup(
            String      tenantName,
            PrincipalId principalId,
            String      groupName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(principalId, "Principal ID");
            ValidateUtil.validateNotEmpty(groupName, "Group name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (provider.findUser(principalId) == null &&
                    provider.findGroup(principalId) == null &&
                    lookupPrincipalIdServicePrincipal(tenantName, provider, principalId))
            {
                // lookup principal Id as user
                try
                {
                    PersonUser user = findPersonUser(tenantName, principalId);

                    if (user != null)
                    {
                        if (user.getObjectId() != null)
                        {
                            PrincipalId newUserId = getFspIdForSystemDomain(
                                    provider,user);

                            return provider.removeFromGroup(newUserId, groupName);
                        }
                    }
                    else
                    {
                        user = findRegisteredExternalIDPUser(tenantName, principalId);
                        if (user != null)
                        {
                            PrincipalId newFspId = getFspIdForSystemDomain(provider, user);
                            return provider.removeFromGroup(newFspId, groupName);
                        }
                    }
                }
                catch(Exception ex)
                {
                    logger.warn(
                            String.format(
                                    "Failed to find principal [%s@%s] as user in tenant [%s]",
                                    principalId != null ? principalId.getName() : "null",
                                            principalId != null ? principalId.getDomain() : "null",
                                                    tenantName));
                }

                // lookup principal Id as group
                try
                {
                    Group group = findGroup(tenantName, principalId);
                    if (group != null && group.getObjectId() != null)
                    {
                        PrincipalId newGroupId = getFspIdForSystemDomain(provider, group);
                        return provider.removeFromGroup(newGroupId, groupName);
                    }
                    //NB: We don't need to support externalIDP group,
                    //So no registration, no removal of register group.
                }
                catch(Exception ex)
                {
                    logger.warn(
                            String.format(
                                    "Failed to find principal [%s@%s] as group in tenant [%s]",
                                    principalId != null ? principalId.getName() : "null",
                                            principalId != null ? principalId.getDomain() : "null",
                                                    tenantName));
                }
            }

            return provider.removeFromGroup(principalId, groupName);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to remove principal [%s@%s] from local group [%s] in tenant [%s]",
                            principalId != null ? principalId.getName() : "null",
                                    principalId != null ? principalId.getDomain() : "null",
                                            groupName,
                                            tenantName));

            throw ex;
        }
            }

    /**
     * @param principal
     * @throws IllegalArgumentException
     *             when the objectId is null
     */
    private void validateObjectIdNotNull(Principal principal)
    {
        ValidateUtil.validateNotNull(principal, "principal");
        if (principal.getObjectId() == null)
        {
            //Throw exception if objectId is not set
            //when such attribute is optional in the IDS's schema
            String msg =
                    String.format("Not allowed: %s's objectId is null",
                            principal.getId().getUPN());
            logger.error(msg);
            throw new IllegalArgumentException(msg);
        }
    }

    /**
     * Adds a security group to another in the tenant's system domain.
     *     Applies to system provider only. For external provider, we have read privilege only.
     *  If the group is already exist, nothing is changed.
     *
     * @param tenantName Name of tenant. required.
     * @param groupId    Name of security group to be added. required, non-null
     * @param groupName  Name of destination security group to add to. required, non-null, non-empty.
     * @return true if the membership has been successfully changed,
     *         false otherwise
     * @throws IDMException
     * @throws InvalidArgumentException    illegal input
     * @throws NoSuchTenantException        tenant does not exist
     * @throws NoSuchIdpException            System tenant is missing
     * @throws InValidPrincipleException    groupId is invalid
     */
    private
    boolean
    addGroupToGroup(
            String      tenantName,
            PrincipalId groupId,
            String      groupName
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(groupId, "Group ID");
            ValidateUtil.validateNotEmpty(groupName, "Group name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (provider.findGroup(groupId) == null)
            {
                Group group = findGroup(tenantName, groupId);

                if (group != null)
                {
                    validateObjectIdNotNull(group);
                    PrincipalId newGroupId = getFspIdForSystemDomain(provider, group);
                    return provider.addGroupToGroup(newGroupId, groupName);
                }
            }

            return provider.addGroupToGroup(groupId, groupName);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to add group [%s@%s] to group [%s] in tenant [%s]",
                            groupId != null ? groupId.getName() : "null",
                                    groupId != null ? groupId.getDomain() : "null",
                                            groupName,
                                            tenantName));

            throw ex;
        }
            }

    private
    void
    deletePrincipal(
            String tenantName,
            String principalAccountName
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotEmpty(principalAccountName, "principal name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            provider.deletePrincipal(principalAccountName);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to delete principalName [%s] in tenant [%s]",
                            principalAccountName,
                            tenantName));

            throw ex;
        }
    }

    /**
     * Updates the details of a regular user in the system domain of the tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param userName   Name of the regular user
     * @param detail     Details to be updated
     * @return Principal id of the regular user
     * @throws InvalidPrincipalException    - empty username or user does not exist
     * @throws NoSuchTenantException
     * @throws IDMException all other unexpected errors
     */
    private
    PrincipalId
    updatePersonUserDetail(
            String       tenantName,
            String       accountName,
            PersonDetail detail
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ServerUtils.validateNotEmptyUsername(accountName);
            ValidateUtil.validateNotNull(detail, "user detail");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.updatePersonUserDetail(accountName, detail);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to update user [%s] in tenant [%s]", accountName,
                    tenantName));

            throw ex;
        }
    }

    /**
     * Updates a service principal account in the tenant
     *
     * The service principal will be located in the system domain of the tenant
     *
     * @param tenantName Name of tenant
     * @param userName   Name of service principal
     * @param detail     Details of the service principal to be updated.
     * @return Principal Id of the service principal account
     * @throws InvalidPrincipalException    - empty username or user does not exist
     * @throws NoSuchTenantException
     * @throws IDMException all other unexpected errors
     */
    private
    PrincipalId
    updateSolutionUserDetail(
            String         tenantName,
            String         accountName,
            SolutionDetail detail
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ServerUtils.validateNotEmptyUsername(accountName);
            ValidateUtil.validateNotNull(detail, "solution user detail");
            // Validate solution user detail: make sure its certificate is valid
            ValidateUtil.validateSolutionDetail(detail, "Solution user detail",
                    this.getClockTolerance(tenantName));

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
            {
                return provider.updateServicePrincipalDetail(accountName, detail);
            }
            else
            {
                return provider.updateServicePrincipalDetailInExternalTenant(accountName, detail);
            }
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to update solution user [%s] in tenant [%s]", accountName,
                    tenantName));

            throw ex;
        }
    }

    private
    PrincipalId
    updateGroupDetail(
            String      tenantName,
            String      groupName,
            GroupDetail detail
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotEmpty(groupName, "group name");
            ValidateUtil.validateNotNull(detail, "group detail");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.updateGroupDetail(groupName, detail);
        }
        catch (Exception ex)
        {
            logger.error(String.format(
                    "Failed to update group [%s] in tenant [%s]", groupName,
                    tenantName));

            throw ex;
        }
            }

    /**
     * Sets the password
     *
     * The new password is not subject to password policy requirements
     *
     * This routine must be invoked in the (login) context of a user with
     * administrative privileges in the domain of the Identity Provider
     * that governs the principal.
     *
     * @param tenantName  Name of tenant, required.non-null non-empty

     * @param userName    Principal for whom the password must be set. required.non-null non-empty
     * @param newPassword New password. required.non-null non-empty
     * @throws PasswordPolicyViolationException    Illegal password, such as empty pw, was used
     * @throws InvalidPrincipalException  Empty user name or user can not be found
     * @throws NoSuchTenantException
     * @throws IDMException
     * @throws Exception
     */
    private
    void
    setUserPassword(
            String tenantName,
            String accountName,
            char[] newPassword
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ServerUtils.validateNotEmptyUsername(accountName);
            ValidateUtil.validateNotNull(newPassword, "New password");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            provider.resetUserPassword(accountName, newPassword);

            updateSystemDomainStorePasswordIfNeeded(tenantName, accountName, newPassword);

            logger.info(String.format(
                    "Password for user [%s] successfully set for tenant [%s]",
                    accountName, tenantName));
        }
        catch (ConstraintViolationLdapException e)
        {
            logger.warn(String.format(
                    "provided password for user [%s] violates password policy constraint for tenant [%s]",
                    accountName, tenantName), e);
            throw new PasswordPolicyViolationException(e.getMessage(), e);
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to reset password for user [%s] in tenant [%s]",
                            accountName,
                            tenantName));

            throw ex;
        }
    }


    /**
     * Changes the user's password after verifying the current password
     *
     * The new password is subject to password policy requirements
     *
     * @param tenantName      Name of tenant
     * @param userName        Principal for which password must be changed
     * @param currentPassword Current password
     * @param newPassword     New password
     * @throws InvalidPrincipalException  Empty user name or user can not be found
     * @throws NoSuchTenantException
     * @throws IDMException
     * @throws Exception
     */
    private
    void
    changeUserPassword(
            String tenantName,
            String accountName,
            char[] currentPassword,
            char[] newPassword
            ) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ServerUtils.validateNotEmptyUsername(accountName);
            ValidateUtil.validateNotNull(currentPassword, "Current password");
            ValidateUtil.validateNotNull(newPassword, "New password");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            provider.resetUserPassword(accountName, currentPassword, newPassword);

            updateSystemDomainStorePasswordIfNeeded(tenantName, accountName, newPassword);
        }
        catch (ConstraintViolationLdapException e)
        {
            logger.warn(String.format(
                    "provided password for user [%s] violates password policy constraint for tenant [%s]",
                    accountName, tenantName), e);
            throw new PasswordPolicyViolationException(e.getMessage(), e);
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to reset password for user [%s] in tenant [%s]",
                            accountName,
                            tenantName));

            throw ex;
        }
    }

    private
    void
    updateSystemDomainStorePasswordIfNeeded(String tenantName, String accountName, char[] newPassword)
        throws Exception
    {
        IdmServerConfig settings = IdmServerConfig.getInstance();

        ServerIdentityStoreData systemStoreData =
               this.getSystemDomainIdentityStoreData(tenantName);

        String adminAccountName = settings.getTenantAdminUserName(tenantName, accountName);

        if (adminAccountName.equalsIgnoreCase(systemStoreData.getUserName()))
        {
            systemStoreData.setPassword( new String(newPassword) );
            _configStore.setProvider(tenantName, systemStoreData);
            _tenantCache.deleteTenant(tenantName);
        }
    }

    private
    String getDefaultTenant() throws Exception
    {
        String defaultTenant = _tenantCache.findDefaultTenant();

        if (defaultTenant == null || defaultTenant.isEmpty())
        {
            try
            {
                defaultTenant = _tenantCache.setDefaultTenant(_configStore.getDefaultTenant());
            }
            catch(Exception ex)
            {
                logger.error("Failed to get default tenant");

                throw ex;
            }
        }

        return defaultTenant;
    }

    private
    String getSystemTenant() throws Exception
    {
        String systemTenant = _tenantCache.findSystemTenant();

        if (systemTenant == null || systemTenant.isEmpty())
        {
            try
            {
                systemTenant = _tenantCache.setSystemTenant(_configStore.getSystemTenant());
            }
            catch(Exception ex)
            {
                logger.error("Failed to get system tenant");

                throw ex;
            }
        }

        return systemTenant;
    }

    private
    Collection<String> getAllTenants() throws Exception
    {
        Collection<String> allTenants = null;
        try
        {
            allTenants = _tenantCache.findAllTenants();
            // if do not find tenant list in the cache, hit the directory backend
            if (allTenants == null || allTenants.isEmpty())
            {
                allTenants = _configStore.getAllTenants();
                if (allTenants == null || allTenants.isEmpty())
                {
                    throw new IDMException("No tenants are found.");
                }
            }
        }
        catch(Exception ex)
        {
            logger.error("Failed to get a list of all tenants.");

            throw ex;
        }

        return allTenants;
    }

    private
    void
    setEntityID(
            String tenantName,
            String entityID
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(entityID, "Entity ID");

            _configStore.setEntityID(tenantName, entityID);

            _tenantCache.deleteTenant(tenantName);

            logger.info(String.format(
                    "EntityID [%s] successfully set for tenant [%s]",
                    entityID, tenantName));
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to set entity Id [%s] in tenant [%s]",
                            entityID,
                            tenantName));

            throw ex;
        }
            }

    private
    String getEntityID(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            String serviceUri = tenantInfo.getEntityId();
            return expandEntityID(serviceUri);
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get entity Id for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    private
    String getLocalIDPAlias(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

           return tenantInfo.getEntityAlias();
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get local idp alias for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    private
    void setLocalIDPAlias(String tenantName, String alias) throws Exception
    {
        ValidateUtil.validateNotEmpty(tenantName, "tenantName");
        ValidateUtil.validateNotEmpty(alias, "alias");

        try
        {
            _configStore.setAlias(tenantName, alias);
            logger.info(String.format("Alias successfully set for tenant [%s]", tenantName));

            _tenantCache.deleteTenant(tenantName);
            logger.debug(String.format("tenant cache removed for tenant [%s]", tenantName));
        }
        catch (Exception ex)
        {
            logger.error(String.format("Failed to set local idp alias for tenant [%s]", tenantName));
            throw ex;
        }
    }

    private
    String expandEntityID(String serviceUri) throws UnknownHostException, SocketException, IOException
    {
        if (null != serviceUri) {
            String hostIPAddress = CommonUtil.getHostIPAddress(IdmUtils.getIdentityServicesConfigDir());
            String stsTomcatPort = IdmUtils.getStsTomcatPort();
            serviceUri = serviceUri.replace(HOSTNAME_MACRO, hostIPAddress);
            if (stsTomcatPort.equals(DEFAULT_HTTPS_PORT)) {
                //not emitting port if it is default https port. Spring framework work portmatching seem removes it
                // if the port is default causing SAML request validation failure. -- Note from Schai 09/13/2013
                serviceUri = serviceUri.replace(String.format(":%s", PORT_MACRO), "");
            }
            else {
                serviceUri = serviceUri.replace(PORT_MACRO, stsTomcatPort);
            }
        }

        return serviceUri;
    }

    private
    String getOIDCEntityID(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            // reuse the existing entity id to create OIDC entity id.
            // if the entity id has websso suffix, replace it with oidc suffix,
            // otherwise, create a new OIDC entity id.
            // in the long term, we want to change the entity id to be service neutral.
            // see bug# 1410188
            String serviceUri = getEntityID(tenantName);
            String webssoSuffix = String.format("/websso/SAML2/Metadata/%s", tenantName);
            String oidcSuffix = String.format("/openidconnect/%s", tenantName);
            if (serviceUri != null && serviceUri.endsWith(webssoSuffix)) {
                serviceUri = serviceUri.replace(webssoSuffix, oidcSuffix);
            } else {
                serviceUri = String.format(
                        "https://%s:%s/openidconnect/%s",
                        HOSTNAME_MACRO,
                        PORT_MACRO,
                        tenantName);
                serviceUri = expandEntityID(serviceUri);
            }

            return serviceUri;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get OIDC Entity ID for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    private
    void setDefaultTenant(String name) throws Exception
    {
        try
        {
            ValidateUtil.validateNotNull(name, "Tenant name");

            _configStore.setDefaultTenant(name);

            _tenantCache.setDefaultTenant(name);

            logger.info(String.format(
                    "Default tenant [%s] successfully set",
                    name));
        }
        catch(Exception ex)
        {
            logger.error(String.format(
                    "Failed to set default tenant [%s]",
                    name));

            throw ex;
        }
    }

    /**
     * Retrieves the password policy for the specified tenant.
     *
     * @param tenantName Name of tenant, required non-null, non-empty
     * @return policy     pw Policy for the tenant.
     * @throws IDMException
     * @throws NoSuchTenantException        - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     */
    private
    PasswordPolicy
    getPasswordPolicy(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.getPasswordPolicy();
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get password policy for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    /**
     * Sets the password policy for the specified tenant.
     *
     * @param tenantName Name of tenant, required non-null, non-empty
     * @param policy     password Policy for the tenant, required non-null
     * @throws IDMException
     * @throws NoSuchTenantException        - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     */
    private
    void
    setPasswordPolicy(
            String         tenantName,
            PasswordPolicy policy
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(policy, "Password policy");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            provider.setPasswordPolicy(policy);

            logger.info(String.format(
                    "Password policy successfully set for tenant [%s]",
                    tenantName));
        }
        catch (ConstraintViolationLdapException e)
        {
            logger.warn(String
                    .format("Invalid password policy for tenant [%s]",
                            tenantName), e);
            throw new InvalidPasswordPolicyException(e.getMessage(), e);
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to set password policy for tenant [%s]",
                            tenantName));

            throw ex;
        }
            }

    private
    LockoutPolicy
    getLockoutPolicy(String tenantName) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.getLockoutPolicy();
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to get lockout policy for tenant [%s]",
                            tenantName));

            throw ex;
        }
    }

    /**
     * Sets the lockout policy for the specified tenant.
     *
     * @param tenantName Name of tenant, required non-null, non-empty
     * @param policy     Lockout Policy for the tenant, required non-null
     * @throws IDMException
     * @throws NoSuchTenantException        - tenant does not exist
     * @throws InvalidArguementException  - invalid input
     */
    private
    void
    setLockoutPolicy(
            String        tenantName,
            LockoutPolicy policy
            ) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(policy, "Lockout policy");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            provider.setLockoutPolicy(policy);

            logger.info(String.format(
                    "Lockout policy successfully set for tenant [%s]",
                    tenantName));
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to set lockout policy for tenant [%s]",
                            tenantName));

            throw ex;
        }
            }

    private
    Group findGroup(String tenantName, String group)
            throws Exception
            {
        Group foundGroup = null;
        PrincipalId normalizedPrincipal = null;
        IIdentityProvider provider = null;

        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name should not be null/empty.");
            ValidateUtil.validateNotEmpty(group, "Group should not be null/empty.");

            TenantInformation tenantInfo = findTenant(tenantName);

            if (tenantInfo == null)
            {
                throw new NoSuchTenantException(
                        String.format("Tenant [%s] does not exist.", tenantName)
                        );
            }

            normalizedPrincipal = ServerUtils.getUserPrincipal(tenantInfo, group);
            if( normalizedPrincipal == null )
            {
                throw new InvalidPrincipalException(
                        String.format( "Invalid group [%s].", group ),
                        group  //used as invalid principal
                        );
            }

            provider = tenantInfo.findProviderADAsFallBack(normalizedPrincipal.getDomain());

            if (provider == null)
            {
                throw new NoSuchIdpException(
                        String.format( "Unknown domain [%s].", normalizedPrincipal.getDomain() )
                        );
            }

            foundGroup = provider.findGroup(normalizedPrincipal);
            // Provider fulfills the contract return null in case group does not exist
            if (foundGroup == null)
            {
                logger.info(
                        String.format(
                                "Failed to find group [%s@%s] in tenant [%s]",
                                normalizedPrincipal.getName(),
                                normalizedPrincipal.getDomain(),
                                tenantName));

                throw new InvalidPrincipalException(
                        String.format("Group [%s] could not be found for tenant [%s]", group, tenantName),
                        ServerUtils.getUpn(normalizedPrincipal)
                        );
            }

            return foundGroup;
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find group [%s] for tenant [%s]",
                            group,
                            tenantName));

            throw ex;
        }
            }

    private
    Principal findUser(String tenantName, String user)
            throws Exception
            {
        Principal foundUser = null;
        PrincipalId normalizedPrincipal = null;
        IIdentityProvider provider = null;

        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name should not be null/empty.");
            ValidateUtil.validateNotEmpty(user, "User should not be null/empty.");

            TenantInformation tenantInfo = findTenant(tenantName);

            if (tenantInfo == null)
            {
                throw new NoSuchTenantException(
                        String.format("Tenant [%s] does not exist.", tenantName)
                        );
            }

            normalizedPrincipal = ServerUtils.getUserPrincipal(tenantInfo, user);
            if( normalizedPrincipal == null )
            {
                throw new InvalidPrincipalException( String.format( "Invalid user [%s].", user ), user);
            }

            provider = tenantInfo.findProviderADAsFallBack(normalizedPrincipal.getDomain());

            if (provider == null)
            {
                throw new NoSuchIdpException(
                        String.format( "Unknown domain [%s].", normalizedPrincipal.getDomain() )
                        );
            }

            foundUser = provider.findUser(normalizedPrincipal);

            if ( (foundUser == null) && (provider instanceof ISystemDomainIdentityProvider) )
            {
                ISystemDomainIdentityProvider systemProvider = (ISystemDomainIdentityProvider)provider;
                if (ServerUtils.isEquals(tenantName, this.getSystemTenant()))
                {
                    foundUser = systemProvider.findServicePrincipal(normalizedPrincipal.getName());
                }
                else
                {
                    foundUser = systemProvider.findServicePrincipalInExternalTenant(normalizedPrincipal.getName());
                }
            }

            if( foundUser == null )
            {
                throw new InvalidPrincipalException(
                        String.format("User [%s] could not be found for tenant [%s]", user, tenantName),
                        user);
            }

            return foundUser;
        }
        catch (Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find user [%s] for tenant [%s]",
                            user,
                            tenantName));

            throw ex;
        }
    }

    private
    ProvidersInfo loadProvidersInfo(String tenantName) throws Exception
    {
        Collection<IIdentityProvider> providers =
                new ArrayList<IIdentityProvider>();
        Collection<IIdentityStoreData> stores = _configStore.getProviders(
                                                             tenantName,
                                                             EnumSet.of(DomainType.EXTERNAL_DOMAIN,
                                                                        DomainType.SYSTEM_DOMAIN,
                                                                        DomainType.LOCAL_OS_DOMAIN),
                                                                        true);
        Collection<String> defaultProviders = _configStore.getDefaultProviders(tenantName);
        IIdentityProvider adProvider = null;

        Collection<Certificate> trustedCertificates = getAllCertificates(
                tenantName, CertificateType.LDAP_TRUSTED_CERT);
        Set<X509Certificate> trustedCertificatesSet = new HashSet<X509Certificate>();
        if (trustedCertificates != null)
        {
            for (Certificate x509Certificate : trustedCertificates) {
                trustedCertificatesSet.add((X509Certificate)x509Certificate);
            }
        }

        if (stores != null && !stores.isEmpty())
        {
            for (IIdentityStoreData store : stores)
            {
                IIdentityProvider provider = providerFactory.buildProvider(tenantName, store, trustedCertificatesSet);

                if ( ( adProvider == null ) &&
                        ( store.getExtendedIdentityStoreData() != null ) &&
                        (store.getExtendedIdentityStoreData().getProviderType()
                              == IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY))
                {
                    adProvider = provider;
                }
                providers.add(provider);
            }
        }

        return new ProvidersInfo(providers, stores, adProvider, defaultProviders);
    }

    private
    AuthnPolicy getAuthnPolicy(String tenantName, int[] authnTypes) throws Exception{
        boolean password = false;
        boolean windows = false;
        boolean certificate = false;
        boolean rsaSecureID = false;

        if (authnTypes == null) {
            // default values if authnTypes attribute is not set to any value
            password = true;
            windows = true;
        } else {
            for (int authnType : authnTypes) {
                if (authnType == DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_PASSWORD)
                    password = true;
                else if (authnType == DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_WINDOWS)
                    windows = true;
                else if (authnType == DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE)
                    certificate = true;
                else if (authnType == DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID)
                    rsaSecureID = true;
            }
        }

        ClientCertPolicy certPolicy = _configStore.getClientCertPolicy(tenantName);
        if(certPolicy == null){ // set default values
            certPolicy = new ClientCertPolicy();
        }

        RSAAgentConfig rsaConfig = _configStore.getRSAAgentConfig(tenantName);
        return new AuthnPolicy(password, windows, certificate, rsaSecureID, certPolicy, rsaConfig);
    }

    protected
    TenantInformation buildTenantInformation(String tenantName) throws Exception
    {
        TenantInformation tenantInfo = null;

        Tenant tenant = _configStore.getTenant(tenantName);

        if (tenant != null)
        {

            TenantAttributes attrs = _configStore.getTokenPolicyExt(tenantName);
            TokenPolicy tokenPolicyInfo = attrs.getTokenPolicy();
            int delegationCount = tokenPolicyInfo.getMaxTokenDelegationCount();
            int renewCount = tokenPolicyInfo.getMaxTokenRenewCount();
            long clockTolerance = tokenPolicyInfo.getMaxTokenClockTolerance();
            long  maxHOKLifetime = tokenPolicyInfo.getMaxHOKLifetime();
            long maxBearerTokenLifetime = tokenPolicyInfo.getMaxBearerTokenLifetime();
            long maxBearerRefreshTokenLifetime = tokenPolicyInfo.getMaxBearerRefreshTokenLifetime();
            long maxHoKRefreshTokenLifetime = tokenPolicyInfo.getMaxHoKRefreshTokenLifetime();
            String signatureAlgorithm = attrs.getSignatureAlgorithm();
            String brandName = attrs.getBrandName();
            String logonBannerTitle = attrs.getLogonBannerTitle();
            String logonBannerContent = attrs.getLogonBannerContent();
            boolean logonBannerEnableCheckbox = attrs.getLogonBannerCheckboxFlag();
            Collection<Attribute> attrDefinitions = _configStore.getTenantAttributes(tenantName);
            String entityId = attrs.getEntityId();
            String alias = attrs.getAlias();
            int[] authnTypes = attrs.getAuthnTypes();
            AuthnPolicy authnPolicy = getAuthnPolicy(tenantName, authnTypes);
            boolean idpSelectionFlag = attrs.isIDPSelectionEnabled();

            List<Certificate> certificate = _configStore.getTenantCertificate(tenantName);
                    Collection<List<Certificate>> certChains = _configStore.getTenantCertChains(tenantName);
                            PrivateKey key = _configStore.getTenantPrivateKey(tenantName);

                            ProvidersInfo providersInfo = loadProvidersInfo(tenantName);
                            Collection<IDPConfig> idpConfigs = _configStore.getExternalIDPConfigs(tenantName);

                            tenantInfo = new TenantInformation(
                                    tenant,
                                    providersInfo != null ? providersInfo._providers : null,
                                    providersInfo != null ? providersInfo._idsStores : null,
                                    providersInfo != null ? providersInfo._adProvider : null,
                                    delegationCount,
                                    renewCount,
                                    clockTolerance,
                                    maxHOKLifetime,
                                    maxBearerTokenLifetime,
                                    maxBearerRefreshTokenLifetime,
                                    maxHoKRefreshTokenLifetime,
                                    signatureAlgorithm,
                                    brandName,
                                    logonBannerTitle,
                                    logonBannerContent,
                                    logonBannerEnableCheckbox,
                                    certificate,
                                    certChains,
                                    key,
                                    idpConfigs,
                                    attrDefinitions,
                                    entityId,
                                    alias,
                                    providersInfo != null ? providersInfo._defaultProviders : null,
                                    authnPolicy,
                                    idpSelectionFlag);
        }

        return tenantInfo;
    }

    protected
    Collection<IIdentityStoreData>
    getProviders(
            String              tenantName,
            EnumSet<DomainType> domainTypes,
            boolean             bGetInternalInfo
            ) throws Exception
    {
        TenantInformation tenantInfo = findTenant(tenantName);
        ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

        Collection<IIdentityStoreData> allIdsStores = tenantInfo.getIdsStores();
        ArrayList<IIdentityStoreData> idsStores = new ArrayList<IIdentityStoreData>();

        for (IIdentityStoreData store : allIdsStores)
        {
            if (domainTypes.contains(store.getDomainType()))
            {
                if (store instanceof ServerIdentityStoreData && !bGetInternalInfo)
                {
                    idsStores.add(((ServerIdentityStoreData) store).getExternalIdentityStoreData());
                }
                else
                {
                    idsStores.add(store);
                }
            }
        }

        return idsStores;
    }

    protected
    ILdapConnectionEx
    getSystemDomainConnection() throws Exception
    {
        IdmServerConfig settings = IdmServerConfig.getInstance();

        Collection<URI> uris = settings.getSystemDomainConnectionInfo();

        return ServerUtils.getLdapConnectionByURIs(uris,
                settings.getSystemDomainUserName(),
                settings.getSystemDomainPassword(),
                settings.getSystemDomainAuthenticationType(),
                false);
    }

    private String registerServiceProviderAsTenant() throws Exception
    {
        IdmServerConfig settings = IdmServerConfig.getInstance();

        String spDomain = settings.getDirectoryConfigStoreDomain();

        Tenant spTenant = _configStore.getTenant(spDomain);

        if (spTenant == null)
        {
            spTenant = new Tenant(spDomain, spDomain);

            spTenant._issuerName = String.format("%s-STS", spDomain);

            boolean keepTrying = false;

            do
            {
                keepTrying = false;

                try
                {
                    // Create a tenant with credentials
                    String adminUsername = settings.getDirectoryConfigStoreUserName();
                    String adminPassword = settings.getDirectoryConfigStorePassword();
                    registerTenant(spTenant, adminUsername, adminPassword.toCharArray());
                    // Change default password to administrator's password

                    ServerIdentityStoreData systemStoreData =
                        getSystemDomainIdentityStoreData(spDomain);

                    systemStoreData.setUserName(settings.getDirectoryConfigStoreUserName());
                    systemStoreData.setPassword(
                            settings.getDirectoryConfigStorePassword());
                    systemStoreData.setAuthenticationType(settings.getDirectoryConfigStoreAuthType());

                    _configStore.setProvider(spDomain, systemStoreData);

                    _configStore.setSystemTenant(spDomain);

                    // call VMCA to create certs/private key and set tenant credentials
                    setTenantCredentials(spDomain);

                    // set default tenant settings
                    _configStore.setClockTolerance(spDomain, IConfigStore.DEFAULT_CLOCK_TOLERANCE);
                    _configStore.setDelegationCount(spDomain, IConfigStore.DEFAULT_DELEGATION_COUNT);
                    _configStore.setRenewCount(spDomain, IConfigStore.DEFAULT_RENEW_COUNT);
                    _configStore.setMaximumBearerTokenLifetime(spDomain, IConfigStore.DEFAULT_MAX_BEARER_LIFETIME);
                    _configStore.setMaximumHoKTokenLifetime(spDomain, IConfigStore.DEFAULT_MAX_HOK_LIFETIME);
                    _configStore.setMaximumBearerRefreshTokenLifetime(spDomain, IConfigStore.DEFAULT_MAX_BEARER_REFRESH_TOKEN_LIFETIME);
                    _configStore.setMaximumHoKRefreshTokenLifetime(spDomain, IConfigStore.DEFAULT_MAX_HOK_REFRESH_TOKEN_LIFETIME);

                    _configStore.updatePasswordExpirationConfiguration(spDomain, PasswordExpiration.createDefaultSettings());

                    // set brand name to Lightwave if needed
                    setTenantBrandName(spDomain);

                    _tenantCache.setSystemTenant(spDomain);
                }
                catch(ServerDownLdapException ex)
                {
                    logger.error("LDAP Server down, unable to register service provider as tenant. Retrying.", ex);
                    keepTrying = true; // allow system domain back-end to start
                }

            } while (keepTrying);

            if (settings.getIsSingletenantConfig())
            {
                setDefaultTenant(spDomain);
            }

            String[] defaultProviders =  { getComputerName() };
            logger.info(String.format("Determined localos providers's default name as [%s].", defaultProviders[0]));

            ServerIdentityStoreData localOSIdp =
                    new ServerIdentityStoreData(DomainType.LOCAL_OS_DOMAIN, defaultProviders[0]);
            localOSIdp.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LOCAL_OS);
            // for now we don't want to set an alias for local OS provider
            // if ( defaultProviders[0].equalsIgnoreCase(LOCAL_OS_STATIC_ALIAS) == false )
            //{
            //    localOSIdp.setAlias(LOCAL_OS_STATIC_ALIAS);
            //}

            addProvider(spDomain, localOSIdp);

            setDefaultProviders(spDomain, Arrays.asList(defaultProviders));
        }

        return spTenant.getName();
    }

    private void setTenantCredentials(String tenantName) throws Exception {
        // VMCA time constants
        final int VMCA_DEFAULT_KEY_LENGTH = 2048;
        final long VMCA_TIME_SECS_PER_MINUTE = 60;
        final long VMCA_TIME_SECS_PER_YEAR = 365 * 24 * 60 * 60;

        final String CONFIG_IDENTITY_ROOT_KEY ="Software\\VMware\\Identity\\Configuration";
        final String HOST_NAME_KEY = "Hostname";
        final String HOST_NAME_TYPE_KEY = "HostnameType";

        final String CERT_ALIAS = "ssoserverSign";

        String hostname;
        String hostnameType;
        VMCAClient vmcaClient = null;
        X509Certificate leafCert = null;
        X509Certificate rootCert = null;

        IRegistryAdapter registryAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
        IRegistryKey rootRegistryKey = registryAdapter.openRootKey((int) RegKeyAccess.KEY_READ);

        try {
            hostname = registryAdapter.getStringValue(
                    rootRegistryKey,
                    CONFIG_IDENTITY_ROOT_KEY,
                    HOST_NAME_KEY,
                    true);
            hostnameType = registryAdapter.getStringValue(
                    rootRegistryKey,
                    CONFIG_IDENTITY_ROOT_KEY,
                    HOST_NAME_TYPE_KEY,
                    true);
        } finally {
            rootRegistryKey.close();
        }

        Request certRequest = new Request();
        certRequest.setName(CERT_ALIAS);
        certRequest.setKeyusage(7); // 7 is to set key usage to: Digital Signature, Non Repudiation, Key Encipherment
        KeyPair keyPair = certRequest.createKeyPair(VMCA_DEFAULT_KEY_LENGTH);

        Date now = new Date();
        Date startFrom = new Date(now.getTime() - 10 * VMCA_TIME_SECS_PER_MINUTE * 1000);
        Date expire = new Date(now.getTime() + 10 * VMCA_TIME_SECS_PER_YEAR * 1000);

        IdmServerConfig settings = IdmServerConfig.getInstance();
        String upn = settings.getDirectoryConfigStoreUserName();
        int idx = upn.indexOf('@');
        String username = upn.substring(0, idx);
        String domain = upn.substring(idx+1);
        String password = settings.getDirectoryConfigStorePassword();

        if ("fqdn".equals(hostnameType) && !hostname.isEmpty()) {
            certRequest.setDnsname(hostname);
        } else if (("ipv4".equals(hostnameType) || "ipv6".equals(hostnameType)) && !hostname.isEmpty()) {
            certRequest.setIpaddress(hostname);
        }

        vmcaClient = new VMCAClient(username, domain, password, "localhost");
        leafCert = vmcaClient.getCertificate(
                certRequest,
                keyPair,
                startFrom,
                expire);

        rootCert = vmcaClient.getRootCertificate();

        if (rootCert == null || leafCert == null) {
            throw new IllegalStateException("Failed to generate tenant certificates.");
        }

        ArrayList<Certificate> tenantCertificates = new ArrayList<Certificate>();
        tenantCertificates.add(leafCert);
        tenantCertificates.add(rootCert);

        _configStore.setTenantCredentials(tenantName, tenantCertificates, keyPair.getPrivate());
    }


    private void setTenantBrandName(String tenantName) throws Exception {
        final String CONFIG_IDENTITY_ROOT_KEY ="Software\\VMware\\Identity\\Configuration";
        final String IS_LIGHTWAVE_KEY = "isLightwave";

        if(getBrandName(tenantName) == null) {
            logger.info("Brand is not set");
            // Set default branding if its Lightwave.
            Integer isLightwave = 0;
            IRegistryAdapter registryAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
            IRegistryKey rootRegistryKey = registryAdapter.openRootKey((int) RegKeyAccess.KEY_READ);
            try {
            	isLightwave = registryAdapter.getIntValue(
                        rootRegistryKey,
                        CONFIG_IDENTITY_ROOT_KEY,
                        IS_LIGHTWAVE_KEY,
                        true);
            	if(isLightwave != 0 ) {
                logger.info("Configuring branding name for Lightwave instance");
            	_configStore.setBrandName(tenantName, "Photon Platform<br/>Single Sign-On");
            }
            } finally {
                rootRegistryKey.close();
            }
        }
    }

    private ServerIdentityStoreData getSystemDomainIdentityStoreData(String tenantName)
        throws Exception
    {
        Collection<IIdentityStoreData> systemStores = getProviders(
            tenantName,
            EnumSet.of(DomainType.SYSTEM_DOMAIN),
            true);
        if (systemStores.size() != 1)
        {
            throw new IllegalStateException(
                "Error : Unexpected number of system stores found");
        }

        IIdentityStoreData cfgStore =
            systemStores.iterator().next();

        if (!(cfgStore instanceof ServerIdentityStoreData))
        {
            throw new IllegalStateException(
                    "Error : Unexpected system store found");
        }

        ServerIdentityStoreData systemStoreData =
            (ServerIdentityStoreData) cfgStore;

        return systemStoreData;
    }

    private static String getComputerName()
    {
        String computerName = LOCAL_OS_STATIC_ALIAS; // this is "localos"
        if(SystemUtils.IS_OS_WINDOWS)
        {
            IIdmClientLibrary client = IdmClientLibraryFactory.getInstance().getLibrary();
            computerName = client.getComputerName();
        }
        return computerName;
    }


    private void initializeTenantCache() throws Exception
    {
        Collection<String> allTenantNames = this.getAllTenants();
        assert(allTenantNames != null && allTenantNames.size() > 0);

        for (String tenantName : allTenantNames)
        {
            TenantInformation tenantInfo = loadTenant(tenantName);
            assert(tenantInfo != null);
        }
    }

    private void refreshTenantCache() throws Exception
    {
        Collection<String> allTenantNames = this.getAllTenants();
        assert(allTenantNames != null && allTenantNames.size() > 0);

        for (String tenantName : allTenantNames)
        {
            try
            {
                refreshTenant(tenantName);
            }
            catch(Exception ex)
            {
                // continue to refresh other tenant (do not fail refresh)
                logger.warn(
                        String.format(
                                "Failed to refreshTenantCredentialCache for tenant %s",
                                tenantName), ex);
                continue;
            }
        }
    }


    /**
     * Check update for cached CRL of all tenants
     *
     * @throws Exception
     */
    private void refreshTenantCrlCache() throws Exception
    {

        Collection<String> allTenantNames = this.getAllTenants();
        assert(allTenantNames != null && allTenantNames.size() > 0);

        for (String tenantName : allTenantNames)
        {

            //First, download custom CRL if defined
            TenantInformation info = this.getTenantInfo(tenantName);

            AuthnPolicy authnPolicy = info.getAuthnPolicy();
            Validate.notNull(authnPolicy, "AuthnPolicy can not be null.");

            ClientCertPolicy certPolicy = authnPolicy.getClientCertPolicy();
            Validate.notNull(certPolicy, "CertPolicy can not be null.");

            URL crlUrl = certPolicy.getCRLUrl();

            if (crlUrl != null) {
                String crlUriString = crlUrl.toString();
                IdmCrlCache crlCache = TenantCrlCache.get().get(tenantName);

                if (crlCache == null) {
                    crlCache = TenantCrlCache.get().put(tenantName, new IdmCrlCache());
                }
                if (null == crlCache.get(crlUriString) && !crlUriString.isEmpty()) {
                    try {
                        X509CRL crl = IdmCrlCache.downloadCrl(crlUriString);
                        if (null !=  crl ) {
                            crlCache.put(crlUriString, crl);
                        } else {
                            throw new Exception("No CRL was download at "+ crlUriString);
                        }
                    } catch (Exception e) {
                        //don't throw because of communication problem. This allow refreshing at other URI's.
                        logger.error("Failed to download custom CRL at CRL refresh. "+e.getMessage());
                    }
                }
            }
            _tenantCrlCache.refreshCrl(tenantName);
        }
    }

    /**
     * Update rsa_api.properties, sdconf.rec and sdopts.rec if there is change in tenant configuration.
     *
     * @param tenantInfo
     * @throws Exception
     */
    private void updateRSAConfigFiles(TenantInformation tenantInfo) throws Exception {
        Validate.notNull(tenantInfo, "tenantInfo");

        AuthnPolicy authnPolicy = tenantInfo.getAuthnPolicy();
        if (authnPolicy == null || authnPolicy.get_rsaAgentConfig() == null) {
            return;
        }

        RSAAgentConfig rsaConfig = authnPolicy.get_rsaAgentConfig();

        try {
            if (rsaConfig == null) {
                return;
            }

            String tenantName = tenantInfo.getTenant().getName();
            Validate.notEmpty(tenantName, "tenantName");

            //detect changes
            RSAAgentConfig existingConfig = _tenantCache.findExtRsaConfig(tenantName);
            if (null != existingConfig &&
                    existingConfig.equals(rsaConfig)) {
                return;
            }

            //update disk files
            RsaAgentConfFilesUpdater updater = new RsaAgentConfFilesUpdater(this.getClusterId());
            updater.updateRSAConfigFiles(tenantInfo, rsaConfig);

            //update existing config cache.
            _tenantCache.deleteExtRsaConfig(tenantName);
            _tenantCache.addExtRsaConfig(tenantName, rsaConfig);

            //remove cached AuthSessionFactory and cached session for the tenant .
            _rsaSessionCache.removeSessionCache(tenantName);
            _rsaSessionFactoryCache.removeFactory(tenantName);
        } catch (Exception e) {
            logger.error("Failed updating RSA config files", e);
            throw e;
        }
    }

    private void registerTenant(Tenant tenant, String adminAccountName, char[] adminPwd) throws Exception
    {
        _configStore.addTenant(tenant, adminAccountName, adminPwd);

        _configStore.setTenantAttributes(
                tenant.getName(),
                _defaultAttributes);

        _configStore.setEntityID(tenant.getName(),
                String.format("https://%s:%s/websso/SAML2/Metadata/%s",
                        HOSTNAME_MACRO, PORT_MACRO,
                        tenant.getName()));
    }

    private void unregisterTenant(String name) throws Exception
    {
        _configStore.deleteTenant(name);
    }

    private void createTenantDomain(String tenantName) throws Exception
    {
        Collection<IIdentityStoreData> stores =
                getProviders(
                        tenantName,
                        EnumSet.of(DomainType.SYSTEM_DOMAIN),
                        true /* get internal info */);

        if (stores.size() != 1)
        {
            throw new IllegalStateException(
                    "Failed to find tenant's system domain");
        }

        IIdentityStoreDataEx details =
                stores.iterator().next().getExtendedIdentityStoreData();

        String upn = details.getUserName();
        if (upn == null || upn.isEmpty())
        {
            throw new IllegalStateException(
                    "Invalid system domain administrator name");
        }

        PrincipalId id = ServerUtils.getPrincipalId(upn);
        Directory.createInstance(
                tenantName,
                id.getName(),
                details.getPassword());
    }

    private
    TenantInformation findTenant(String tenantName) throws Exception
    {
        TenantInformation tenantInfo = _tenantCache.findTenant(tenantName);
        if (tenantInfo == null)
        {
            tenantInfo = loadTenant(tenantName);
        }

        return tenantInfo;
    }

    private TenantInformation getTenantInfo(String tenantName) throws Exception
    {
        TenantInformation tenantInfo = null;
        try {
            tenantInfo = findTenant(tenantName);
        }
        catch (Exception e)
        {
            logger.error(String.format("Failed to get all external IDP config for tenant [%s]", tenantName));
            throw e;
        }
        return tenantInfo;
    }

    // refresh tenant currently only refreshes tenant certificates and token policy if tenant already exists
    private
    TenantInformation refreshTenant(String tenantName) throws Exception
    {
        TenantInformation tenantInfo = loadTenant(tenantName);

        if (tenantInfo == null) {

            _tenantCache.deleteTenant(tenantName);

            ssoHealthStatistics.removeTenantStats(tenantName);
            LdapConnectionPool.getInstance().cleanPool(tenantName);

        }
        return tenantInfo;
    }

    private TenantInformation loadTenant(String tenantName) throws Exception
    {
        TenantInformation tenantInfo = buildTenantInformation(tenantName);

        if (tenantInfo != null)
        {
            _tenantCache.addTenant(tenantInfo);
            updateRSAConfigFiles(tenantInfo);
            LdapConnectionPool.getInstance().createPool(tenantName);
        }

        return tenantInfo;
    }


   /**
    * check the optional data against the set of invalid chars and throw
    * exception if detected
    *
    * @param data
    *           could be null or empty, in which case will return normally.
    * @param invalidChars
    *           array of invalid chars
    * @throws InvalidPrincipalException if any of illegal chars are found
    */
    private void checkInvalidCharForUserData(String data, final char[] invalidChars)
         throws InvalidArgumentException
    {
       if (null == data) return;
       for (char invalidChar : invalidChars)
        {
            if (-1 != data.indexOf(invalidChar))
            {
                throw new InvalidArgumentException(
                   String.format(
                      "Invalid character [%c] detected! please check PrincipalManagement service API",
                      invalidChar));
            }
        }
   }

    private void ensureWellKnownGroupExists(String tenantName, String wellknownGroupName, String description) throws Exception
    {
        try
        {
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider systemProvider =
                    tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(systemProvider, tenantName);

            Group solutionUsersGroup = systemProvider.findGroup(new PrincipalId(wellknownGroupName, tenantName));
            if (solutionUsersGroup == null)
            {
                PrincipalId group = systemProvider.addGroup(wellknownGroupName,
                        new GroupDetail(description));
                if (group == null)
                {
                    throw new InvalidPrincipalException(String.format("Failed to create solutinUsers group %s for tenant %s",
                            wellknownGroupName,
                            tenantName), new PrincipalId(wellknownGroupName, tenantName).getUPN());
                }
            }
        }
        catch (Exception ex)
        {
            logger.error("Failed to create group '{}' for tenant '{}'", wellknownGroupName, tenantName, ex);
            throw ex;
        }
    }

    private void ensureContainerExists(String tenantName, String containerName) throws Exception {
        try {
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider systemProvider = tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(systemProvider, tenantName);

            systemProvider.addContainer(containerName);
        } catch (ContainerAlreadyExistsException e) {
            logger.debug("Container '{}' already exists for tenant '{}'", containerName, tenantName);
        } catch (Exception e) {
            logger.error("Failed to create container '{}' for tenant '{}'", containerName, tenantName, e);
            throw e;
        }
    }

    /**
     * get external IDP user's object id.
     *
     * we use Upn at the moment.
     *
     * EG: external PersonUser
     * with PrincipalId [_name: joe, _domain: <guid>.vsphere.local] ==>
     * joe@<guid>.vsphere.local
     *
     * @param id PrincipalId of the external user
     * @return external IDP user's object id
     */
    private static String getExternalIdpUserObjectId(PrincipalId id)
    {
        return id.getUPN();
    }

    // TODO: we should look into whether this logic of building fspid can be eliminated or hidden within system provider
    private static PrincipalId getFspIdForSystemDomain(ISystemDomainIdentityProvider systemProvider, Principal fspPrincipal)
    {
        return getFspIdForSystemDomain(systemProvider, fspPrincipal.getObjectId(), fspPrincipal.getId());
    }

    // TODO: we should look into whether this logic of building fspid can be eliminated or hidden within system provider
    private static PrincipalId getFspIdForSystemDomain(ISystemDomainIdentityProvider systemProvider, String objectId, PrincipalId principalId)
    {
        return new PrincipalId(
                systemProvider.getObjectIdName(objectId),
                principalId.getDomain());
    }

    private IIdentityStoreData getADIdsToStore(IIdentityStoreData store)
    {
        IIdentityStoreDataEx extData = store.getExtendedIdentityStoreData();
        if (store == null || extData == null)
        {
            return null;
        }

        // native provider stores using domain forest information as provider name
        // in case forestDcInfo is null, use the generic name
        // to support a replicated multi-sites scenario
        // domainName is used internal and before server returns to client
        ActiveDirectoryJoinInfo joinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
        DomainControllerInfo forestDcInfo = IdmDomainState.getInstance().getForestDcInfo();
        if (joinInfo == null)
        {
            throw new IllegalStateException(
                    "Trying to store native AD information, however machine is not properly joined.");
        }

        ArrayList<String> connStrs = new ArrayList<String>();
        connStrs.add(String.format("ldap://%s",
                                   forestDcInfo != null ? forestDcInfo.domainName : joinInfo.getName()));
        return IdentityStoreData.CreateExternalIdentityStoreData(
                forestDcInfo != null ? forestDcInfo.domainName : joinInfo.getName(),
                forestDcInfo != null ? forestDcInfo.domainNetBiosName : joinInfo.getAlias(),
                extData.getProviderType(),
                extData.getAuthenticationType(),
                forestDcInfo != null ? forestDcInfo.domainNetBiosName : joinInfo.getAlias(),
                extData.getSearchTimeoutSeconds(),
                extData.getUserName(),
                extData.useMachineAccount(),
                extData.getServicePrincipalName(),
                extData.getPassword(),
                null, //userBaseDN
                null, //groupBaseDN
                connStrs, /* connection strings */
                extData.getAttributeMap(),
                extData.getIdentityStoreSchemaMapping(),
                extData.getUpnSuffixes(),
                extData.getFlags(),
                null, extData.getAuthnTypes(),
                extData.getCertUserHintAttributeName(),
                extData.getCertLinkingUseUPN()
        );
    }

    private
    void setExternalIdpForTenant(String tenantName,
            IDPConfig idpConfig) throws Exception
    {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "[tenantname]");
            ValidateUtil.validateNotNull(idpConfig, "[idpConfig]");

            // validate IDP config has all the required meta data before registering..
            ValidateUtil.validateNotEmpty(idpConfig.getSigningCertificateChain(), "[idpConfig.signingCertificates]");

            // validate groups exist in system domain before setting claim group mappings
            Map<TokenClaimAttribute, List<String>> claimGroupMappings = idpConfig.getTokenClaimGroupMappings();
            if (claimGroupMappings != null && !claimGroupMappings.isEmpty()) {
                for (Entry<TokenClaimAttribute, List<String>> map : claimGroupMappings.entrySet()) {
                    if (map.getValue() == null || map.getValue().isEmpty()) {
                        continue;
                    }
                    for (String groupSid : map.getValue()) {
                        try {
                            findGroupByObjectId(tenantName, groupSid);
                        } catch (Exception e) {
                            throw new IDMException(String.format("Failed to set claim group mapping. "
                                    + "Group %s is not found in tenant %s.", groupSid, tenantName), e);
                        }
                    }
                }
            }

            _configStore.registerExternalIdpConfig(tenantName, idpConfig);

            // delete cached data so that it will be reloaded next time
            _tenantCache.deleteTenant(tenantName);
        }
        catch (Exception e)
        {
            logger.error(
                    String.format("Failed to set external IDP for tenant [%s]", tenantName));

            throw e;
        }
    }

    private
    void removeExternalIdpForTenant(String tenantName,
            String configEntityId, boolean removeJitUsers) throws Exception
            {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotEmpty(configEntityId, "configEntityId");

            _configStore.removeExternalIdpConfig(tenantName, configEntityId);

            //delete JIT users
            if (removeJitUsers) {
                TenantInformation tenantInfo = findTenant(tenantName);
                ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

                ISystemDomainIdentityProvider provider =
                        tenantInfo.findSystemProvider();
                ServerUtils.validateNotNullSystemIdp(provider, tenantName);
                provider.deleteJitUsers(configEntityId);
            }

            _tenantCache.deleteTenant(tenantName);
        }
        catch (Exception e)
        {
            logger.error(String.format(
                    "Failed to delete external IDP for tenant [%s]",
                    tenantName));
            throw e;
        }
            }

    private
    Collection<IDPConfig> getAllExternalIdpsForTenant(String tenantName)
            throws Exception
            {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");

            TenantInformation tenantInfo = getTenantInfo(tenantName);
            if (tenantInfo == null)
            {
                throw new NoSuchTenantException(String.format("Tenant [%s] not found", tenantName));
            }
            else
            {
                return tenantInfo.getExternalIdpConfigs();
            }
        }
        catch (Exception e)
        {
            logger.error(String.format(
                    "Failed to get all external IDP for tenant [%s]",
                    tenantName));
            throw e;
        }
            }

    private
    IDPConfig getExternalIdpForTenant(String tenantName,
            String configEntityId) throws Exception
            {
        try {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");

            TenantInformation tenantInfo = getTenantInfo(tenantName);
            if (tenantInfo == null)
            {
                throw new NoSuchTenantException(String.format("Tenant [%s] not found", tenantName));
            }
            else
            {
                for (IDPConfig config : tenantInfo.getExternalIdpConfigs())
                {
                    if (config.getEntityID().equals(configEntityId))
                        return config;
                }
                return null;   // not found
            }
        }
        catch (Exception e)
        {
            logger.error(String.format(
                    "Failed to get external IDP [%s] for tenant [%s]",
                    configEntityId, tenantName));
            throw e;
        }
            }

    private
    Collection<IDPConfig> getExternalIdpForTenantByUrl(
            String tenantName, String urlStr) throws Exception
            {
        try  {
            ValidateUtil.validateNotEmpty(tenantName, "urlStr");
            ValidateUtil.validateNotEmpty(urlStr, "urlStr");
            Collection<IDPConfig> result = new ArrayList<IDPConfig>();

            for (IDPConfig config : getAllExternalIdpsForTenant(tenantName))
            {
                if (config.isMatchingUrl(urlStr))
                {
                    result.add(config);
                }
            }
            return result;
        }
        catch (Exception e)
        {
            logger.error(String.format(
                    "Failed to get external IDP by URL for tenant [%s]",
                    tenantName));
            throw e;
        }
            }

    private
    boolean registerThirdPartyIDPUser(String tenantName, PrincipalId userId)
            throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(userId, "userId");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider systemProvider =
                    tenantInfo.findSystemProvider();
            assert (systemProvider != null);

            //construct the fspUser's DN and use it to register with system provider
            PrincipalId fspId = getFspIdForSystemDomain(systemProvider, getExternalIdpUserObjectId(userId), userId);
            return systemProvider.registerExternalIDPUser(fspId.getUPN());
        } catch (Exception ex)
        {
            logger.error(String
                    .format("Failed to register third party IDP user [%s] in tenant [%s]",
                            userId, tenantName));
            throw ex;
        }
            }

    private
    boolean removeThirdPartyIDPUser(String tenantName, PrincipalId userId)
            throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "tenantName");
            ValidateUtil.validateNotNull(userId, "userId");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider systemProvider =
                    tenantInfo.findSystemProvider();
            assert (systemProvider != null);

            //construct the fsp objectId used to register with system provider
            PrincipalId fspId = getFspIdForSystemDomain(systemProvider, getExternalIdpUserObjectId(userId), userId);
            systemProvider.removeExternalIDPUser(fspId.getUPN());
            return true;
        } catch (Exception ex)
        {
            logger.error(
                    String.format("Failed to remove third party IDP user [%s] in tenant [%s]",
                            userId, tenantName));
            throw ex;
        }
            }

    private
    PersonUser findRegisteredExternalIDPUser(String tenantName,
            PrincipalId userId) throws Exception
            {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotNull(userId, "User principal Id");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider = tenantInfo.findSystemProvider();

            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            PrincipalId pid = provider.findExternalIDPUserRegistration(userId);

            if (pid != null)
            {
                return new PersonUser(pid, null/*alias*/, getExternalIdpUserObjectId(pid)/*objectId*/, EXTERNAL_USER_SAMPLE_PERSON_DETAIL, false, false);
            }
            return null;
        }
        catch(Exception ex)
        {
            logger.error(
                    String.format(
                            "Failed to find registered external IDP user [%s@%s] in tenant [%s]",
                            userId != null ? userId.getName() : "null",
                                    userId != null ? userId.getDomain() : "null",
                                            tenantName));

            throw ex;
        }
            }

    private
    String getExternalIDPRegistrationGroupName()
    {
        return WELLKNOWN_EXTERNALIDP_USERS_GROUP_NAME;
    }

    private
    PrincipalId findActiveUserInSystemDomain(String tenantName, String attributeName,
            String attributeValue) throws Exception
    {
        try
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNotEmpty(attributeName, "attributeName");
            ValidateUtil.validateNotEmpty(attributeValue, "attributeValue");

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider = tenantInfo.findSystemProvider();

            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            PrincipalId pid = provider.findActiveUser(attributeName, attributeValue);

            return pid;
        }
        catch(Exception ex)
        {
            logger.error(
                  String.format(
                            "Failed to find active user by attribute=[%s], attribute_value=[%s], in tenant [%s]",
                            (attributeName != null ? attributeName : "(null)"),
                            (attributeValue != null ? attributeValue : "(null)"),
                            (tenantName != null ? tenantName : "(null)"))
                    );

            throw ex;
        }
    }

    private
    boolean registerUpnSuffix(String tenantName, String domainName,
         String upnSuffix) throws Exception {

      boolean updateStarted = false;
      boolean added = false;
      try {

         ValidateUtil.validateNotEmpty(tenantName, "tenantName");
         ValidateUtil.validateNotEmpty(domainName, "domainName");
         ValidateUtil.validateNotEmpty(upnSuffix, "upnSuffix");

         TenantInformation tenantInfo = findTenant(tenantName);
         ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
         for (IIdentityStoreData store : tenantInfo.getIdsStores())
         {
            if (store.getName().equalsIgnoreCase(domainName))
            {
               IdentityStoreType type = store.getExtendedIdentityStoreData().getProviderType();
               if (type.equals(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP) ||
                   type.equals(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING) ||
                   type.equals(IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY))
               {
                  return false; // for OpenLdap/AdOverLdap/NativeAD providers, registerUpn suffix is not allowed.
               }
               else
               {
                  break;
               }
            }
         }

         updateStarted = true;
         added =_configStore.registerUpnSuffixForDomain(tenantName, domainName, upnSuffix);

      } catch (Exception ex) {

         logger.error(String.format(
                     "Failed to register UPN suffix [%s] in domain [%s] for tenant [%s]",
                     upnSuffix, domainName, tenantName));
         throw ex;
      } finally {
         if (updateStarted)
         {
            _tenantCache.deleteTenant(tenantName);
         }
      }
      return added;
   }

    private
    boolean unregisterUpnSuffix(String tenantName, String domainName,
         String upnSuffix) throws Exception {

      boolean updateStarted = false;
      boolean removed = false;
      try
      {
         ValidateUtil.validateNotEmpty(tenantName, "tenantName");
         ValidateUtil.validateNotEmpty(domainName, "domainName");
         ValidateUtil.validateNotEmpty(upnSuffix, "upnSuffix");

         TenantInformation tenantInfo = findTenant(tenantName);
         ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

         updateStarted = true;
         removed =
               _configStore.unregisterUpnSuffixForDomain(tenantName,
                     domainName, upnSuffix);

      } catch (Exception ex)
      {
         logger.error(String.format(
                     "Failed to unregister UPN suffix [%s] in domain [%s] for tenant [%s]",
                     upnSuffix, domainName, tenantName));
         throw ex;
      } finally
      {
         if (updateStarted)
         {
            _tenantCache.deleteTenant(tenantName);
         }
      }
      return removed;
   }

    private
    Set<String> getUpnSuffixes(String tenantName, String domainName)
         throws Exception {

      Set<String> result = null;
      try
      {
         ValidateUtil.validateNotEmpty(tenantName, "tenantName");
         ValidateUtil.validateNotEmpty(domainName, "domainName");

         TenantInformation tenantInfo = findTenant(tenantName);
         ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

         Collection<IIdentityProvider> providers = tenantInfo.getProviders();

         boolean providerFound = false;
         for (IIdentityProvider provider : providers)
         {
            if (0 != provider.getDomain().compareToIgnoreCase(domainName)) {
               continue;
            }

            providerFound = true;
            if (provider instanceof BaseLdapProvider)
            {
               Set<String> data = provider.getRegisteredUpnSuffixes();
               result = (data != null ? Collections.unmodifiableSet(data) : null);
            } else if (provider instanceof LocalOsIdentityProvider)
            {
               throw new IllegalStateException(
                     "UPN suffixes for LocalOS provider is not supported");
            } else
            {
               throw new IllegalStateException(String.format(
                     "Unsupport IdentityProvider type: [%s]", provider
                           .getClass().getName()));
            }
            break;
         }
         if (!providerFound) {
            String msg =
                  String.format("Provider [%s] is not found in tenant [%s]",
                        domainName, tenantName);
            throw new NoSuchIdpException(msg);
         }
      } catch (Exception e)
      {
         logger.error(String.format(
               "Failed to get UPN suffixes in domain [%s] for tenant [%s]",
               domainName, tenantName));
         throw e;
      }
      return result;
   }

    private String getExternalIDPAlias(String tenantName, String entityId) throws IDMException, Exception
    {
        ValidateUtil.validateNotEmpty(tenantName, "tenantName");
        ValidateUtil.validateNotEmpty(entityId, "entityId");

        try
        {
            IDPConfig idpConfig = this.getExternalIdpForTenant(tenantName, entityId);
            return idpConfig.getAlias();
        } catch (Exception e)
        {
            logger.error(String.format(
                  "Failed to get alias for entityID [%s] for tenant [%s]",
                  entityId, tenantName), e);
            throw e;
        }
    }

    private void setExternalIDPAlias(String tenantName, String entityId, String alias)
            throws IDMException, Exception
    {
        ValidateUtil.validateNotEmpty(tenantName, "tenantName");
        ValidateUtil.validateNotEmpty(entityId, "entityId");

        try
        {
            IDPConfig idpConfig = this.getExternalIdpForTenant(tenantName, entityId);
            idpConfig.setAlias(alias);
            this.setExternalIdpForTenant(tenantName, idpConfig);
        }
        catch (Exception ex)
        {
            logger.error(String.format("Failed to set alias for entityId [%s] for tenant [%s]", entityId, tenantName));
            throw ex;
        }
    }

    private
    ActiveDirectoryJoinInfo
    getActiveDirectoryJoinStatus() throws IDMException, Exception
    {
        try
        {
            ActiveDirectoryJoinInfo adJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
            if (adJoinInfo == null)
            {
                adJoinInfo = new ActiveDirectoryJoinInfo("WORKGROUP", "WORKGROUP", null);
            }

            return adJoinInfo;
        }
        catch(Exception e)
        {
            logger.error("Failed to get active directory join status");
            throw e;
        }
    }

    private
    Collection<DomainTrustsInfo>
    getDomainTrustInfo() throws IDMException, Exception
    {
        try
        {
            ActiveDirectoryJoinInfo adJoinInfo = IdmDomainState.getInstance().getDomainJoinInfo();
            if ( adJoinInfo != null && adJoinInfo.getJoinStatus() == ActiveDirectoryJoinInfo.JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN)
            {
                DomainTrustInfo[] domainTrustsInfo = IdmDomainState.getInstance().getDomainTrustInfo();
                //clone the DomainTrustInfo to another object, as it is not exposed to the client.
                Collection<DomainTrustsInfo> trustedDomains= new ArrayList<DomainTrustsInfo>();
                if (domainTrustsInfo != null && domainTrustsInfo.length > 0)
                {
                    for (DomainTrustInfo trust : domainTrustsInfo)
                    {
                        if (trust != null && trust.dcInfo != null)
                        {
                            DomainTrustsInfo domain =  new DomainTrustsInfo.DomainTrustInfoBuilder(trust.IsInforest,trust.IsOutBound,trust.IsInBound)
                                                   .isExternal(trust.isExternal)
                                                   .IsNativeMode(trust.IsNativeMode)
                                                   .IsPrimary(trust.IsPrimary)
                                                   .IsRoot(trust.IsRoot)
                                                   .dcInfo(trust.dcInfo.domainName, trust.dcInfo.domainNetBiosName, trust.dcInfo.domainIpAddress, trust.dcInfo.domainFQDN, trust.dcInfo.domainDnsForestName).build();
                            trustedDomains.add(domain);
                        }
                    }
                }
                return trustedDomains;
            }
            else
            {
                return null;
            }
        }
        catch(Exception e)
        {
            logger.error("Failed to get domain trust info");
            throw e;
        }
    }

    private String getClusterId() throws Exception
    {
        try
        {
            String tenantName = getSystemTenant();

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
            tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.getSiteId();
        }
        catch(Exception e)
        {
            logger.error("Failed to get the cluster identifier");
            throw e;
        }
    }

    private String getDeploymentId() throws Exception
    {
        try
        {
            String tenantName = getSystemTenant();

            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

            ISystemDomainIdentityProvider provider =
            tenantInfo.findSystemProvider();
            ServerUtils.validateNotNullSystemIdp(provider, tenantName);

            return provider.getDomainId();
        }
        catch(Exception e)
        {
            logger.error("Failed to get the deployment identifier");
            throw e;
        }
    }

    private String getSsoMachineHostName()
    throws Exception
    {
        try
        {
            return CommonUtil.getHostIPAddress(IdmUtils.getIdentityServicesConfigDir());
        }
        catch(Exception ex)
        {
            logger.error("Failed to get getSsoMachineHostName", ex);
            throw ex;
        }
    }

   private static IDiagnosticsContextScope getDiagnosticsContext(Tenant tenant, IIdmServiceContext serviceContext, String operationName)
   {
      return getDiagnosticsContext((tenant != null) ? tenant.getName() : "(NULL)", serviceContext, operationName);
   }

   private static IDiagnosticsContextScope getDiagnosticsContext(String tenantName, IIdmServiceContext serviceContext, String operationName)
   {
      String correlationId = null;
      if(serviceContext != null)
      {
          correlationId = serviceContext.getCorrelationId();
      }
      if (ServerUtils.isNullOrEmpty(correlationId))
      {
          correlationId = UUID.randomUUID().toString();
      }
      return getDiagnosticsContext(tenantName, correlationId, operationName);
   }

   private static IDiagnosticsContextScope getDiagnosticsContext(String tenantName, String correlationId, String operationName)
   {
       // for now we don't use operationName, but can add support in the future
       return DiagnosticsContextFactory.createContext(correlationId, tenantName);
   }

    // ---------------------------------------------
    // IIdentityManager interface implementation
    // ---------------------------------------------

    /**
     * {@inheritDoc}
     */
    @Override
    public void addTenant(Tenant tenant, String adminAccountName, char[] adminPwd, IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenant, serviceContext, "addTenant"))
        {
            try
            {
                this.addTenant(tenant, adminAccountName, adminPwd);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deleteTenant(String name, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(name, serviceContext, "deleteTenant"))
        {
            try
            {
                this.deleteTenant(name);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Tenant getTenant(String name, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(name, serviceContext, "getTenant"))
        {
            try
            {
                return this.getTenant(name);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDefaultTenant(IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getDefaultTenant"))
        {
            try
            {
                return this.getDefaultTenant();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getSystemTenant(IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getSystemTenant"))
        {
            try
            {
                return this.getSystemTenant();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<String> getAllTenants(IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getAllTenants"))
        {
            try
            {
                return this.getAllTenants();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDefaultTenant(String name, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(name, serviceContext, "setDefaultTenant"))
        {
            try
            {
                this.setDefaultTenant(name);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setTenant(Tenant tenant, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenant, serviceContext, "setTenant"))
        {
            try
            {
                this.setTenant(tenant);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getTenantSignatureAlgorithm(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getTenantSignatureAlgorithm"))
        {
            try
            {
                return this.getTenantSignatureAlgorithm(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setTenantSignatureAlgorithm(String tenantName,
            String signatureAlgorithm, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setTenantSignatureAlgorithm"))
        {
            try
            {
                this.setTenantSignatureAlgorithm(tenantName, signatureAlgorithm);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public List<Certificate> getTenantCertificate(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getTenantCertificate"))
        {
            try
            {
                return this.getTenantCertificate(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<List<Certificate>> getTenantCertificates(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getTenantCertificates"))
        {
            try
            {
                return this.getTenantCertificates(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setTenantTrustedCertificateChain(String tenantName,
            Collection<Certificate> tenantCertificates,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setTenantTrustedCertificateChain"))
        {
            try
            {
                this.setTenantTrustedCertificateChain(tenantName, tenantCertificates);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setTenantCredentials(String tenantName,
            Collection<Certificate> tenantCertificate, PrivateKey tenantPrivateKey,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setTenantCredentials"))
        {
            try
            {
                this.setTenantCredentials(tenantName, tenantCertificate, tenantPrivateKey);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrivateKey getTenantPrivateKey(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getTenantPrivateKey"))
        {
            try
            {
                return this.getTenantPrivateKey(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public boolean isTenantIDPSelectionEnabled(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getTenantIDPSelectionFlag"))
        {
            try
            {
                return this.isTenantIDPSelectionEnabled(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void setTenantIDPSelectionEnabled(String tenantName, boolean enableIDPSelection,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setTenantIDPSelectionFlag"))
        {
            try
            {
                this.setTenantIDPSelectionEnabled(tenantName, enableIDPSelection);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public long getClockTolerance(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getClockTolerance"))
        {
            try
            {
                return this.getClockTolerance(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setClockTolerance(String tenantName, long milliseconds,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setClockTolerance"))
        {
            try
            {
                this.setClockTolerance(tenantName, milliseconds);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getDelegationCount(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getDelegationCount"))
        {
            try
            {
                return this.getDelegationCount(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDelegationCount(String tenantName, int delegationCount,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setDelegationCount"))
        {
            try
            {
                this.setDelegationCount(tenantName, delegationCount);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getRenewCount(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getRenewCount"))
        {
            try
            {
                return this.getRenewCount(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setRenewCount(String tenantName, int renewCount,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setRenewCount"))
        {
            try
            {
                this.setRenewCount(tenantName, renewCount);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public SsoHealthStatsData getSsoStatistics(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException {

        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getSsoStatistics"))
        {
            try
            {
                return this.getSsoStatistics(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * It calls SSOHealthStatistcs service to increment generated tokens count
     * for a given tenant.
     */
    @Override
    public void incrementGeneratedTokens(String tenantName, IIdmServiceContext serviceContext) throws
            IDMException {

        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "incrementGeneratedTokens"))
        {
            try
            {
                this.incrementGeneratedTokens(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * It calls SSOHealthStatistcs service to increment renewed tokens count for
     * a given tenant.
     */
    @Override
    public void incrementRenewedTokens(String tenantName, IIdmServiceContext serviceContext) throws
            IDMException {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "incrementRenewedTokens"))
        {
            try
            {
                this.incrementRenewedTokens(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public long getMaximumBearerTokenLifetime(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getMaximumBearerTokenLifetime"))
        {
            try
            {
                return this.getMaximumBearerTokenLifetime(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setMaximumBearerTokenLifetime(String tenantName, long maxLifetime,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setMaximumBearerTokenLifetime"))
        {
            try
            {
                this.setMaximumBearerTokenLifetime(tenantName, maxLifetime);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public long getMaximumHoKTokenLifetime(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getMaximumHoKTokenLifetime"))
        {
            try
            {
                return this.getMaximumHoKTokenLifetime(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setMaximumHoKTokenLifetime(String tenantName, long maxLifetime,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setMaximumHoKTokenLifetime"))
        {
            try
            {
                this.setMaximumHoKTokenLifetime(tenantName, maxLifetime);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public long getMaximumBearerRefreshTokenLifetime(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getMaximumBearerRefreshTokenLifetime"))
        {
            try
            {
                return this.getMaximumBearerRefreshTokenLifetime(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setMaximumBearerRefreshTokenLifetime(String tenantName, long maxLifetime,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setMaximumBearerRefreshTokenLifetime"))
        {
            try
            {
                this.setMaximumBearerRefreshTokenLifetime(tenantName, maxLifetime);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public long getMaximumHoKRefreshTokenLifetime(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getMaximumHoKRefreshTokenLifetime"))
        {
            try
            {
                return this.getMaximumHoKRefreshTokenLifetime(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setMaximumHoKRefreshTokenLifetime(String tenantName, long maxLifetime,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setMaximumHoKRefreshTokenLifetime"))
        {
            try
            {
                this.setMaximumHoKRefreshTokenLifetime(tenantName, maxLifetime);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setEntityID(String tenantName, String entityID,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setEntityID"))
        {
            try
            {
                this.setEntityID(tenantName, entityID);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getEntityID(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getEntityID"))
        {
            try
            {
                return this.getEntityID(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public String getLocalIDPAlias(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getLocalIDPAlias"))
        {
            try
            {
                return this.getLocalIDPAlias(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void setLocalIDPAlias(String tenantName, String alias, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setIdentityProviderAlias"))
        {
            try
            {
                this.setLocalIDPAlias(tenantName, alias);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public String getExternalIDPAlias(String tenantName, String entityId, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getLocalIDPAlias"))
        {
            try
            {
                return this.getExternalIDPAlias(tenantName, entityId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void setExternalIDPAlias(String tenantName, String entityId, String alias, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setIdentityProviderAlias"))
        {
            try
            {
                this.setExternalIDPAlias(tenantName, entityId, alias);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getOIDCEntityID(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getOIDCEntityID"))
        {
            try
            {
                return this.getOIDCEntityID(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PasswordExpiration getPasswordExpirationConfiguration(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getPasswordExpirationConfiguration"))
        {
            try
            {
                return this.getPasswordExpirationConfiguration(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void updatePasswordExpirationConfiguration(String tenantName,
            PasswordExpiration config, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "updatePasswordExpirationConfiguration"))
        {
            try
            {
                this.updatePasswordExpirationConfiguration(tenantName, config);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addCertificate(String tenantName, Certificate idmCert,
            CertificateType certificateType, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addCertificate"))
        {
            try
            {
                this.addCertificate(tenantName, idmCert, certificateType);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<Certificate> getAllCertificates(String tenantName,
            CertificateType certificateType, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getAllCertificates"))
        {
            try
            {
                return this.getAllCertificates(tenantName, certificateType);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deleteCertificate(String tenantName, String fingerprint,
            CertificateType certificateType, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "deleteCertificate"))
        {
            try
            {
                this.deleteCertificate(tenantName, fingerprint, certificateType);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addRelyingParty(String tenantName, RelyingParty rp,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addRelyingParty"))
        {
            try
            {
                this.addRelyingParty(tenantName, rp);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deleteRelyingParty(String tenantName, String rpName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "deleteRelyingParty"))
        {
            try
            {
                this.deleteRelyingParty(tenantName, rpName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RelyingParty getRelyingParty(String tenantName, String rpName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getRelyingParty"))
        {
            try
            {
                return this.getRelyingParty(tenantName, rpName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RelyingParty getRelyingPartyByUrl(String tenantName, String url,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getRelyingPartyByUrl"))
        {
            try
            {
                return this.getRelyingPartyByUrl(tenantName, url);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setRelyingParty(String tenantName, RelyingParty rp,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setRelyingParty"))
        {
            try
            {
                this.setRelyingParty(tenantName, rp);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<RelyingParty> getRelyingParties(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getRelyingParties"))
        {
            try
            {
                return this.getRelyingParties(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addOIDCClient(String tenantName, OIDCClient oidcClient, IIdmServiceContext serviceContext)
            throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addOIDCClient")) {
            try {
                this.addOIDCClient(tenantName, oidcClient);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deleteOIDCClient(String tenantName, String clientID, IIdmServiceContext serviceContext)
            throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "deleteOIDCClient")) {
            try {
                this.deleteOIDCClient(tenantName, clientID);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public OIDCClient getOIDCClient(String tenantName, String clientID, IIdmServiceContext serviceContext)
            throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getOIDCClient")) {
            try {
                return this.getOIDCClient(tenantName, clientID);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setOIDCClient(String tenantName, OIDCClient oidcClient, IIdmServiceContext serviceContext)
            throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setOIDCClient")) {
            try {
                this.setOIDCClient(tenantName, oidcClient);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<OIDCClient> getOIDCClients(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getOIDCClients")) {
            try {
                return this.getOIDCClients(tenantName);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void addResourceServer(
            String tenantName,
            ResourceServer resourceServer,
            IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addResourceServer")) {
            try {
                this.addResourceServer(tenantName, resourceServer);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void deleteResourceServer(
            String tenantName,
            String resourceServerName,
            IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "deleteResourceServer")) {
            try {
                this.deleteResourceServer(tenantName, resourceServerName);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public ResourceServer getResourceServer(
            String tenantName,
            String resourceServerName,
            IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getResourceServer")) {
            try {
                return this.getResourceServer(tenantName, resourceServerName);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void setResourceServer(
            String tenantName,
            ResourceServer resourceServer,
            IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setResourceServer")) {
            try {
                this.setResourceServer(tenantName, resourceServer);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public Collection<ResourceServer> getResourceServers(
            String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getResourceServers")) {
            try {
                return this.getResourceServers(tenantName);
            } catch (Exception ex) {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addProvider(String tenantName, IIdentityStoreData idpData,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addProvider"))
        {
            try
            {
                this.addProvider(tenantName, idpData);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deleteProvider(String tenantName, String providerName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "deleteProvider"))
        {
            try
            {
                this.deleteProvider(tenantName, providerName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IIdentityStoreData getProvider(String tenantName, String ProviderName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getProvider"))
        {
            try
            {
                return this.getProvider(tenantName, ProviderName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IIdentityStoreData getProviderWithInternalInfo(String tenantName, String ProviderName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getProvider"))
        {
            try
            {
                return this.getProviderWithInternalInfo(tenantName, ProviderName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setProvider(String tenantName, IIdentityStoreData idpData,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setProvider"))
        {
            try
            {
                this.setProvider(tenantName, idpData);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setNativeADProvider(String tenantName, IIdentityStoreData idpData,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setProvider"))
        {
            try
            {
                this.setNativeADProvider(tenantName, idpData);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<IIdentityStoreData> getProviders(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getProviders"))
        {
            try
            {
                return this.getProviders(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public
    Collection<SecurityDomain>
    getSecurityDomains(
                       String tenantName,
                       String providerName,
                       IIdmServiceContext serviceContext
                      ) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getSecurityDomains"))
        {
            try
            {
                return this.getSecurityDomains(tenantName, providerName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }


    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<IIdentityStoreData> getProviders(String tenantName,
            EnumSet<DomainType> domainTypes, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getProviders"))
        {
            try
            {
                return this.getProviders(tenantName, domainTypes);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void probeProviderConnectivity(String tenantName, IIdentityStoreData idsData, IIdmServiceContext serviceContext) throws IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "probeProviderConnectivity"))
        {
            probeProviderConnectivity(tenantName, idsData);
        }
        catch(Exception ex)
        {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void probeProviderConnectivity(String tenantName, String providerUri,
            AuthenticationType authType, String userName, String pwd, Collection<X509Certificate> certificates,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "probeProviderConnectivity"))
        {
            try
            {
                LdapCertificateValidationSettings certValidationSettings = new LdapCertificateValidationSettings(certificates);
                this.probeProviderConnectivity(tenantName, providerUri, authType, userName, pwd, certValidationSettings);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void probeProviderConnectivityWithCertValidation(String tenantName, String providerUri,
            AuthenticationType authType, String userName, String pwd, Collection<X509Certificate> certificates,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "probeProviderConnectivity"))
        {
            try
            {
                LdapCertificateValidationSettings certValidationSettings = new LdapCertificateValidationSettings(certificates, null, true);
                this.probeProviderConnectivity(tenantName, providerUri, authType, userName, pwd, certValidationSettings);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<String> getDefaultProviders(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getDefaultProviders"))
        {
            try
            {
                return this.getDefaultProviders(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDefaultProviders(String tenantName,
            Collection<String> defaultProviders, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setDefaultProviders"))
        {
            try
            {
                this.setDefaultProviders(tenantName, defaultProviders);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId authenticate(String tenantName, String principal,
            String password, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "authenticate"))
        {
            try
            {
                return this.authenticate(tenantName, principal, password);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public GSSResult authenticate(String tenantName, String contextId, byte[] gssTicket,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "authenticate"))
        {
            try
            {
                return this.authenticate(tenantName, contextId, gssTicket);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }


    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId authenticate(String tenantName, X509Certificate[] tlsCertChain, String hint,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "authenticate"))
        {
            try
            {
                return this.authenticate(tenantName, tlsCertChain, hint);
            } catch (Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * authenticate with secure ID
     *
     * @param tenantName
     * @param principal
     *            userID (if userID is UPN) or userID@domainName (if userID is not UPN)
     * @param passcode
     * @return
     * @throws RemoteException
     */

    @Override
    public RSAAMResult authenticateRsaSecurId(String tenantName,
            String sessionId, String principal,
            String passcode, IIdmServiceContext serviceContext) throws  IDMException
    {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "authenticate"))
        {
            IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
                    tenantName,
                    IdentityManager.PROVIDER_TYPE_RSA_SECURID, IdentityManager.PROVIDER_TYPE_RSA_SECURID, 0,
                    ActivityKind.AUTHENTICATE, EventLevel.INFO, principal);
            idmAuthStatRecorder.start();

            try
            {

                RSAAMResult result = this.authenticateRsaSecurId(tenantName, sessionId,
                        principal,
                        passcode);
                idmAuthStatRecorder.end();
                return result;
            } catch (Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    protected IIdmAuthStatRecorder createIdmAuthStatRecorderInstance(
            String tenantName, String providerName, String providerType, int providerFlag, ActivityKind opType, EventLevel eventLevel, String id) {
        if (PerformanceMonitorFactory.getPerformanceMonitor().getCache(tenantName).isEnabled()) {
            return new IdmAuthStatRecorder(
                    tenantName,
                    providerName,
                    providerType,
                    providerFlag,
                    opType,
                    eventLevel,
                    id != null ? id : "",
                    PerformanceMonitorFactory.getPerformanceMonitor().summarizeLdapQueries(),
                    DiagnosticsContextFactory.getCurrentDiagnosticsContext().getCorrelationId());
        } else {
            return NoopIdmAuthStatRecorder.getInstance();
        }
    }


    /**
     * {@inheritDoc}
     */
    @Override
    public boolean IsActive(String tenantName, PrincipalId principal,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "IsActive"))
        {
            try
            {
                return IdentityManager.IsActive(findTenant(tenantName), tenantName, principal);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<Attribute> getAttributeDefinitions(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getAttributeDefinitions"))
        {
            try
            {
                return this.getAttributeDefinitions(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<AttributeValuePair> getAttributeValues(String tenantName,
            PrincipalId principal, Collection<Attribute> attributes,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getAttributeValues"))
        {
            try
            {
                return this.getAttributeValues(tenantName, principal, attributes);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public byte[] getUserHashedPassword(String tenantName, PrincipalId principal,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getUserHashedPassword"))
        {
            try
            {
                return this.getUserHashedPassword(tenantName, principal);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId addSolutionUser(String tenantName, String userName,
            SolutionDetail detail, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addSolutionUser"))
        {
            try
            {
                return this.addSolutionUser(tenantName, userName, detail);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SolutionUser findSolutionUser(String tenantName, String userName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findSolutionUser"))
        {
            try
            {
                return this.findSolutionUser(tenantName, userName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SolutionUser findSolutionUserByCertDn(String tenantName,
            String subjectDN, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findSolutionUserByCertDn"))
        {
            try
            {
                return this.findSolutionUserByCertDn(tenantName, subjectDN);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PersonUser findPersonUser(String tenantName, PrincipalId id,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findPersonUser"))
        {
            try
            {
                return this.findPersonUser(tenantName, id);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PersonUser findPersonUserByObjectId(String tenantName,
            String userObjectId, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findPersonUserByObjectId"))
        {
            try
            {
                return this.findPersonUserByObjectId(tenantName, userObjectId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Group findGroupByObjectId(String tenantName, String groupObjectId,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findGroupByObjectId"))
        {
            try
            {
                return this.findGroupByObjectId(tenantName, groupObjectId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<PersonUser> findPersonUsers(
        String tenantName,
        SearchCriteria criteria,
        int limit,
        IIdmServiceContext serviceContext)
        throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findPersonUsers"))
        {
            try
            {
                return this.findPersonUsers(tenantName, criteria, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<PersonUser> findPersonUsersByName(
        String tenantName,
        SearchCriteria criteria,
        int limit,
        IIdmServiceContext serviceContext)
        throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findPersonUsersByName"))
        {
            try
            {
                return this.findPersonUsersByName(tenantName, criteria, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<SolutionUser> findSolutionUsers(String tenantName,
            String searchString,
            int limit,
            IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findSolutionUsers"))
        {
            try
            {
                return this.findSolutionUsers(tenantName, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<Group> findGroups(String tenantName, SearchCriteria criteria, int limit,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findGroups"))
        {
            try
            {
                return this.findGroups(tenantName, criteria, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<Group> findGroupsByName(String tenantName, SearchCriteria criteria, int limit,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findGroupsByName"))
        {
            try
            {
                return this.findGroupsByName(tenantName, criteria, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<PersonUser> findPersonUsersInGroup(String tenantName,
            PrincipalId groupId, String searchString, int limit,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findPersonUsersInGroup"))
        {
            try
            {
                return this.findPersonUsersInGroup(tenantName, groupId, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<PersonUser> findPersonUsersByNameInGroup(String tenantName,
            PrincipalId groupId, String searchString, int limit,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findPersonUsersByNameInGroup"))
        {
            try
            {
                return this.findPersonUsersByNameInGroup(tenantName, groupId, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<SolutionUser> findSolutionUsersInGroup(String tenantName,
            String groupName, String searchString, int limit, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findSolutionUsersInGroup"))
        {
            try
            {
                return this.findSolutionUsersInGroup(tenantName, groupName, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<Group> findGroupsInGroup(String tenantName, PrincipalId groupId,
            String searchString, int limit, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findGroupsInGroup"))
        {
            try
            {
                return this.findGroupsInGroup(tenantName, groupId, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<Group> findGroupsByNameInGroup(String tenantName, PrincipalId groupId,
            String searchString, int limit, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findGroupsByNameInGroup"))
        {
            try
            {
                return this.findGroupsByNameInGroup(tenantName, groupId, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isMemberOfSystemGroup(String tenantName, PrincipalId principalId, String groupName, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException,  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "isMemberOfSystemGroup"))
        {
            try
            {
                return this.isMemberOfSystemGroup(tenantName, principalId, groupName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<Group> findDirectParentGroups(String tenantName,
            PrincipalId principalId, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findDirectParentGroups"))
        {
            try
            {
                return this.findDirectParentGroups(tenantName, principalId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<Group> findNestedParentGroups(String tenantName, PrincipalId userId,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findNestedParentGroups"))
        {
            try
            {
                return this.findNestedParentGroups(tenantName, userId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<PersonUser> findLockedUsers(String tenantName, String searchString, int limit,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findLockedUsers"))
        {
            try
            {
                return this.findLockedUsers(tenantName, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<PersonUser> findDisabledPersonUsers(String tenantName,
            String searchString, int limit, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findDisabledPersonUsers"))
        {
            try
            {
                return this.findDisabledPersonUsers(tenantName, searchString, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId findActiveUserInSystemDomain(String tenantName,
            String attributeName, String attributeValue,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findActiveUserInSystemDomain"))
        {
            try
            {
                return this.findActiveUserInSystemDomain(tenantName, attributeName, attributeValue);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<SolutionUser> findDisabledSolutionUsers(String tenantName,
            String searchString, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findDisabledSolutionUsers"))
        {
            try
            {
                return this.findDisabledSolutionUsers(tenantName, searchString);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SearchResult find(String tenantName, SearchCriteria criteria, int limit,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "find"))
        {
            try
            {
                return this.find(tenantName, criteria, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SearchResult findByName(String tenantName, SearchCriteria criteria, int limit,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findByName"))
        {
            try
            {
                return this.findByName(tenantName, criteria, limit);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId addUser(String tenantName, String userName,
            PersonDetail detail, char[] password, IIdmServiceContext serviceContext)
            throws Exception
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addUser"))
        {
            try
            {
                return this.addUser(tenantName, userName, detail, null, null, password);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId addUser(String tenantName, String userName,
            PersonDetail detail, byte[] hashedPassword, String hashingAlgorithm,
            IIdmServiceContext serviceContext) throws Exception
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addUser"))
        {
            try
            {
                return this.addUser(tenantName, userName, detail, hashedPassword, hashingAlgorithm);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId addJitUser(String tenantName, String userName,
            PersonDetail detail, String extIdpEntityId, String extUserId,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addUser"))
        {
            try
            {
                return this.addUser(tenantName, userName, detail, extIdpEntityId, extUserId, null);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId addGroup(String tenantName, String groupName,
            GroupDetail groupDetail, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addGroup"))
        {
            try
            {
                return this.addGroup(tenantName, groupName, groupDetail);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean addUserToGroup(String tenantName, PrincipalId userId,
            String groupName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addUserToGroup"))
        {
            try
            {
                return this.addUserToGroup(tenantName, userId, groupName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean removeFromLocalGroup(String tenantName, PrincipalId principalId,
            String groupName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "removeFromLocalGroup"))
        {
            try
            {
                return this.removeFromLocalGroup(tenantName, principalId, groupName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean addGroupToGroup(String tenantName, PrincipalId groupId,
            String groupName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "addGroupToGroup"))
        {
            try
            {
                return this.addGroupToGroup(tenantName, groupId, groupName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deletePrincipal(String tenantName, String principalName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "deletePrincipal"))
        {
            try
            {
                this.deletePrincipal(tenantName, principalName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean enableUserAccount(String tenantName, PrincipalId userId,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "enableUserAccount"))
        {
            try
            {
                return this.enableUserAccount(tenantName, userId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean unlockUserAccount(String tenantName, PrincipalId userId,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "unlockUserAccount"))
        {
            try
            {
                return this.unlockUserAccount(tenantName, userId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean disableUserAccount(String tenantName, PrincipalId userId,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "disableUserAccount"))
        {
            try
            {
                return this.disableUserAccount(tenantName, userId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId updatePersonUserDetail(String tenantName, String userName,
            PersonDetail detail, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "updatePersonUserDetail"))
        {
            try
            {
                return this.updatePersonUserDetail(tenantName, userName, detail);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId updateGroupDetail(String tenantName, String groupName,
            GroupDetail detail, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "updateGroupDetail"))
        {
            try
            {
                return this.updateGroupDetail(tenantName, groupName, detail);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PrincipalId updateSolutionUserDetail(String tenantName, String userName,
            SolutionDetail detail, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "updateSolutionUserDetail"))
        {
            try
            {
                return this.updateSolutionUserDetail(tenantName, userName, detail);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setUserPassword(String tenantName, String userName,
            char[] newPassword, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setUserPassword"))
        {
            try
            {
                this.setUserPassword(tenantName, userName, newPassword);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void changeUserPassword(String tenantName, String userName,
            char[] currentPassword, char[] newPassword,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "changeUserPassword"))
        {
            try
            {
                this.changeUserPassword(tenantName, userName, currentPassword, newPassword);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void updateSystemDomainStorePassword(String tenantName, char[] newPassword,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
        ValidateUtil.validateNotNull(newPassword, "New password");

        ILdapConnectionEx connection = null;
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "updateSystemDomainStorePassword"))
        {
            ServerIdentityStoreData systemStoreData =
                    this.getSystemDomainIdentityStoreData(tenantName);

            // validate the new password
            Collection<String> connectionStrings = systemStoreData.getConnectionStrings();
            connection = ServerUtils.getLdapConnectionByURIs(
                    ServerUtils.toURIObjects(connectionStrings),
                    systemStoreData.getUserName(), new String(newPassword),
                    systemStoreData.getAuthenticationType(), false);

            // update password after validation
            systemStoreData.setPassword( new String(newPassword) );
            _configStore.setProvider(tenantName, systemStoreData);
            _tenantCache.deleteTenant(tenantName);
        }
        catch(Exception ex)
        {
            logger.error("Encountered an error while updating system domain "
                    + "identity store password.", ex);
            throw ServerUtils.getRemoteException(ex);
        }
        finally
        {
            if(connection != null)
                connection.close();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PasswordPolicy getPasswordPolicy(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getPasswordPolicy"))
        {
            try
            {
                return this.getPasswordPolicy(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setPasswordPolicy(String tenantName, PasswordPolicy policy,
            IIdmServiceContext serviceContext) throws Exception
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setPasswordPolicy"))
        {
            try
            {
                this.setPasswordPolicy(tenantName, policy);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public LockoutPolicy getLockoutPolicy(String tenantName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getLockoutPolicy"))
        {
            try
            {
                return this.getLockoutPolicy(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setLockoutPolicy(String tenantName, LockoutPolicy policy,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setLockoutPolicy"))
        {
            try
            {
                this.setLockoutPolicy(tenantName, policy);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Group findGroup(String tenantName, PrincipalId groupId,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findGroup"))
        {
            try
            {
                return this.findGroup(tenantName, groupId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Group findGroup(String tenantName, String group,
            IIdmServiceContext serviceContext) throws
            NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException,
            IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findGroup"))
        {
            try
            {
                return this.findGroup(tenantName, group);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Principal findUser(String tenantName, String user,
            IIdmServiceContext serviceContext) throws
            NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException,
            IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findUser"))
        {
            try
            {
                return this.findUser(tenantName, user);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setExternalIdpForTenant(String tenantName, IDPConfig idpConfig,
            IIdmServiceContext serviceContext) throws
            NoSuchTenantException, IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setExternalIdpForTenant"))
        {
            try
            {
                this.setExternalIdpForTenant(tenantName, idpConfig);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeExternalIdpForTenant(String tenantName,
            String configEntityId, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException,
            NoSuchExternalIdpConfigException, IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "removeExternalIdpForTenant"))
        {
            try
            {
                this.removeExternalIdpForTenant(tenantName, configEntityId, false);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeExternalIdpForTenant(String tenantName,
            String configEntityId, boolean removeJitUsers, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException,
            NoSuchExternalIdpConfigException, IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "removeExternalIdpForTenant"))
        {
            try
            {
                this.removeExternalIdpForTenant(tenantName, configEntityId, removeJitUsers);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<IDPConfig> getAllExternalIdpsForTenant(String tenantName,
            IIdmServiceContext serviceContext) throws
            NoSuchTenantException, IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getAllExternalIdpsForTenant"))
        {
            try
            {
                return this.getAllExternalIdpsForTenant(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IDPConfig getExternalIdpForTenant(String tenantName,
            String configEntityId, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException, IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getExternalIdpForTenant"))
        {
            try
            {
                return this.getExternalIdpForTenant(tenantName, configEntityId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<IDPConfig> getExternalIdpForTenantByUrl(String tenantName,
            String urlStr, IIdmServiceContext serviceContext)
            throws  NoSuchTenantException, IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getExternalIdpForTenantByUrl"))
        {
            try
            {
                return this.getExternalIdpForTenantByUrl(tenantName, urlStr);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean registerThirdPartyIDPUser(String tenantName, PrincipalId userId,
            IIdmServiceContext serviceContext) throws
            IDMException, NoSuchTenantException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "registerThirdPartyIDPUser"))
        {
            try
            {
                return this.registerThirdPartyIDPUser(tenantName, userId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean removeThirdPartyIDPUser(String tenantName, PrincipalId userId,
            IIdmServiceContext serviceContext) throws
            IDMException, NoSuchTenantException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "removeThirdPartyIDPUser"))
        {
            try
            {
                return this.removeThirdPartyIDPUser(tenantName, userId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PersonUser findRegisteredExternalIDPUser(String tenantName,
            PrincipalId userId, IIdmServiceContext serviceContext)
            throws  IDMException, NoSuchTenantException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "findRegisteredExternalIDPUser"))
        {
            try
            {
                return this.findRegisteredExternalIDPUser(tenantName, userId);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getExternalIDPRegistrationGroupName(
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getExternalIDPRegistrationGroupName"))
        {
            try
            {
                return this.getExternalIDPRegistrationGroupName();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean registerUpnSuffix(String tenantName, String domainName,
            String upnSuffix, IIdmServiceContext serviceContext)
            throws  IDMException, NoSuchTenantException,
            NoSuchIdpException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "registerUpnSuffix"))
        {
            try
            {
                return this.registerUpnSuffix(tenantName, domainName, upnSuffix);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean unregisterUpnSuffix(String tenantName, String domainName,
            String upnSuffix, IIdmServiceContext serviceContext)
            throws  IDMException, NoSuchTenantException,
            NoSuchIdpException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "unregisterUpnSuffix"))
        {
            try
            {
                return this.unregisterUpnSuffix(tenantName, domainName, upnSuffix);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<String> getUpnSuffixes(String tenantName, String domainName,
            IIdmServiceContext serviceContext) throws
            IDMException, NoSuchTenantException, NoSuchIdpException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getUpnSuffixes"))
        {
            try
            {
                return this.getUpnSuffixes(tenantName, domainName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getBrandName(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getBrandName"))
        {
            try
            {
                return this.getBrandName(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setBrandName(String tenantName, String brandName,
            IIdmServiceContext serviceContext) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setBrandName"))
        {
            try
            {
                this.setBrandName(tenantName, brandName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getLogonBannerContent(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getLogonBannerContent"))
        {
            try
            {
                return this.getLogonBannerContent(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setLogonBannerContent(String tenantName, String logonBannerContent, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setLogonBannerContent"))
        {
            try
            {
                this.setLogonBannerContent(tenantName, logonBannerContent);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getLogonBannerTitle(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getLogonBannerTitle"))
        {
            try
            {
                return this.getLogonBannerTitle(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public boolean getLogonBannerCheckboxFlag(String tenantName, IIdmServiceContext serviceContext)
            throws   IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getLogonBannerCheckboxFlag"))
        {
            try
            {
                return this.getLogonBannerCheckboxFlag(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setLogonBannerTitle(String tenantName, String logonBannerTitle, IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setLogonBannerTitle"))
        {
            try
            {
                this.setLogonBannerTitle(tenantName, logonBannerTitle);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    @Override
    public void setLogonBannerCheckboxFlag(String tenantName, boolean enableLogonBannerCheckbox, IIdmServiceContext serviceContext)
            throws   IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setLogonBannerCheckbox"))
        {
            try
            {
                this.setLogonBannerCheckbox(tenantName, enableLogonBannerCheckbox);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public
    Collection<Certificate>
    getTrustedCertificates(
        String tenantName, IIdmServiceContext serviceContext
    ) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getTrustedCertificates"))
        {
            try
            {
                return this.getTrustedCertificates(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public
    Collection<Certificate>
    getStsIssuersCertificates(
        String tenantName, IIdmServiceContext serviceContext
    ) throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getStsIssuersCertificates"))
        {
            try
            {
                return this.getStsIssuersCertificates(tenantName);
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /*
     * {@inheritDoc}
     */
    @Override
    public
    ActiveDirectoryJoinInfo
    getActiveDirectoryJoinStatus(IIdmServiceContext serviceContext)
        throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getActiveDirectoryJoinStatus"))
        {
            try
            {
                return this.getActiveDirectoryJoinStatus();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /*
     * {@inheritDoc}
     */
    @Override
    public
    Collection<DomainTrustsInfo>
    getDomainTrustInfo(IIdmServiceContext serviceContext)
        throws IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getDomainTrustInfo"))
        {
            try
            {
                return this.getDomainTrustInfo();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /*
     * {@inheritDoc}
     */
    @Override
    public String getClusterId(IIdmServiceContext serviceContext)
        throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getClusterId"))
        {
            try
            {
                return this.getClusterId();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /*
     * {@inheritDoc}
     */
    @Override
    public String getDeploymentId(IIdmServiceContext serviceContext)
        throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getDeploymentId"))
        {
            try
            {
                return this.getDeploymentId();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

    /*
     * {@inheritDoc}
     */
    @Override
    public String getSsoMachineHostName(IIdmServiceContext serviceContext)
            throws  IDMException
    {
        try(IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "getSsoMachineHostName"))
        {
            try
            {
                return this.getSsoMachineHostName();
            }
            catch(Exception ex)
            {
                throw ServerUtils.getRemoteException(ex);
            }
        }
    }

   @Override
   public Collection<VmHostData> getComputers(String tenantName, boolean getDCOnly, IIdmServiceContext serviceContext)
            throws  IDMException
   {

      try(IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getComputers"))
      {
         TenantInformation tenantInfo = findTenant(tenantName);
         ServerUtils.validateNotNullTenant(tenantInfo, tenantName);

         ISystemDomainIdentityProvider provider = tenantInfo.findSystemProvider();

         ServerUtils.validateNotNullSystemIdp(provider, tenantName);

         Collection<VmHostData> hosts = provider.getComputers(getDCOnly);

         return hosts;
      }
      catch(Exception ex)
      {
         logger.error(String.format("Failed to get joined systems from tenant [%s]", tenantName), ex);
         throw ServerUtils.getRemoteException(ex);
      }
   }

   @Override
   public void joinActiveDirectory(String user, String password, String domain, String orgUnit, IIdmServiceContext serviceContext)
      throws  IDMException, IdmADDomainException
   {
      try (IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "joinActiveDirectory"))
      {
         VmafClientUtil.joinActiveDirectory(user, password, domain, orgUnit);
      } catch (VmAfAccessDeniedException e) {
        logger.error("VmafAccessDeniedException occurred", e);
        throw new IdmADDomainException(String.format("user [%s] cannot access domain [%s]", user, domain), e.getErrorCode());
    } catch (VmAfUnknownServerException | VmAfNoSuchDomainException e) {
        logger.error(e.getClass().getSimpleName() + " occurred", e);
        throw new IdmADDomainException(String.format("cannot contact domain [%s]", domain), e.getErrorCode());
    } catch (VmAfAlreadyJoinedException e) {
        logger.error("VmafAlreadyJoinedException occurred", e);
        throw new IdmADDomainException("SSO server is already joined to AD domain", e.getErrorCode());
    } catch (VmAfInvalidComputerNameException e) {
        logger.error("VmAfInvalidComputerNameException occurred", e);
        throw new IdmADDomainException(
            String.format("The format of the specified computer name is invalid [%s]", domain), e.getErrorCode());
    } catch (VmAfBadPacketException e) {
        logger.error("VmAfBadPacketException occurred", e);
        throw new IdmADDomainException(
            String.format(
                    "A bad packet was received from a DNS server. Potentially the requested address [%s] does not exist.",
                    domain), e.getErrorCode());
    } catch (VmAfInvalidParameterException e) {
        logger.error("VmAfInvalidParameterException occurred", e);
        throw new IdmADDomainException("Invalid parameter passed",
            e.getErrorCode());
    } catch (VmAfNoSuchLogonSessionException e) {
        logger.error("VmAfNoSuchLogonSessionException occurred", e);
        throw new IdmADDomainException(
            String.format(
                    "The specified logon session does not exist: domain [%s], username [%s]",
                    domain, user), e.getErrorCode());
    } catch (VmAfWrongPasswordException e) {
        logger.error("VmAfWrongPasswordException occurred", e);
        throw new IdmADDomainException(
            "The value provided as the current password is incorrect",
            e.getErrorCode());
    } catch (VmAfNotSupportedException e) {
        logger.error("VmAfNotSupportedException occurred", e);
        throw new IdmADDomainException("The request is not supported",
            e.getErrorCode());
    } catch (VmAfLdapNoSuchObjectException e) {
        logger.error("VmAfLdapNoSuchObjectException occurred", e);
        throw new IdmADDomainException(
            String.format(
                    "The specified entry does not exist in the directory, domain [%s], orgUnit [%s]",
                    domain, orgUnit), e.getErrorCode());
    } catch (VmAfClientNativeException e) {
        logger.error("VmAfClientNativeException occurred", e);
        throw new IDMException(
            String.format(
                    "Error trying to join AD, error code [%s], user [%s], domain [%s], orgUnit [%s]",
                    e.getErrorCode(), user, domain, orgUnit));
      }catch (Exception ex){
         throw ServerUtils.getRemoteException(ex);
      }
   }

   @Override
   public void leaveActiveDirectory(String user, String password, IIdmServiceContext serviceContext)
         throws  IDMException, ADIDSAlreadyExistException, IdmADDomainException
   {
      try (IDiagnosticsContextScope ctxt = getDiagnosticsContext("", serviceContext, "leaveActiveDirectory"))
      {
         String adIDP = findIdpTypeRegistered(getSystemTenant(), IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY);
         if (adIDP != null)
         {
            throw new ADIDSAlreadyExistException(adIDP);
         }
         VmafClientUtil.leaveActiveDirectory(user, password);
      } catch (VmAfAccessDeniedException e) {
        logger.error("VmafAccessDeniedException occurred", e);
        throw new IdmADDomainException(String.format("user [%s] cannot access domain [%s]", user, null), e.getErrorCode());
    } catch (VmAfUnknownServerException | VmAfNoSuchDomainException e) {
        logger.error(e.getClass().getSimpleName() + " occurred", e);
        throw new IdmADDomainException(String.format("cannot contact domain [%s]", null), e.getErrorCode());
    } catch (VmAfAlreadyJoinedException e) {
        logger.error("VmafAlreadyJoinedException occurred", e);
        throw new IdmADDomainException("SSO server is already joined to AD domain", e.getErrorCode());
    } catch (VmAfInvalidComputerNameException e) {
        logger.error("VmAfInvalidComputerNameException occurred", e);
        throw new IdmADDomainException(
            String.format("The format of the specified computer name is invalid [%s]", null), e.getErrorCode());
    } catch (VmAfBadPacketException e) {
        logger.error("VmAfBadPacketException occurred", e);
        throw new IdmADDomainException(
            String.format(
                    "A bad packet was received from a DNS server. Potentially the requested address [%s] does not exist.",
                    null), e.getErrorCode());
    } catch (VmAfInvalidParameterException e) {
        logger.error("VmAfInvalidParameterException occurred", e);
        throw new IdmADDomainException("Invalid parameter passed",
            e.getErrorCode());
    } catch (VmAfNoSuchLogonSessionException e) {
        logger.error("VmAfNoSuchLogonSessionException occurred", e);
        throw new IdmADDomainException(
            String.format(
                    "The specified logon session does not exist: domain [%s], username [%s]",
                    null, user), e.getErrorCode());
    } catch (VmAfWrongPasswordException e) {
        logger.error("VmAfWrongPasswordException occurred", e);
        throw new IdmADDomainException(
            "The value provided as the current password is incorrect",
            e.getErrorCode());
    } catch (VmAfNotSupportedException e) {
        logger.error("VmAfNotSupportedException occurred", e);
        throw new IdmADDomainException("The request is not supported",
            e.getErrorCode());
    } catch (VmAfLdapNoSuchObjectException e) {
        logger.error("VmAfLdapNoSuchObjectException occurred", e);
        throw new IdmADDomainException("The specified entry does not exist in the directory",
                    e.getErrorCode());
    } catch (VmAfClientNativeException e) {
        logger.error("VmAfClientNativeException occurred", e);
        throw new IDMException(
            String.format(
                    "Error trying to leave AD, error code [%s], user [%s]",
                    e.getErrorCode(), user));
      }catch (Exception ex){
         throw ServerUtils.getRemoteException(ex);
      }
   }

    @Override
    public List<IIdmAuthStat> getIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getIdmAuthStats"))
        {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            return PerformanceMonitor.getInstance().getCache(tenantName).getIdmAuthStats();
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    @Override
    public IIdmAuthStatus getIdmAuthStatus(String tenantName, IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "getIdmAuthStatus")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            IdmAuthStatCache cache = PerformanceMonitor.getInstance().getCache(tenantName);
            return new IdmAuthStatus(cache.isEnabled(), cache.getDepth());
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    @Override
    public void clearIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "clearIdmAuthStats")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            PerformanceMonitor.getInstance().getCache(tenantName).clear();
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    @Override
    public void setIdmAuthStatsSize(String tenantName, int size, IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "setIdmAuthStatsSize")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            ValidateUtil.validateNonNegativeNumber(size, "size");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            PerformanceMonitor.getInstance().getCache(tenantName).setDepth(size);
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    @Override
    public void enableIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "enableIdmAuthStats")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            PerformanceMonitor.getInstance().getCache(tenantName).enable();
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    @Override
    public void disableIdmAuthStats(String tenantName, IIdmServiceContext serviceContext) throws  IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName, serviceContext, "disableIdmAuthStats")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            PerformanceMonitor.getInstance().getCache(tenantName).disable();
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    /**
     * Retrieves SPN in the form of HTTP/computername.domainname in case is
     * AD-joined, or the HTTP/computer name if not joined.
     *
     * Use case: WEBSSO server pass this information to logon page script for
     * windows session authentication.
     *
     * @throws Exception
     *
     *@return
     *
     * null : If the machine is not joined AD or left AD domain <br/>
     * HTTP/computername.domainname : If machine is domain joined.
     */
    @Override
    public String getServerSPN() throws Exception {

        IIdmClientLibrary idmAdapter = IdmClientLibraryFactory.getInstance()
                .getLibrary();

        String spn = null;
        try {
            ActiveDirectoryJoinInfo joinInfo = getActiveDirectoryJoinStatus();
            ActiveDirectoryJoinInfo.JoinStatus joinStatus = joinInfo
                    .getJoinStatus();
            if (logger.isDebugEnabled()) {
                logger.debug(
                        "AD-joined status is",
                        (joinStatus == ActiveDirectoryJoinInfo.JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN) ? "joined"
                                : "not joined");
            }
            String domainName = (joinInfo.getJoinStatus() == ActiveDirectoryJoinInfo.JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN) ? joinInfo
                    .getName() : null;

                    if (domainName != null && !domainName.isEmpty()) {
                if (SystemUtils.IS_OS_LINUX) {
                    final String FQDN_CONFIG_KEY =
                        "Services\\lsass\\Parameters\\Providers\\ActiveDirectory\\DomainJoin\\"
                    + domainName.toUpperCase() + "\\Pstore";

                    IRegistryAdapter registryAdapter = RegistryAdapterFactory.getInstance().getRegistryAdapter();
                    IRegistryKey rootRegistryKey = registryAdapter.openRootKey((int) RegKeyAccess.KEY_READ);
                    String fqdn = null;

                    try {
                        fqdn = registryAdapter.getStringValue(rootRegistryKey,
                                FQDN_CONFIG_KEY, "Fqdn", true);
                    } finally {
                        rootRegistryKey.close();
                    }

                    if (fqdn != null && !fqdn.isEmpty()) {
                        if (logger.isDebugEnabled()) {
                            logger.debug("Fqdn read from likewise regisry is: "
                            + fqdn);
                        }

                        spn = "HTTP/" + fqdn;
                    }
                } else {
                    //Windows
                    String computerName = idmAdapter.getComputerName();
                    Validate.notEmpty(computerName);
                    spn = "HTTP/" + computerName;
                    spn += "." + domainName;
                }
            }
            return spn;
        } catch (Exception e) {
            logger.error("Failed to get service principal name");
            throw new IdmADDomainJoinStatusException(
                    "Failed to get service principal name in creating SPN", e);
        }

    }
    @Override
    public AuthnPolicy getAuthNPolicy(String tenantName,
            IIdmServiceContext serviceContext) throws
            IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName,
                serviceContext, "getAuthPolicy")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            return tenantInfo.getAuthnPolicy();
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    @Override
    public void setAuthNPolicy(String tenantName, AuthnPolicy policy,
            IIdmServiceContext serviceContext) throws
            IDMException {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName,
                serviceContext, "getAuthPolicy")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            TenantInformation tenantInfo = findTenant(tenantName);
            ServerUtils.validateNotNullTenant(tenantInfo, tenantName);
            if (isAllAuthnTypesDisabled(policy))
            {
                throw new IllegalArgumentException("Disabling all authentication types is not allowed.");
            }
            _configStore.setAuthnTypes(tenantName, policy.IsPasswordAuthEnabled()
                    , policy.IsWindowsAuthEnabled(), policy.IsTLSClientCertAuthnEnabled()
                    , policy.IsRsaSecureIDAuthnEnabled());
            _configStore.setClientCertPolicy(tenantName, policy.getClientCertPolicy());

            RSAAgentConfig rsaConfig = policy.get_rsaAgentConfig();
            if (policy.IsRsaSecureIDAuthnEnabled()) {

                checkPasscodeAuthProviderConfigured();

                if (rsaConfig == null) {
                    //create a default tenant-wide settings
                    rsaConfig = new RSAAgentConfig();
                }
            }
            _configStore.setRsaAgentConfig(tenantName, rsaConfig);
            _tenantCache.deleteTenant(tenantName);

            ManageCrlCacheChecker();
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }
    }

    /**
     * Turn on/off crl checker thread based on current setting of authentication policy of all tenants.
     * Making sure the thread is on only if at least one tenant uses smartcard authentication.
     *
     * @throws Exception
     */
    private void ManageCrlCacheChecker() throws Exception {
        if (smartCardAuthnEnabled()) {
            //start the thread
            if (null == this._crlCacheChecker) {
                this._crlCacheChecker = new IdmCrlCachePeriodicChecker();
                this._crlCacheChecker.start();
                logger.info("Started CrlCacheChecker thread");
            }
        } else {
            //stop the thread
            if (null != this._crlCacheChecker) {
                this._crlCacheChecker = null;
                logger.info("Stopped CrlCacheChecker thread");
            }
        }
    }

    private boolean isAllAuthnTypesDisabled(AuthnPolicy policy) {
        return (!policy.IsPasswordAuthEnabled()
                && !policy.IsWindowsAuthEnabled()
                && !policy.IsTLSClientCertAuthnEnabled()
                && !policy.IsRsaSecureIDAuthnEnabled());
    }

    @Override
    public void setRSAConfig(String tenantName, RSAAgentConfig rsaAgentConfig,
            IIdmServiceContext serviceContext) throws Exception {

        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName,
                serviceContext, "setRSAConfig")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            Validate.notNull(rsaAgentConfig, "rsaAgentConfig");

            checkPasscodeAuthProviderConfigured();

            _configStore.setRsaAgentConfig(tenantName, rsaAgentConfig);
            _tenantCache.deleteTenant(tenantName);
        } catch (Exception ex) {
            logger.error(
                    String.format(
                            "Failed to set RSA agent configin tenant [%s]",
                            tenantName));

            throw ServerUtils.getRemoteException(ex);
        }

    }

    @Override
    public RSAAgentConfig getRSAConfig(String tenantName, IIdmServiceContext serviceContext)
            throws  IDMException {

        try {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");

            AuthnPolicy authnPolicy = getAuthNPolicy(tenantName, serviceContext);

            RSAAgentConfig rsaConfig = authnPolicy.get_rsaAgentConfig();

            return rsaConfig;
        } catch (Exception ex) {
            throw ServerUtils.getRemoteException(ex);
        }

    }

    @Override
    public void addRSAInstanceInfo(String tenantName, RSAAMInstanceInfo instInfo,
            IIdmServiceContext serviceContext) throws Exception {

        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName,
                serviceContext, "addRSAInstanceInfo")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            Validate.notNull(instInfo, "instInfo");

            checkPasscodeAuthProviderConfigured();

            _configStore.addRSAInstanceInfo(tenantName, instInfo);
            _tenantCache.deleteTenant(tenantName);
        } catch (Exception ex) {
            logger.error(
                    String.format(
                            "Failed to set RSA agent configin tenant [%s]",
                            tenantName));

            throw ServerUtils.getRemoteException(ex);
        }

    }

    private void checkPasscodeAuthProviderConfigured() {
        List<SessionFactoryProvider> providers = PasscodeAuthenticationServiceProvider.getInstance().getProviders(PASSCODE_PROVIDER_TYPE);
        if (providers.size() == 0)
            throw new IllegalStateException("Passcode authentication provider implementation not found");
    }

    @Override
    public void deleteRSAInstanceInfo(String tenantName, String siteID,
            IIdmServiceContext serviceContext) throws Exception {
        try (IDiagnosticsContextScope ctxt = getDiagnosticsContext(tenantName,
                serviceContext, "deleteRSAInstanceInfo")) {
            ValidateUtil.validateNotEmpty(tenantName, "Tenant name");
            Validate.notNull(siteID, "siteID");

            _configStore.deleteRSAInstanceInfo(tenantName, siteID);
            _tenantCache.deleteTenant(tenantName);
        } catch (Exception ex) {
            logger.error(
                    String.format(
                            "Failed to delete RSAAM instance config for tenant [%s]",
                            tenantName));

            throw ServerUtils.getRemoteException(ex);
        }

    }

    /**
     * Vends the IDM singleton instance
     */
    public static IdentityManager getIdmInstance() throws IDMException {
        if (idmInstance == null) {
            synchronized (identityManagerLock) {
                if (idmInstance == null) {
                    idmInstance = new IdentityManager();
                }
            }
        }
        return idmInstance;
    }
}
