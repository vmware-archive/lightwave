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

/**
 * VMware Identity Service
 *
 * VMware (LDAP) Directory Provider
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.identity.idm.server.provider.vmwdirectory;

import java.security.Principal;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import javax.security.auth.login.LoginException;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.ContainerAlreadyExistsException;
import com.vmware.identity.idm.DuplicateCertificateException;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.HashingAlgorithmType;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.MemberAlreadyExistException;
import com.vmware.identity.idm.PasswordExpiredException;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SSOImplicitGroupNames;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.VmHostData;
import com.vmware.identity.idm.server.IdentityManager;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.provider.BaseLdapProvider;
import com.vmware.identity.idm.server.provider.ISystemDomainIdentityProvider;
import com.vmware.identity.idm.server.provider.NoSuchGroupException;
import com.vmware.identity.idm.server.provider.NoSuchUserException;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.UserSet;
import com.vmware.identity.interop.ldap.AlreadyExistsLdapException;
import com.vmware.identity.interop.ldap.AttributeOrValueExistsLdapException;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.InvalidCredentialsLdapException;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.NoSuchAttributeLdapException;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.IIdmAuthStat.EventLevel;

public class VMwareDirectoryProvider extends BaseLdapProvider implements
        ISystemDomainIdentityProvider
{
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(VMwareDirectoryProvider.class);

    private static final String SVC_PRINC_QUERY_BY_SUBJECT_DN =
        // at the moment vmwSTSSubjectDN is not indexed, so vmdir gurus say
        // query by objectClass first...
        "(&(objectClass=vmwServicePrincipal)(vmwSTSSubjectDN=%s))";
    /**
     * arg1 - userPrincipalName
     */
    private static final String SVC_PRINC_QUERY_BY_USER_PRINCIPAL =
            "(&(userPrincipalName=%1$s)(objectClass=vmwServicePrincipal))";
    /**
     * arg1 - userPrincipalName
     * arg2 - sAMAccountName
     */
    private static final String SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT =
            "(&(|(userPrincipalName=%1$s)(sAMAccountName=%2$s))(objectClass=vmwServicePrincipal))";

    /**
     * arg1 - userPrincipalName
     */
    private static final String USER_PRINC_QUERY_BY_USER_PRINCIPAL =
            "(&(userPrincipalName=%1$s)(objectClass=user)(!(vmwSTSSubjectDN=*))%2$s)";
    /**
     * arg1 - userPrincipalName
     * arg2 - sAMAccountName
     */
    private static final String USER_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCT =
            "(&(|(userPrincipalName=%1$s)(sAMAccountName=%2$s))(objectClass=user)(!(vmwSTSSubjectDN=*))%3$s)";

    private static final String USER_PRINC_QUERY_BY_OBJECTSID =
            "(&(objectSid=%s)(objectClass=user)(!(vmwSTSSubjectDN=*)))";

    private static final String GROUP_PRINC_QUERY_BY_OBJECTSID =
            "(&(objectSid=%s)(objectClass=group))";

    private static final String GROUP_PRINC_QUERY_BY_ACCOUNT =
            "(&(sAMAccountName=%s)(objectClass=group))";

    private static final String CONTAINER_QUERY_BY_NAME =
            "(&(cn=%s)(objectClass=container))";

   /**
    * arg1 - userPrincipalName
    */
    private static final String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL =
            "(&(userPrincipalName=%1$s)(objectClass=user))";
    /**
     * arg1 - userPrincipalName
     * arg2 - sAMAccountName
     */
     private static final String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT =
             "(&(|(userPrincipalName=%1$s)(sAMAccountName=%2$s))(objectClass=user))";

    private static final String USER_OR_SVC_PRINC_QUERY_BY_ATTRIBUTE =
            "(&(%1$s=%2$s)(objectClass=user))";

    private static final String JIT_USER_QUERY =
            "(&(vmwSTSEntityId=%1$s)(objectClass=vmwExternalIdpUser))";

    /**
    * arg1 - userPrincipalName
    */
    private static final String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL =
            "(&(userPrincipalName=%1$s)(objectClass=user))";
    /**
    * arg1 - userPrincipalName
    * arg2 - sAMAccountName
    */
    private static final String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT =
            "(|" +
                "(&(|(userPrincipalName=%1$s)(sAMAccountName=%2$s))(objectClass=user))" +
                "(&(sAMAccountName=%2$s)(objectClass=group))" +
             ")";


    /**
    * arg1 - userPrincipalName1
    * arg2 - userPrincipalName2
    * arg3 - samAccountName1
    */
    private static final String PRINCIPAL_EXISTENCE_CHECK_CUSTOM_UPN_DOMAIN =
            "(|" +
                "(&(|(userPrincipalName=%1$s)(userPrincipalName=%2$s))(objectClass=user))" +
                "(&(sAMAccountName=%3$s)(|(objectClass=user)(objectClass=group)))" +
             ")";

    /**
    * arg1 - userPrincipalName1
    * arg2 - userPrincipalName2
    * arg3 - samAccountName1
    * arg4 - samAccountName2
    */
    private static final String PRINCIPAL_EXISTENCE_CHECK_DEFAULT_UPN_DOMAIN =
            "(|" +
                "(&(|(userPrincipalName=%1$s)(userPrincipalName=%2$s))(objectClass=user))" +
                "(&(|(sAMAccountName=%3$s)(sAMAccountName=%4$s))(|(objectClass=user)(objectClass=group)))" +
             ")";


    private static final String FSP_PRINC_QUERY_BY_EXTERNALID =
            "(&(%s)(objectClass=foreignSecurityPrincipal))";

    // This is to get all users under Users CN
    // We need this do to substring match on users and groups
    private static final String USER_ALL_PRINC_QUERY =
            "(&(objectClass=user)(!(objectClass=computer))(!(vmwSTSSubjectDN=*)))";

    private static final String USER_ALL_BY_ACCOUNT_NAME =
            "(&(objectClass=user)(!(objectClass=computer))(|(sAMAccountName=%1$s*)(sn=%1$s*)(givenName=%1$s*)(cn=%1$s*)(mail=%1$s*)(userPrincipalName=%1$s*)))";

    private static final String USER_ALL_QUERY =
            "(&(objectClass=user)(!(objectClass=computer)))";

    // This is to get all solution users
    // We need this do to substring match on users and groups
    private static final String SVC_ALL_PRINC_QUERY =
            "(objectClass=vmwServicePrincipal)";

    // This is to get all group
    // We need this do to substring match on users and groups
    private static final String GROUP_ALL_PRINC_QUERY =
            "(objectClass=group)";

    private static final String GROUP_ALL_BY_ACCOUNT_NAME =
            "(&(objectClass=group)(|(sAMAccountName=%1$s*)(cn=%1$s*)))";

    private static final String GROUP_ALL_QUERY =
            "(objectClass=group)";

    // Filter to get all groups that has a specific member directly
    private static final String GROUP_DIRECT_MEMBER_QUERY =
            "(&(objectClass=group)(member=%s))";

    private static final String ATTR_NAME_OBJECTCLASS = "objectclass";
    private static final String ATTR_NAME_USER = "user";
    private static final String ATTR_NAME_JIT_USER = "vmwExternalIdpUser";
    private static final String ATTR_JIT_USER_ID = "vmwSTSExternalIdpUserId";
    private static final String ATTR_EXTERNAL_IDP_ID = "vmwSTSEntityId";
    private static final String ATTR_NAME_GROUP = "group";
    private static final String ATTR_NAME_CONTAINER = "container";
    private static final String ATTR_NAME_SERVICE =
            "vmwServicePrincipal";
    private static final String ATTR_NAME_GROUP_NAME = "name";
    private static final String ATTR_NAME_MEMBER = "member";
    private static final String ATTR_NAME_EXTERNAL_OBJECT_ID = "externalObjectId";
    private static final String ATTR_NAME_MEMBEROF = "memberOf";
    private static final String ATTR_NAME_CN = "cn";
    private static final String ATTR_NAME_ACCOUNT = "sAMAccountName";
    private static final String ATTR_NAME_SUBJECTDN = "vmwSTSSubjectDN";
    private static final String ATTR_NAME_CERT = "userCertificate";
    private static final String ATTR_NAME_ACCOUNT_FLAGS = "userAccountControl";
    private static final String ATTR_LAST_NAME = "sn";
    private static final String ATTR_FIRST_NAME = "givenName";
    private static final String ATTR_EMAIL_ADDRESS = "mail";
    private static final String ATTR_USER_PASSWORD = "userPassword";
    private static final String ATTR_PWD_LAST_SET = "pwdLastSet";
    private static final String ATTR_DESCRIPTION = "description";
    private static final String ATTR_SVC_DESCRIPTION = "description";
    private static final String ATTR_SUBJECT_TYPE = "subjectType";
    private static final String ATTR_USER_PRINCIPAL_NAME = "userPrincipalName";
    private static final String ATTR_OBJECT_GUID = "objectGUID";
    private static final String ATTR_SITE = "msDS-SiteName";
    private static final String ATTR_NAME_PASSWORD_HISTORY_COUNT =
            "vmwPasswordProhibitedPreviousCount";
    private static final String ATTR_NAME_PASSWORD_LIFETIME_DAYS =
            "vmwPasswordLifetimeDays";
    private static final String ATTR_NAME_PASSWORD_MAX_LENGTH =
            "vmwPasswordMaxLength";
    private static final String ATTR_NAME_PASSWORD_MIN_LENGTH =
            "vmwPasswordMinLength";
    private static final String ATTR_NAME_PASSWORD_ALPHABET_COUNT =
            "vmwPasswordMinAlphabeticCount";
    private static final String ATTR_NAME_PASSWORD_MIN_UPPERCASE_COUNT =
            "vmwPasswordMinUpperCaseCount";
    private static final String ATTR_NAME_PASSWORD_MIN_LOWERCASE_COUNT =
            "vmwPasswordMinLowerCaseCount";
    private static final String ATTR_NAME_PASSWORD_MIN_NUMBER_COUNT =
            "vmwPasswordMinNumericCount";
    private static final String ATTR_NAME_PASSWORD_MIN_SPECIAL_CHAR_COUNT =
            "vmwPasswordMinSpecialCharCount";
    private static final String ATTR_NAME_MAX_IDENTICAL_ADJACENT_CHAR_COUNT =
            "vmwPasswordMaxIdenticalAdjacentChars";
    private static final String ATTR_NAME_PASSWORD_MAX_FAILED_ATTEMPTS =
            "vmwPasswordChangeMaxFailedAttempts";
    private static final String ATTR_NAME_PASSWORD_FAILED_INTERVAL_SECS =
            "vmwPasswordChangeFailedAttemptIntervalSec";
    private static final String ATTR_NAME_PASSWORD_CHANGE_AUTO_UNLOCK_INTERVAL_SECS =
            "vmwPasswordChangeAutoUnlockIntervalSec";
    private static final String ATTR_NAME_OBJECTSID = "objectSid";
    private static final String ATTR_NAME_PASSWORD_SCHEMA =
            "passwordHashScheme";

    static final String SPECIAL_NAME_TO_REQUEST_USERPASSWORD = "-";
    static final String ATTR_NAME_USERPASSWORD = "userPassword";

    private static final int USER_ACCT_DISABLED_FLAG = 0x0002;
    private static final int USER_ACCT_LOCKED_FLAG = 0x0010;
    private static final int USER_ACCT_PASSWORD_EXPIRED_FLAG = 0x00800000;

    private static final String ATTR_OBJECT_ID_PREFIX = "externalObjectId=";

    private static String[] GROUP_ALL_ATTRIBUTES = { ATTR_NAME_CN, ATTR_NAME_MEMBER, ATTR_DESCRIPTION, ATTR_NAME_OBJECTSID };
    private static String[] USER_ALL_ATTRIBUTES = { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_FIRST_NAME, ATTR_LAST_NAME, ATTR_EMAIL_ADDRESS, ATTR_DESCRIPTION, ATTR_NAME_ACCOUNT_FLAGS,  ATTR_NAME_OBJECTSID };
    private static String[] GROUPS_BY_CRITERIA_ATTRIBUTES = { ATTR_NAME_CN, ATTR_DESCRIPTION };
    private static String[] GROUPS_BY_CRITERIA_FOR_NAME_ATTRIBUTES = { ATTR_NAME_CN, ATTR_DESCRIPTION };
    private static String[] USERS_BY_CRITERIA_ATTRIBUTES = { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_DESCRIPTION };
    private static String[] USERS_BY_CRITERIA_FOR_NAME_ATTRIBUTES = { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_FIRST_NAME, ATTR_LAST_NAME, ATTR_EMAIL_ADDRESS };

    private static Matcher containsMatcher = null;
    private static Matcher startsWithMatcher = null;

    static {
        ensureAttributeSubset(GROUPS_BY_CRITERIA_ATTRIBUTES, GROUP_ALL_ATTRIBUTES);
        ensureAttributeSubset(GROUPS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, GROUP_ALL_ATTRIBUTES);
        ensureAttributeSubset(USERS_BY_CRITERIA_ATTRIBUTES, USER_ALL_ATTRIBUTES);
        ensureAttributeSubset(USERS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, USER_ALL_ATTRIBUTES);
    }

    private final Set<String> _specialAttributes;

    private final Group _everyoneGroup;

    private String _domainId = null;

    private final boolean _isSystemDomainProvider;

    private final String tenantName;

    public VMwareDirectoryProvider(String tenantName, IIdentityStoreData store, boolean isSystemDomainProvider)
    {
        super(tenantName, store);

        _isSystemDomainProvider = isSystemDomainProvider;

        Validate.isTrue(
                getStoreDataEx().getProviderType() == IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                "IIdentityStoreData must represent a store of "
                        + "'IDENTITY_STORE_TYPE_VMWARE_DIRECTORY' type.");

        _specialAttributes = new HashSet<String>();

        _specialAttributes.add(ATTR_SUBJECT_TYPE);
        this.tenantName = tenantName;

        this._everyoneGroup = new Group(
            new PrincipalId(
                SSOImplicitGroupNames.getEveryoneGroupName(),
                this.getDomain()),
            ServerUtils.getPrincipalAliasId(SSOImplicitGroupNames.getEveryoneGroupName(), this.getAlias()),
            null/*sid*/,
            new GroupDetail(""));
    }

    @Override
    public String getAlias()
    {
        // standard system domain provider does not have an alias
        return null;
    }

    @Override
    public PrincipalId authenticate(PrincipalId principal, String password)
            throws LoginException
    {
        ValidateUtil.validateNotNull(principal, "principal");

        IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
                DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
                ActivityKind.AUTHENTICATE, EventLevel.INFO, principal);
        idmAuthStatRecorder.start();

        principal = this.normalizeAliasInPrincipal(principal);
        InvalidCredentialsLdapException srpEx = null;
        try {
            ILdapConnectionEx connection = null;
            try {
                connection = this.getConnection(principal.getUPN(), password, AuthenticationType.SRP, false);
            } catch (InvalidCredentialsLdapException ex) {
                logger.warn("Failed to authenticate using SRP binding", ex);
                srpEx = ex;
            } finally {
                if (connection != null) {
                    connection.close();
                    connection = null;
                }
            }
            if(srpEx != null){
                String userDn = getUserDn(principal, true);
                if(userDn != null){
                    try {
                        logger.warn("The user is not SRP-enabled. Attempting to authenticate using simple bind.");
                        connection = this.getConnection(userDn, password, AuthenticationType.PASSWORD, false);
                    } finally {
                        if (connection != null) {
                            connection.close();
                            connection = null;
                        }
                    }
                } else {
                    logger.warn("The user is SRP-enabled and failed to authenticate.");
                    throw srpEx;
                }
            }
        } catch (Exception ex) {
            final LoginException loginException =
                    new LoginException("Login failed");
            loginException.initCause(ex);
            throw loginException;
        }

        idmAuthStatRecorder.end();

        return principal;
    }

    @Override
    public byte[] getUserHashedPassword(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        byte[] userPassword = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            String[] attrNames = { SPECIAL_NAME_TO_REQUEST_USERPASSWORD };

            String filter = buildQueryByUserFilter(id);

            String domainName = getDomain();

            // Search from Users by default
            String searchBaseDn = getUsersDN(domainName);

            ILdapMessage message =
                    pooledConnection.getConnection().search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);
            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length != 1)
                {
                    // Use doesn't exist or multiple user same name
                    throw new InvalidPrincipalException(
                            String.format(
                                    "user %s doesn't exist or multiple users with same name",
                                    id.getName()), id.getUPN());
                }

                userPassword =
                        getBinaryBlobValue(entries[0]
                                .getAttributeValues(ATTR_USER_PASSWORD));
            } finally
            {
                message.close();
            }
        }

        return userPassword;
    }

    @Override
    public PersonUser findUser(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");
        PersonUser user = null;

        String domainName = getDomain();
        if (!this.belongsToThisIdentityProvider(id.getDomain()))
        {//   For the case of same username exist on system provider as well as
         //   [non-system provider (AD/LDAP) and/or External IDP]:
         //   mismatched domain --> user not found.
            return null;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            long pwdLifeTime = getPwdLifeTime();

            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_FIRST_NAME, ATTR_LAST_NAME, ATTR_EMAIL_ADDRESS,
                            ATTR_DESCRIPTION, ATTR_NAME_ACCOUNT_FLAGS,
                            ATTR_NAME_OBJECTSID, ATTR_PWD_LAST_SET };

            String filter = buildQueryByUserFilter(id);

            // Search from Users by default
            String searchBaseDn = getUsersDN(domainName);

            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0)
                {
                    // According to admin interface, we should return null
                    // if not found

                    // we may want to eventually throw 'InvalidPrincipalException'
                    // equivelent to non-lotus provider does (this also require tests change)
                    return null;
                } else if (entries.length != 1)
                {
                    throw new IllegalStateException(
                            "More than one object was found");
                }

                String accountName =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_ACCOUNT));

                long pwdLastSet =
                        getOptionalLongValue(
                                entries[0]
                                        .getAttributeValues(ATTR_PWD_LAST_SET),
                                PersonDetail.UNSPECIFIED_TS_VALUE);

                String resultObjectSid =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_OBJECTSID));

                String firstName =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_FIRST_NAME));

                String lastName =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_LAST_NAME));

                String email =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_EMAIL_ADDRESS));

                String upn =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                String description =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_DESCRIPTION));

                PersonDetail detail =
                        new PersonDetail.Builder()
                                .firstName(firstName)
                                .lastName(lastName)
                                .emailAddress(email)
                                .userPrincipalName(upn)
                                .description(description)
                                .pwdLastSet(
                                        pwdLastSet,
                                        IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY)
                                .pwdLifeTime(
                                        pwdLifeTime,
                                        IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY)
                                .build();

                int currentFlag =
                        getOptionalIntegerValue(
                                entries[0]
                                        .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                0);

                boolean locked = ((currentFlag & USER_ACCT_LOCKED_FLAG) != 0);
                boolean disabled =
                        ((currentFlag & USER_ACCT_DISABLED_FLAG) != 0);

                // Note: for system domain, no domain alias, therefore, we are
                // not setting the alias principalid here.
                // Same reason applies to system domain's user, solution user
                // and group.
                user =
                        new PersonUser(this.getPrincipalId(upn, accountName, domainName), this.getPrincipalAliasId(accountName), resultObjectSid, detail,
                                disabled, locked);
            } finally
            {
                message.close();
            }
        }

        return user;
    }

    @Override
    public PersonUser findUserByObjectId(String userObjectSid) throws Exception
    {
        ValidateUtil.validateNotEmpty(userObjectSid, "User ObjectSid");

        PersonUser user = null;
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            long pwdLifeTime = getPwdLifeTime();

            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_FIRST_NAME, ATTR_LAST_NAME,
                            ATTR_EMAIL_ADDRESS, ATTR_DESCRIPTION,
                            ATTR_NAME_ACCOUNT_FLAGS, ATTR_NAME_OBJECTSID,
                            ATTR_PWD_LAST_SET };

            String filter =
                    String.format(USER_PRINC_QUERY_BY_OBJECTSID, LdapFilterString.encode(userObjectSid));

            String searchBaseDn = this.getStoreDataEx().getUserBaseDn();

            message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length != 1)
            {
                // Use doesn't exist or multiple user same name
                throw new InvalidPrincipalException(
                        String.format(
                                "user with objectSid %s doesn't exist or multiple users with same name",
                                userObjectSid), userObjectSid);
            }

            long pwdLastSet =
                    getOptionalLongValue(
                            entries[0].getAttributeValues(ATTR_PWD_LAST_SET),
                            PersonDetail.UNSPECIFIED_TS_VALUE);

            String resultObjectSid =
                    getStringValue(entries[0]
                            .getAttributeValues(ATTR_NAME_OBJECTSID));

            assert resultObjectSid.equals(userObjectSid);

            String accountName =
                    getStringValue(entries[0]
                            .getAttributeValues(ATTR_NAME_ACCOUNT));

            String firstName =
                    getOptionalStringValue(entries[0]
                            .getAttributeValues(ATTR_FIRST_NAME));

            String lastName =
                    getOptionalStringValue(entries[0]
                            .getAttributeValues(ATTR_LAST_NAME));

            String email =
                    getOptionalStringValue(entries[0]
                            .getAttributeValues(ATTR_EMAIL_ADDRESS));

            String upn =
                    getOptionalStringValue(entries[0]
                            .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

            String description =
                    getOptionalStringValue(entries[0]
                            .getAttributeValues(ATTR_DESCRIPTION));

            PersonDetail detail =
                    new PersonDetail.Builder()
                            .firstName(firstName)
                            .lastName(lastName)
                            .description(description)
                            .emailAddress(email)
                            .userPrincipalName(upn)
                            .pwdLastSet(
                                    pwdLastSet,
                                    IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY)
                            .pwdLifeTime(
                                    pwdLifeTime,
                                    IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY)
                            .build();

            int currentFlag =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                            0);

            boolean locked = ((currentFlag & USER_ACCT_LOCKED_FLAG) != 0);
            boolean disabled = ((currentFlag & USER_ACCT_DISABLED_FLAG) != 0);

            // Note: for system domain, no domain alias, therefore, we are
            // not setting the alias principalid here.
            // Same reason applies to system domain's user, solution user
            // and group.
            user =
                    new PersonUser(this.getPrincipalId(upn, accountName, getDomain()),
                            this.getPrincipalAliasId(accountName), resultObjectSid, detail, disabled, locked);
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return user;
    }

    @Override
    public Set<PersonUser> findUsers(String searchString, String searchDomainName, int limit) throws Exception
    {
        return findUsersInternal(USER_ALL_PRINC_QUERY, searchDomainName, limit, searchString, USERS_BY_CRITERIA_ATTRIBUTES, getStringContainsMatcher());
    }

    @Override
    public Set<PersonUser> findUsersByName(String searchString, String searchDomainName, int limit) throws Exception
    {
        String filter = createSearchFilter(USER_ALL_QUERY, USER_ALL_BY_ACCOUNT_NAME, searchString);
        return findUsersInternal(filter, searchDomainName, limit, searchString, USERS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, getStringStartsWithMatcher());
    }

    private Set<PersonUser> findUsersInternal(String groupFilter, String searchDomainName, int limit, String searchString, String[] userAttributes, Matcher matcher) throws Exception
    {
        Set<PersonUser> users = new HashSet<PersonUser>();
        PersonUser user = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_DESCRIPTION, ATTR_FIRST_NAME,
                            ATTR_LAST_NAME, ATTR_EMAIL_ADDRESS,
                            ATTR_NAME_ACCOUNT_FLAGS };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.

            String domainName = this.getDomain();

            // Search from domain dn by default
            // we probably need to optimize here to search from a
            // more narrowed-down the search base dn.
            // We can add search on description as well. Right now only search
            // for group names.
            String searchBaseDn = getUsersDN(domainName);
            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            groupFilter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                // This shouldn't happen because we should at least have
                // Administrator user in the Castle 2.0 system.
                // However, for completeness of this api interface, if somehow
                // We couldn't find the user, we should always return an empty
                // set based on admin api interface. (NOT to throw some exception.)
                if (entries == null || entries.length == 0)
                {
                    return users;
                }

                int i = 0;
                for (ILdapEntry entry : entries)
                {

                    if (matcher.matches(entry, userAttributes, searchString))
                    {

                        String accountName =
                                getStringValue(entry
                                        .getAttributeValues(ATTR_NAME_ACCOUNT));

                        String upn =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                        String description =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_DESCRIPTION));

                        String firstName =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_FIRST_NAME));

                        String lastName =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_LAST_NAME));

                        String email =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_EMAIL_ADDRESS));

                        PrincipalId id =
                                this.getPrincipalId(upn, accountName, domainName);

                        PersonDetail detail =
                                new PersonDetail.Builder().firstName(firstName)
                                        .lastName(lastName).emailAddress(email)
                                        .userPrincipalName(upn)
                                        .description(description).build(); // leave pwdLastSet as default for group query

                        int flag =
                                getOptionalIntegerValue(
                                        entry.getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                        0);

                        boolean locked = ((flag & USER_ACCT_LOCKED_FLAG) != 0);
                        boolean disabled =
                                ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                        user = new PersonUser(id, this.getPrincipalAliasId(accountName), null/*sid*/, detail, disabled, locked);

                        if (i++ < limit || limit <= 0)
                        {
                            users.add(user);
                        }
                        else break;
                    }
                }
            } finally
            {
                message.close();
            }
        }

        return users;
    }

    @Override
    public Set<PersonUser> findUsersInGroup(PrincipalId id, String searchString, int limit) throws Exception
    {
        return findUsersInGroupInternal(id, limit, searchString, USERS_BY_CRITERIA_ATTRIBUTES, getStringContainsMatcher());
    }

    @Override
    public Set<PersonUser> findUsersByNameInGroup(PrincipalId id, String searchString, int limit) throws Exception
    {
        return findUsersInGroupInternal(id, limit, searchString, USERS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, getStringStartsWithMatcher());
    }

    private Set<PersonUser> findUsersInGroupInternal(PrincipalId id, int limit, String searchString, String[] userAttributes, Matcher matcher) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");
        Set<PersonUser> users = new HashSet<PersonUser>();

        if (limit == 0) {
            // Short circuit since they're asking for a list of nothing anyway
            return users;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_NAME_MEMBER };

            String filter = buildQueryByGroupFilter(id.getName());

            String domainName = getDomain();

            // Search from domain dn by default
            String searchBaseDn = getDomainDN(domainName);

            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                // Check if multiple group same name but different level
                // We probably should check the searchbaseDN
                if (entries == null || entries.length != 1)
                {
                    throw new InvalidPrincipalException(
                            String.format(
                                    "group %s doesn't exist or multiple groups same name",
                                    id.getName()),
                            ServerUtils.getUpn(id));
                }

                // We found the group
                LdapValue[] values =
                        entries[0].getAttributeValues(ATTR_NAME_MEMBER);

                if (values != null && values.length > 0)
                {
                    for (LdapValue val : values)
                    {
                        int numUsersToRet = 0;
                        if (limit <= 0 || numUsersToRet < limit)
                        {
                            String userId = val.toString();
                            PersonUser user = null;

                            // Currently support FSP in AD/Openldap provider
                            if (userId.startsWith(ATTR_OBJECT_ID_PREFIX))
                            {
                                final String upn = null;
                                // Create a PersonUser directly using the 'val' retrived from group's member attribute
                                // such object shall be resolved by the caller.
                                user =
                                        new PersonUser(
                                                this.getPrincipalId(upn, userId, domainName),
                                                this.getPrincipalAliasId(userId),
                                                null/*sid*/,
                                                new PersonDetail.Builder()
                                                        .firstName("dummyFirstName")
                                                        .lastName("dummyLastName")
                                                        .emailAddress(null)
                                                        .userPrincipalName(upn)
                                                        .description("Comments")
                                                        .build(), true, true);
                                users.add(user);
                            } else
                            {
                                user =
                                        findUser(connection, domainName,
                                                val, searchString, userAttributes, matcher);
                                if (user != null)
                                {
                                    users.add(user);
                                }
                            }
                            numUsersToRet++;
                        }
                    }
                }
            } finally
            {
                message.close();
            }
        }

        return users;
    }

    @Override
    public SearchResult find(String searchString, String domainName, int limit) throws Exception
    {
        Set<PersonUser> personUsers = null;
        Set<SolutionUser> solutionUsers = null;
        Set<Group> groups = null;

        solutionUsers = this.findServicePrincipals(searchString);
        if (limit>=0 && limit <= solutionUsers.size())
        {//return solution users as result since limit is reached
           List<SolutionUser> all = new ArrayList<SolutionUser>(solutionUsers);
           solutionUsers = new HashSet<SolutionUser>(all.subList(0,limit));
        }
        else
        {
           int limitNew = limit<0? -1: (limit - ((solutionUsers != null) ? solutionUsers.size() : 0));
           int limitUsers = limitNew ==-1? -1: (limitNew/2+limitNew%2);
           personUsers = this.findUsers(searchString, domainName, limitUsers);
           if (limitNew != -1)
           {
              limitNew = limitNew-((personUsers!=null)? personUsers.size():0);
           }
           if (limitNew != 0)
           {//find groups only if needed
              groups = this.findGroups(searchString, domainName, limitNew);
           }
        }

        return new SearchResult(personUsers, solutionUsers, groups);
    }

    @Override
    public SearchResult findByName(String searchString, String domainName, int limit) throws Exception
    {
        Set<PersonUser> personUsers = null;
        Set<SolutionUser> solutionUsers = null;
        Set<Group> groups = null;

        solutionUsers = this.findServicePrincipals(searchString);
        int limit_new = limit - ((solutionUsers != null) ? solutionUsers.size() : 0);
        personUsers = this.findUsersByName(searchString, domainName, limit_new/2);
        groups = this.findGroupsByName(searchString, domainName, limit_new/2+limit_new%2);

        return new SearchResult(personUsers, solutionUsers, groups);
    }

    @Override
    public Set<PersonUser> findDisabledUsers(String searchString, int limit)
            throws Exception
    {
        Set<PersonUser> users = new HashSet<PersonUser>();
        PersonUser user = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_DESCRIPTION, ATTR_FIRST_NAME,
                            ATTR_LAST_NAME, ATTR_EMAIL_ADDRESS,
                            ATTR_NAME_ACCOUNT_FLAGS };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.
            String filter = USER_ALL_PRINC_QUERY;

            String domainName = getDomain();

            // Search from domain dn by default
            //  we probably need to optimize here to search from a
            // more narrowed-down the search base dn.
            // we can add search on description as well. Right now only search
            // for group names.
            String searchBaseDn = getUsersDN(domainName);
            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                // return empty set if not found
                if (entries == null || entries.length == 0)
                {
                    return users;
                }

                int i = 0;
                for (ILdapEntry entry : entries)
                {

                    String accountName =
                            getStringValue(entry
                                    .getAttributeValues(ATTR_NAME_ACCOUNT));

                    String upn =
                            getOptionalStringValue(entry
                                    .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                    String description =
                            getOptionalStringValue(entry
                                    .getAttributeValues(ATTR_DESCRIPTION));

                    int flag =
                            getOptionalIntegerValue(
                                    entry.getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                    0);

                    boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                    // Last bit set if disabled
                    if ((containsSearchString(accountName, searchString) ||
                         containsSearchString(upn, searchString) ||
                         containsSearchString(description, searchString))
                        &&
                        disabled)
                    {
                        String firstName =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_FIRST_NAME));

                        String lastName =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_LAST_NAME));

                        String email =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_EMAIL_ADDRESS));

                        PrincipalId id =
                                this.getPrincipalId(upn, accountName, domainName);

                        PersonDetail detail =
                                new PersonDetail.Builder().firstName(firstName)
                                        .lastName(lastName).emailAddress(email)
                                        .userPrincipalName(upn)
                                        .description(description).build(); //leave pwdLastSet as default

                        boolean locked = ((flag & USER_ACCT_LOCKED_FLAG) == 1);

                        user = new PersonUser(id, this.getPrincipalAliasId(accountName), null/*sid*/, detail, disabled, locked);
                        if (i++ < limit || limit <= 0)
                        {
                            users.add(user);
                        }
                        else break;
                    }
                }
            } finally
            {
                message.close();
            }
        }

        return users;
    }

    @Override
    public Set<PersonUser> findLockedUsers(String searchString, int limit)
            throws Exception
    {

        ILdapMessage message = null;
        PersonUser user = null;
        Set<PersonUser> users = new HashSet<PersonUser>();

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_DESCRIPTION, ATTR_FIRST_NAME,
                            ATTR_LAST_NAME, ATTR_EMAIL_ADDRESS,
                            ATTR_NAME_ACCOUNT_FLAGS };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.
            String filter = USER_ALL_PRINC_QUERY;

            String domainName = getDomain();

            // Search from domain dn by default
            // we probably need to optimize here to search from a
            // more narrowed-down the search base dn.
            String searchBaseDn = getUsersDN(domainName);
            message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                // Based on existing admin interface return empty set if not
                // found
                return users;
            }

            int i = 0;
            for (ILdapEntry entry : entries)
            {

                String accountName =
                        getStringValue(entry
                                .getAttributeValues(ATTR_NAME_ACCOUNT));

                String upn =
                        getOptionalStringValue(entry
                                .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                String description =
                        getOptionalStringValue(entry
                                .getAttributeValues(ATTR_DESCRIPTION));

                int flag =
                        getOptionalIntegerValue(
                                entry.getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                0);

                boolean locked = ((flag & USER_ACCT_LOCKED_FLAG) != 0);

                if ((
                        containsSearchString(accountName, searchString) ||
                        containsSearchString(upn, searchString) ||
                        containsSearchString(description, searchString)
                    ) &&
                    locked)
                {
                    String firstName =
                            getOptionalStringValue(entry
                                    .getAttributeValues(ATTR_FIRST_NAME));

                    String lastName =
                            getOptionalStringValue(entry
                                    .getAttributeValues(ATTR_LAST_NAME));

                    String email =
                            getOptionalStringValue(entry
                                    .getAttributeValues(ATTR_EMAIL_ADDRESS));

                    PrincipalId id =
                            this.getPrincipalId(upn, accountName, domainName);
                    PersonDetail detail =
                            new PersonDetail.Builder().firstName(firstName)
                                    .lastName(lastName).emailAddress(email)
                                    .userPrincipalName(upn)
                                    .description(description).build();

                    boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                    user = new PersonUser(id, this.getPrincipalAliasId(accountName), null/*sid*/, detail, disabled, locked);

                    if (i++ < limit || limit <= 0)
                    {
                        users.add(user);
                    }
                    else break;
                }
            }
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return users;
    }

    @Override
    public Set<SolutionUser> findDisabledServicePrincipals(String searchString)
            throws Exception
    {
        return findDisabledServicePrincipalsInContainer(searchString,
                getServicePrincipalsDN(getDomain()), true);
    }

    @Override
    public Set<SolutionUser> findDisabledServicePrincipalsInExternalTenant(
            String searchString) throws Exception
    {
        return findDisabledServicePrincipalsInContainer(searchString,
                getTenantSearchBaseRootDN(), true);
    }

    private Set<SolutionUser> findDisabledServicePrincipalsInContainer(
            String searchString, String containerDn, boolean isExternal) throws Exception
    {
        Set<SolutionUser> solutions = new HashSet<SolutionUser>();
        SolutionUser solution = null;
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            ValidateUtil.validateNotEmpty(containerDn, "container Dn");

            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_SVC_DESCRIPTION,
                            ATTR_NAME_ACCOUNT_FLAGS, ATTR_NAME_CERT };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.
            String filter = SVC_ALL_PRINC_QUERY;

            String domainName = getDomain();

            message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            // return empty set if nothing to find
            if (entries == null || entries.length == 0)
            {
                return solutions;
            }

            for (ILdapEntry entry : entries)
            {

                String accountName =
                        getStringValue(entry
                                .getAttributeValues(ATTR_NAME_ACCOUNT));

                String upn =
                        getOptionalStringValue(entry
                                .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                String description =
                        getOptionalStringValue(entry
                                .getAttributeValues(ATTR_SVC_DESCRIPTION));

                int flag =
                        getOptionalIntegerValue(
                                entry.getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                0);

                boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                if ((containsSearchString(accountName, searchString) ||
                     containsSearchString(upn, searchString) ||
                     containsSearchString( description, searchString)
                    ) &&
                    disabled)
                {
                    LdapValue[] certValue =
                            entry.getAttributeValues(ATTR_NAME_CERT);

                    if (certValue == null || certValue[0] == null)
                    {
                        throw new IllegalStateException(
                                    "Certificate content should exist.");
                    }

                    PrincipalId id = this.getPrincipalId(upn, accountName, domainName);
                    X509Certificate cert = ServerUtils.getCertificateValue(certValue);

                    SolutionDetail detail =
                                new SolutionDetail(cert, description);

                    solution =
                                new SolutionUser(id, this.getPrincipalAliasId(accountName), null, detail,
                                        disabled, isExternal);

                    solutions.add(solution);
                }
            }
        } finally
        {

            if (null != message)
            {
                message.close();
            }
        }

        return solutions;
    }

    @Override
    public boolean IsActive(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        int accountFlags = retrieveUserAccountFlags(id);
        return IsAccountActive(accountFlags);
    }
    private static boolean IsAccountActive(int accountFlags)
    {
        return ((accountFlags & USER_ACCT_DISABLED_FLAG) == 0);
    }

    @Override
    public void checkUserAccountFlags(PrincipalId principalId) throws IDMException
    {
        ValidateUtil.validateNotNull(principalId, "principalId");

        int accountFlags = retrieveUserAccountFlags(principalId);
        if ((accountFlags & USER_ACCT_LOCKED_FLAG) == USER_ACCT_LOCKED_FLAG)
        {
            throw new UserAccountLockedException(String.format(
                    "User account locked: %s", principalId));
        } else if ((accountFlags & USER_ACCT_PASSWORD_EXPIRED_FLAG) == USER_ACCT_PASSWORD_EXPIRED_FLAG)
        {
            throw new PasswordExpiredException(String.format(
                    "User account expired: %s", principalId));
        }
    }

    private int retrieveUserAccountFlags(PrincipalId id) throws IDMException
    {
        int accountFlags = 0;

        try
        {
            // look in container cn=users
            accountFlags =
                    retrieveUserAccountFlagsInContainer(id, this
                            .getStoreDataEx().getUserBaseDn());
        } catch (InvalidPrincipalException ex)
        {
            // look in container cn=Ldus
            accountFlags =
                    retrieveUserAccountFlagsInContainer(id, getServicePrincipalsDN(getDomain()));
        }

        return accountFlags;
    }

    private int retrieveUserAccountFlagsInContainer(PrincipalId id,
            String containerDn) throws IDMException
    {
        int accountFlags = 0;
        PooledLdapConnection pooledConnection = null;
        ILdapConnectionEx connection = null;
        ILdapMessage message = null;

        try
        {
            pooledConnection = borrowConnection();
            connection = pooledConnection.getConnection();
        } catch (Exception ex)
        {
            throw new IDMException("Failed to establish server connection", ex);
        }

        try
        {
            String filter = buildQueryByUserORSrvFilter(id);

            String attributes[] = { ATTR_NAME_ACCOUNT_FLAGS };

            message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attributes, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                throw new InvalidPrincipalException(String.format(
                        "No such principal %s was found in %s", id.getName(),
                        id.getDomain()), id.getUPN());
            }

            if (entries.length != 1)
            {
                throw new IllegalStateException(
                        "Internal error : duplicate entries were found");
            }

            accountFlags =
                    getOptionalIntegerValue(
                            entries[0].getAttributeValues(attributes[0]), 0);
        } catch (NoSuchObjectLdapException e)
        {
            throw new InvalidPrincipalException(String.format(
                    "errorCode; %d; %s", e.getErrorCode(), e.getMessage()), id.getUPN());
        } finally
        {
            if (null != message)
            {
                message.close();
            }
            if (pooledConnection != null) {
                pooledConnection.close();
            }
        }

        return accountFlags;
    }

    @Override
    public Group findGroup(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        Group group = null;

        String domainName = getDomain();
        if (!this.isSameDomainUpn(id))
        {//   For the case of same group exist on system provider as well as
         //   [non-system provider (AD/LDAP) and/or External IDP]:
         //   mismatched domain --> group not found.
            return null;
        }

        //check Everyone group
        if ( ( id.getName().equalsIgnoreCase(this._everyoneGroup.getName()) ) &&
             (this.isSameDomainUpn(id)) )
        {
           return this._everyoneGroup;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_CN, ATTR_DESCRIPTION,
                            ATTR_NAME_OBJECTSID };

            String filter = buildQueryByGroupFilter(id.getName());


            // Search from domain dn by default
            String searchBaseDn = getDomainDN(domainName);

            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0)
                {
                    return null;
                }
                else if (entries.length != 1)
                {
                    throw new IllegalStateException("entries.length > 1");
                }

                String groupName =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_CN));

                String resultObjectSid =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_OBJECTSID));

                String description =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_DESCRIPTION));

                GroupDetail detail = new GroupDetail(description);

                group = new Group(new PrincipalId(groupName, domainName), this.getPrincipalAliasId(groupName), resultObjectSid, detail);
            } finally
            {
                message.close();
            }
        }

        return group;
    }

    @Override
    public Group findGroupByObjectId(String groupObjectSid) throws Exception
    {
        Group group = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_CN, ATTR_DESCRIPTION,
                            ATTR_NAME_OBJECTSID };

            String filter =
                    String.format(GROUP_PRINC_QUERY_BY_OBJECTSID,
                        LdapFilterString.encode(groupObjectSid));

            String domainName = getDomain();

            // Search from domain dn by default
            String searchBaseDn = getDomainDN(domainName);

            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0)
                {
                    // According to admin interface, we should return null
                    // if not found
                    return null;
                } else if (entries.length != 1)
                {
                    throw new IllegalStateException("entries.length > 1");
                }

                String resultObjectSid =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_OBJECTSID));

                String accountName =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_CN));

                String description =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_DESCRIPTION));

                GroupDetail detail = new GroupDetail(description);

                group =
                        new Group(new PrincipalId(accountName, getDomain()),
                                this.getPrincipalAliasId(accountName),
                                resultObjectSid, detail);
            } finally
            {
                message.close();
            }
        }

        return group;
    }

    @Override
    public Set<Group> findGroups(String searchString, String searchDomainName, int limit) throws Exception
    {
        return findGroupsInternal(GROUP_ALL_PRINC_QUERY, searchDomainName, limit, searchString, GROUPS_BY_CRITERIA_ATTRIBUTES, getStringContainsMatcher());
    }

    @Override
    public Set<Group> findGroupsByName(String searchString, String searchDomainName, int limit) throws Exception
    {
        String filter = createSearchFilter(GROUP_ALL_QUERY, GROUP_ALL_BY_ACCOUNT_NAME, searchString);
        return findGroupsInternal(filter, searchDomainName, limit, searchString, GROUPS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, getStringStartsWithMatcher());
    }

    private Set<Group> findGroupsInternal(String groupFilter, String searchDomainName, int limit, String searchString, String[] groupAttributes, Matcher matcher) throws Exception
    {
        Set<Group> groups = new HashSet<Group>();
        Group group = null;
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.
            // For now, we retrieve all groups and match the criteria

            String domainName = this.getDomain();

            // Search from domain dn by default
            // we probably need to optimize here to search from a
            // more narrowed-down the search base dn.
            // we can add search on description as well. Right now only search
            // for group names.
            String searchBaseDn = getDomainDN(domainName);

            message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            groupFilter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            // According to this api interface, if somehow
            // We couldn't find the group, we should always return an empty
            // set. (NOT to throw some exception.)
            if (entries == null || entries.length == 0)
            {
                // No groups in system domain.
                // According to API definition we should return empty set.
                return groups;
            }

            int i = 0;
            for (ILdapEntry entry : entries)
            {

                if (matcher.matches(entry, groupAttributes, searchString))
                {
                    String groupName =
                            getStringValue(entry
                                    .getAttributeValues(ATTR_NAME_CN));

                    String description =
                            getOptionalStringValue(entry
                                    .getAttributeValues(ATTR_DESCRIPTION));

                    GroupDetail detail = new GroupDetail(description);

                    PrincipalId groupId =
                            new PrincipalId(groupName, domainName);

                    group = new Group(groupId, this.getPrincipalAliasId(groupName), null/*sid*/, detail);

                    if (i++ < limit || limit <= 0)
                    {
                        groups.add(group);
                    }
                    else break;
                }
            }

        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return groups;
    }

    @Override
    public PrincipalGroupLookupInfo findDirectParentGroups(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        Set<Group> groups = new HashSet<Group>();
        Group group = null;
        String userDn = null;
        ILdapMessage userMessage = null;
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String domainName = getDomain();
            String principalDomainName = id.getDomain();
            String domainBaseDn = getDomainDN(domainName);
            String idName = id.getName();

            if (domainName == null || domainName.isEmpty())
            {
                throw new IllegalStateException(
                        "No domain name found for identity provider");
            }

            String[] userAttrNames = { ATTR_NAME_ACCOUNT };
            String userFilter = null;

            if (belongsToThisIdentityProvider(principalDomainName))
            {
                userFilter = buildQueryByUserORSrvORGroupFilter(id);
            }
            // Handling FSPs to look up FSP DN in SP
            else if (this.isObjectIdCandidate(idName))
            {
                userFilter =
                        String.format(FSP_PRINC_QUERY_BY_EXTERNALID, LdapFilterString.encode(idName));
            } else
            {
                throw new InvalidPrincipalException(String.format(
                        "No principal found for %s.", idName), id.getUPN());
            }

            // Search from domain by default
            // since we need to get group as well
            // check if need to change.

            userMessage =
                    connection.search(domainBaseDn, LdapScope.SCOPE_SUBTREE,
                            userFilter, userAttrNames, false);

            ILdapEntry[] userEntries = userMessage.getEntries();

            if (userEntries == null || userEntries.length == 0)
            {
                throw new InvalidPrincipalException(String.format(
                        "No principal found for %s.", idName), ServerUtils.getUpn(id));
            }
            else if (userEntries.length == 1)
            {
                // Get the principal dn
                userDn = userEntries[0].getDN();
            }
            else
            {
                // This should not happen.
                throw new IllegalStateException(
                        "Invalid state in findDirectParentGroups.");
            }

            String[] attrNames = { ATTR_NAME_CN, ATTR_DESCRIPTION };

            String filter = String.format(GROUP_DIRECT_MEMBER_QUERY, LdapFilterString.encode(userDn));

            // Search from domain dn by default
            // we probably need to optimize here to search from a
            // more narrowed-down the search base dn.
            String groupSearchBaseDn = getDomainDN(domainName);

            message =
                    connection.search(groupSearchBaseDn,
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length > 0)
            {

               for (ILdapEntry entry : entries)
               {

                   String groupName =
                           getStringValue(entry
                                   .getAttributeValues(ATTR_NAME_CN));

                   String description =
                           getOptionalStringValue(entry
                                   .getAttributeValues(ATTR_DESCRIPTION));

                   GroupDetail detail = new GroupDetail(description);

                   PrincipalId groupId = new PrincipalId(groupName, domainName);

                   group = new Group(groupId, this.getPrincipalAliasId(groupName), null/*sid*/, detail);
                   groups.add(group);
               }
            }
        } finally
        {
            if (null != userMessage)
            {
                userMessage.close();
            }
            if (null != message)
            {
                message.close();
            }
        }
        return new PrincipalGroupLookupInfo(groups, null);// this provider does not expose objectIds at the moment
    }

    @Override
    public PrincipalGroupLookupInfo findNestedParentGroups(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        Set<Group> groups = new HashSet<Group>();
        Group group = null;
        ILdapMessage userMessage = null;
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            String domainName = getDomain();
            String principalDomainName = id.getDomain();
            String domainBaseDn = getDomainDN(domainName);
            String idName = id.getName();

            String userFilter = null;

            if (this.belongsToThisIdentityProvider(principalDomainName))
            {
                // According to api interface javadoc, we are handling both user
                // and solution user in this api.
                userFilter = buildQueryByUserORSrvFilter(id);

            }
            // Handling FSPs to look up FSP DN in SP
            else if (this.isObjectIdCandidate(idName))
            {
                userFilter =
                        String.format(FSP_PRINC_QUERY_BY_EXTERNALID, LdapFilterString.encode(idName));
            } else
            {
                throw new InvalidPrincipalException(String.format(
                        "No principal found for %s.", idName), ServerUtils.getUpn(id));
            }

            String[] userAttrNames = { ATTR_NAME_CN, ATTR_NAME_MEMBEROF };

            // Search from domain by default
            // since we need to get group as well
            // check if need to change.

            userMessage =
                    connection.search(domainBaseDn, LdapScope.SCOPE_SUBTREE,
                            userFilter, userAttrNames, false);

            ILdapEntry[] userEntries = userMessage.getEntries();

            if (userEntries == null || userEntries.length != 1)
            {
                // SSO1 interface seems coupling user, solution user and
                // group altogether. However, a customer can pick a user name
                // same as solution user name. This needs to be revisited.
                throw new InvalidPrincipalException(
                        "No user found or we have multiple user, solution with same name.",
                        ServerUtils.getUpn(id));
            } else
            {
               LdapValue[] values =
                        userEntries[0].getAttributeValues(ATTR_NAME_MEMBEROF);

                if (null != values)
                {
                   for (LdapValue val : values)
                   {
                       String groupDn = val.toString();
                       String[] attrNames =
                               { ATTR_NAME_CN, ATTR_DESCRIPTION,
                                       ATTR_NAME_MEMBER };

                       String filter = GROUP_ALL_PRINC_QUERY;

                       // Search just this group
                       String groupSearchBaseDn = groupDn;

                       message =
                               connection.search(groupSearchBaseDn,
                                       LdapScope.SCOPE_BASE, filter, attrNames,
                                       false);

                       ILdapEntry[] entries = message.getEntries();

                       // TODO: is this this the right exception to throw
                       // We need to propagate appropriate exceptions to the admin
                       // and its client.
                       if (entries == null || entries.length == 0)
                       {
                           // No group found for the groupDn, this really shouldn't
                           // happen
                           throw new RuntimeException(String.format(
                                   "no group found for %s", groupDn));
                       }

                       String groupName =
                               getStringValue(entries[0]
                                       .getAttributeValues(ATTR_NAME_CN));

                       String description =
                               getOptionalStringValue(entries[0]
                                       .getAttributeValues(ATTR_DESCRIPTION));

                       GroupDetail detail = new GroupDetail(description);

                       PrincipalId groupId =
                               new PrincipalId(groupName, domainName);

                       group = new Group(groupId, this.getPrincipalAliasId(groupName), null/*sid*/, detail);
                       groups.add(group);
                   }
                }
            }
        } finally
        {
            if (null != userMessage)
            {
                userMessage.close();
            }
            if (null != message)
            {
                message.close();
            }
        }
        return new PrincipalGroupLookupInfo(groups, null); // this provider does not expose object Ids at the moment
    }

    @Override
    public Set<Group> findGroupsInGroup(PrincipalId id, String searchString, int limit)
            throws Exception
    {
        return findGroupsInGroupInternal(id, limit, searchString, GROUPS_BY_CRITERIA_ATTRIBUTES, getStringContainsMatcher());
    }

    @Override
    public Set<Group> findGroupsByNameInGroup(PrincipalId id, String searchString, int limit)
            throws Exception
    {
        return findGroupsInGroupInternal(id, limit, searchString, GROUPS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, getStringStartsWithMatcher());
    }

    private Set<Group> findGroupsInGroupInternal(PrincipalId id, int limit, String searchString, String[] groupAttributes, Matcher matcher) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        Set<Group> groups = new HashSet<Group>();

        if (limit == 0) {
            // Short circuit since they're asking for a list of nothing anyway
            return groups;
        }

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_CN, ATTR_NAME_MEMBER };

            String groupFilter = buildQueryByGroupFilter(id.getName());

            String domainName = getDomain();

            // Search from domain dn by default
            String searchBaseDn = getDomainDN(domainName);

            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            groupFilter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                // Check if multiple group same name but different level
                // We probably should check the searchbaseDN
                if (entries == null || entries.length != 1)
                {
                    throw new InvalidPrincipalException(
                            String.format(
                                    "group %s doesn't exist or multiple groups same name",
                                    id.getName()),
                            ServerUtils.getUpn(id));
                }

                // We found the group
                LdapValue[] values =
                        entries[0].getAttributeValues(ATTR_NAME_MEMBER);

                if (values != null && values.length > 0)
                {
                    for (LdapValue val : values)
                    {
                        int numGroupToRet = 0;
                        if (limit <= 0 || numGroupToRet < limit )
                        {
                            String groupId = val.toString();
                            Group group = null;

                            // Currenlty support FSP in AD/Openldap provider
                            if (groupId.startsWith(ATTR_OBJECT_ID_PREFIX))
                            {
                                // Create a Group directly using the 'val' retrived from group's member attribute
                                // such object shall be resolved by the caller.
                                group =
                                        new Group(
                                            new PrincipalId(groupId, domainName),
                                            this.getPrincipalAliasId(groupId),
                                            null/*sid*/,
                                            new GroupDetail("GroupDetails")
                                        );

                                groups.add(group);
                            } else
                            {
                                // Find subgroups that obey our matching rules
                                Group subGroup =
                                        findGroup(connection, domainName,
                                                val, searchString, groupAttributes, matcher);
                                if (subGroup != null)
                                {
                                    groups.add(subGroup);
                                }
                            }
                            numGroupToRet++;
                        }
                    }
                }
            } finally
            {
                if (message != null)
                {
                    message.close();
                }
            }
        }

        return groups;
    }

    @Override
    public Collection<AttributeValuePair> getAttributes(
            PrincipalId principalId, Collection<Attribute> attributes)
            throws Exception
    {
        ValidateUtil.validateNotNull(principalId, "principalId");

        IIdmAuthStatRecorder idmAuthStatRecorder = this.createIdmAuthStatRecorderInstance(
                DiagnosticsContextFactory.getCurrentDiagnosticsContext().getTenantName(),
                ActivityKind.GETATTRIBUTES, EventLevel.INFO, principalId);
        idmAuthStatRecorder.start();

        List<AttributeValuePair> result = new ArrayList<AttributeValuePair>();

        assert (attributes != null);

        Collection<String> objectclasses = Collections.emptyList();
        List<String> attrNames = new ArrayList<String>();
        List<Attribute> regularAttrs = new ArrayList<Attribute>();
        HashMap<String, Attribute> specialAttrs =
                new HashMap<String, Attribute>();
        String accountName = null;

        Map<String, String> attrMap = this.getStoreDataEx().getAttributeMap();
        if (attrMap != null)
        {
            attrNames.add(ATTR_NAME_OBJECTCLASS);

            for (Attribute attr : attributes)
            {
                String mappedAttr = attrMap.get(attr.getName());
                if (mappedAttr == null)
                {
                    throw new IllegalArgumentException(String.format(
                            "No attribute mapping found for [%s]",
                            attr.getName()));
                }
                if (_specialAttributes.contains(mappedAttr))
                {
                    specialAttrs.put(mappedAttr, attr);
                } else
                {
                    regularAttrs.add(attr);
                    attrNames.add(mappedAttr);
                }
            }
        }

        // we need to retrieve ATTR_NAME_ACCOUNT
        // to make sure we use user name exactly as it is stored
        // in identity provider when constructing UPN
        attrNames.add(ATTR_NAME_ACCOUNT);
        attrNames.add(ATTR_NAME_OBJECTSID);

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String baseDN = this.getStoreDataEx().getUserBaseDn();

            String filter = buildQueryByUserORSrvFilter(principalId);

            ILdapMessage message =
                    connection.search(baseDN, LdapScope.SCOPE_SUBTREE, filter,
                            attrNames.toArray(new String[attrNames.size()]),
                            false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries != null)
                {
                    if (entries.length > 1)
                    {
                        throw new IllegalStateException(
                                "Internal error : duplicate entries were found");
                    }

                    int iAttr = 0;

                    objectclasses =
                            getStringValues(entries[0]
                                    .getAttributeValues(attrNames.get(iAttr++)));

                    accountName =
                            getStringValue(entries[0]
                                    .getAttributeValues(ATTR_NAME_ACCOUNT));

                    String resultObjectSid =
                            getStringValue(entries[0]
                                    .getAttributeValues(ATTR_NAME_OBJECTSID));


                  AttributeValuePair pairGroupSids = new AttributeValuePair();
                  pairGroupSids.setAttrDefinition(new Attribute(IdentityManager.INTERNAL_ATTR_GROUP_OBJECTIDS));
                  if ( ServerUtils.isNullOrEmpty(resultObjectSid) == false )
                  {
                      pairGroupSids.getValues().add( resultObjectSid );
                  }

                  for (Attribute attr : regularAttrs)
                    {
                        try
                        {
                            AttributeValuePair pair = new AttributeValuePair();

                            pair.setAttrDefinition(attr);

                            String attrName = attrNames.get(iAttr);

                            LdapValue[] values =
                                    entries[0].getAttributeValues(attrName);

                            if (values != null)
                            {
                                for (LdapValue value : values)
                                {
                                    if (!value.isEmpty())
                                    {
                                        String val = value.getString();

                                        if (attrName.equals(ATTR_NAME_MEMBEROF))
                                        {
                                            Group group = findGroupByDN(connection,val);
                                            pair.getValues().add(group.getNetbios());
                                            pairGroupSids.getValues().add(group.getObjectId());
                                        } else
                                        {
                                            pair.getValues().add(val);
                                        }
                                    }
                                }
                            }

                            // userPrincipalName and value un-set => set default
                            if (
                                ( attrName.equalsIgnoreCase(ATTR_USER_PRINCIPAL_NAME) ) &&
                                ( pair.getValues().size() == 0 )
                               )
                            {
                                pair.getValues().add(
                                        accountName + "@" + this.getDomain());
                            }

                            result.add(pair);
                        } catch (NoSuchGroupException ex)
                        {
                        }
                        iAttr++;
                    }
                    result.add(pairGroupSids);
                } else
                {
                    throw new InvalidPrincipalException(
                            String.format(
                                    "object not found -- baseDN: [%s], scope: [%s], filter: [%s]",
                                    baseDN, LdapScope.SCOPE_SUBTREE, filter), principalId.getUPN());
                }
            } finally
            {
                message.close();
            }
        }

        Iterator<String> iter = specialAttrs.keySet().iterator();
        while (iter.hasNext())
        {
            String key = iter.next();

            if (key.equals(ATTR_SUBJECT_TYPE))
            {
                AttributeValuePair avPair = new AttributeValuePair();

                avPair.setAttrDefinition(specialAttrs.get(key));
                if (objectclasses.contains(ATTR_NAME_SERVICE))
                {
                    avPair.getValues().add("true");
                } else
                {
                    avPair.getValues().add("false");
                }
                result.add(avPair);
            }
        }

        idmAuthStatRecorder.end();

        return result;
    }

    protected Group findGroupByDN(ILdapConnectionEx connection, String groupDN)
            throws NoSuchGroupException
    {
        String[] attrNames = { ATTR_NAME_CN, ATTR_NAME_OBJECTSID };
        String filter = "(objectClass=group)";

        ILdapMessage message =
                connection.search(groupDN, LdapScope.SCOPE_BASE, filter,
                        attrNames, false);

        String groupName = null;
        String groupSid = null;

        try
        {
            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                throw new NoSuchGroupException();
            } else if (entries.length != 1)
            {
                throw new IllegalStateException(
                        "Internal error : duplicate entries were found");


            }

            LdapValue[] values =
                    entries[0].getAttributeValues(ATTR_NAME_CN);

            if (values == null || values.length <= 0)
            {
                throw new NoSuchGroupException();
            }
            if (values.length != 1)
            {
                throw new IllegalStateException(
                        "Internal error : duplicate entries were found");
            }

            if (!values[0].isEmpty())
            {
                groupName = values[0].getString();
            }

            if (groupName == null || groupName.length() == 0)
            {
                throw new IllegalStateException(
                        "group name is null or empty string");
            }

            groupSid =
                    getStringValue(entries[0]
                            .getAttributeValues(ATTR_NAME_OBJECTSID));

        } finally
        {
            message.close();
        }

        // Also whether detail can be null or not
        PrincipalId gid = new PrincipalId(groupName, getDomainFromDN(groupDN));

        return new Group(gid, this.getPrincipalAliasId(groupName), groupSid, null);
    }

    @Override
    public PrincipalId addServicePrincipal(String accountName,
            SolutionDetail detail) throws Exception
    {
        ValidateUtil.validateNotEmpty(accountName, "accountName");

        return addServicePrincipalInternal(accountName, detail);
    }

    private PrincipalId addServicePrincipalInternal(String accountName,
            SolutionDetail detail) throws Exception
    {
        ILdapMessage message = null;
        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();

        final String domainName = getDomain();
        final PrincipalId newSolutionUserId = new PrincipalId(accountName, domainName);
        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            if (existsPrincipal(accountName, newSolutionUserId, connection))
            {
                // There exists a user or group already with the same name
                // Due to limitation on SSO1 interface, this is forbidden.
                // In another word, we can not have a user, solution user or group
                // with the same name.
                throw new InvalidPrincipalException(
                        String.format(
                                "Another user or group already exists with the same name: accountName=%s, domain=%s",
                                accountName, domainName), newSolutionUserId.getUPN());
            }

            // Detect duplicate certificate that has the same subject DN
            String[] attrNames = { ATTR_NAME_ACCOUNT, ATTR_NAME_CERT};

            X509Certificate cert = detail.getCertificate();
            if (cert == null) {
               throw new InvalidArgumentException(String.format(
                     "addServicePrincipal for user %s fails: solution user must have certificate", accountName));
            }
            Principal subjectDN = cert.getSubjectX500Principal();
            ValidateUtil.validateNotNull(subjectDN, "Solution userCertificate subjectDn");
            ValidateUtil.validateNotEmpty(subjectDN.getName(), "Solution userCertificate subjectDn");

            String filter = String.format(SVC_PRINC_QUERY_BY_SUBJECT_DN, LdapFilterString.encode(subjectDN.getName()));
            String solutionUsersContainerDn = getServicePrincipalsDN(domainName);

            message =
                    connection.search(solutionUsersContainerDn,
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length > 0)
            {
               throw new DuplicateCertificateException(
                           "Same subjectDN already exists for another solution user");
            }

            String dn =
                    String.format("CN=%s,%s", accountName,
                            solutionUsersContainerDn);

            LdapMod objectClass = null; // Mandatory
            LdapMod attrName = null; // Mandatory
            LdapMod attrAccount = null; // Mandatory
            LdapMod attrSubjectDN = null; // Mandatory
            // (this should be mandatory, subjectDn is a MUST which is derived from cert)
            LdapMod attrCert = null; // Mandatory
            LdapMod attrDescription = null; // Optional

            objectClass =
                    new LdapMod(
                            LdapModOperation.ADD,
                            ATTR_NAME_OBJECTCLASS,
                            new LdapValue[] {
                                    LdapValue.fromString("user"),
                                    LdapValue.fromString("vmwServicePrincipal") });
            attributeList.add(objectClass);

            attrName =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_CN,
                            new LdapValue[] { LdapValue.fromString(accountName) });
            attributeList.add(attrName);

            attrAccount =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_ACCOUNT,
                            new LdapValue[] { LdapValue.fromString(accountName) });
            attributeList.add(attrAccount);

            String desc = detail.getDescription();
            if (null != desc && !desc.isEmpty())
            {
                attrDescription =
                        new LdapMod(LdapModOperation.ADD, ATTR_SVC_DESCRIPTION,
                                new LdapValue[] { LdapValue.fromString(desc) });

                attributeList.add(attrDescription);
            }

            // vmwSTSSubjectDn is a MUST attribute
            attrSubjectDN = new LdapMod(LdapModOperation.ADD,
                                        ATTR_NAME_SUBJECTDN,
                                        new LdapValue[] { LdapValue.fromString(subjectDN.getName()) });
            attributeList.add(attrSubjectDN);

            byte[] certBytes = cert.getEncoded();
            assert (certBytes != null);

            attrCert =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_CERT,
                            new LdapValue[] { new LdapValue(certBytes) });
            attributeList.add(attrCert);

           connection.addObject(dn,
                   attributeList.toArray(new LdapMod[attributeList.size()]));

            if (this.addUserToGroup(newSolutionUserId, IdentityManager.WELLKNOWN_SOLUTIONUSERS_GROUP_NAME) == false)
            {
                logger.info(
                        String.format(
                            "Failed to add user %s to well-known SolutionUsers group",
                            accountName));
            }

            return newSolutionUserId;
        }
        catch (AlreadyExistsLdapException e)
        {
            // The solution user already exists.
            throw new InvalidPrincipalException(String.format(
               "solution user %s already exists", accountName),
               ServerUtils.getUpn(newSolutionUserId), e);
        }
        finally
        {
            if (null != message)
            {
                message.close();
            }
        }
    }

    @Override
    public SolutionUser findServicePrincipal(String accountName) throws Exception
    {
        return findServicePrincipalInContainer(accountName,
                getServicePrincipalsDN(getDomain()), true);
    }

    @Override
    public SolutionUser findServicePrincipalInExternalTenant(String accountName)
            throws Exception
    {
        return findServicePrincipalInContainer(accountName,
                getTenantSearchBaseRootDN(), true);
    }

    private SolutionUser findServicePrincipalInContainer(String accountName,
            String containerDn, boolean isExternal) throws Exception
    {
        ValidateUtil.validateNotEmpty(containerDn, "container Dn");

        SolutionUser result = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            final PrincipalId principal = new PrincipalId(accountName, this.getDomain());

            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_NAME_CERT, ATTR_SVC_DESCRIPTION,
                            ATTR_NAME_ACCOUNT_FLAGS };

            String filter = buildQueryBySrvFilter(principal);

            ILdapMessage message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0)
                {
                    // According to admin interface, we should return null
                    // if not found
                    logger.info(String.format(
                                "Cannot find solution user [%s@%s] in [%s]",
                                principal.getName(), principal.getDomain(),
                                containerDn));

                    return null;
                } else if (entries.length != 1)
                {
                    throw new IllegalStateException(
                            "More than one object was found");
                }

                String upn =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                String username =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_ACCOUNT));

                final PrincipalId foundPrincipal = this.getPrincipalId(upn, username, getDomain());

                //                LdapValue[] aliasValue =
                //                        entries[0].getAttributeValues(ATTR_NAME_ALIAS);
                //
                //                if (aliasValue != null)
                //                {
                //                    alias = new PrincipalId(
                //                                    getStringValue(aliasValue),
                //                                    getDomain());
                //                }

                LdapValue[] certValue =
                        entries[0].getAttributeValues(ATTR_NAME_CERT);

                if (certValue == null || certValue[0] == null)
                {
                    throw new IllegalStateException(
                                "Certificate content should exist.");
                }

                X509Certificate cert = ServerUtils.getCertificateValue(certValue);

                String description =
                            getOptionalStringValue(entries[0]
                                    .getAttributeValues(ATTR_SVC_DESCRIPTION));

                int flag =
                            getOptionalIntegerValue(
                                    entries[0]
                                            .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                    0);

                boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                SolutionDetail detail =
                            new SolutionDetail(cert, description);

                result = new SolutionUser(foundPrincipal, this.getPrincipalAliasId(username), null, detail, disabled, isExternal);
            } finally
            {
                message.close();
            }
        }

        return result;
    }

    @Override
    public Set<SolutionUser> findServicePrincipals(String searchString)
            throws Exception
    {
        return findServicePrincipalsInContainer(searchString,
                getServicePrincipalsDN(getDomain()), false);
    }

    @Override
    public Set<SolutionUser> findServicePrincipalsInExternalTenant(
            String searchString) throws Exception
    {
        return findServicePrincipalsInContainer(searchString,
                getTenantSearchBaseRootDN(), true);
    }

    private Set<SolutionUser> findServicePrincipalsInContainer(
            String searchString, String containerDn, boolean isExternal) throws Exception
    {
        Set<SolutionUser> solutions = new HashSet<SolutionUser>();
        SolutionUser solution = null;
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            ValidateUtil.validateNotEmpty(containerDn, "container Dn");

            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_NAME_CERT, ATTR_SVC_DESCRIPTION,
                            ATTR_NAME_ACCOUNT_FLAGS };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.
            String filter = SVC_ALL_PRINC_QUERY;

            // we can add search on description as well. Right now only search
            // for group names.
            message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            // According to this api interface, if somehow
            // We couldn't find the solution user, we should always return an empty
            // set. (NOT to throw some exception.)
            if (entries == null || entries.length == 0)
            {
                return solutions;
            }

            for (ILdapEntry entry : entries)
            {
                // Also fill alias info
                // PrincipalId alias = null;
                // PrincipalId alias = null;
                // we should change to use AccountName which is
                // supposed to be unique. For now use CN

                String solutionName =
                        getStringValue(entry
                                .getAttributeValues(ATTR_NAME_ACCOUNT));

                String upn =
                        getOptionalStringValue(entry
                                .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                String description =
                        getOptionalStringValue(entry
                                .getAttributeValues(ATTR_SVC_DESCRIPTION));

                if (containsSearchString(solutionName, searchString) ||
                    containsSearchString(upn, searchString) ||
                    containsSearchString(description, searchString))
                {
                    LdapValue[] certValue =
                            entry.getAttributeValues(ATTR_NAME_CERT);

                    if (certValue == null || certValue[0] == null)
                    {
                        throw new IllegalStateException(
                                    "Certificate content should exist.");
                    }

                    PrincipalId principal = this.getPrincipalId(upn, solutionName, getDomain());

                    X509Certificate cert = ServerUtils.getCertificateValue(certValue);

                    SolutionDetail detail =
                                new SolutionDetail(cert, description);

                    int flag = getOptionalIntegerValue(
                                        entries[0].getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                        0);

                    boolean disabled =
                                ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                    solution = new SolutionUser(principal, this.getPrincipalAliasId(solutionName), null, detail,
                                                disabled, isExternal);

                    solutions.add(solution);
                }
            }
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return solutions;
    }

    @Override
    public Set<SolutionUser> findServicePrincipalsInGroup(String groupName,
            String searchString) throws Exception
    {
        Set<SolutionUser> solutions = new HashSet<SolutionUser>();
        ILdapMessage message = null;
        ILdapMessage solutionMessage = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_CN, ATTR_DESCRIPTION, ATTR_NAME_MEMBER,
                      ATTR_NAME_ACCOUNT_FLAGS };

            String filter = buildQueryByGroupFilter(groupName);

            String domainName = getDomain();

            // Search from domain dn by default
            String searchBaseDn = getDomainDN(domainName);
            message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            // Check if multiple group same name but different level
            // We probably should check the searchbaseDN
            if (entries == null || entries.length != 1)
            {
                throw new InvalidPrincipalException(String.format(
                        "group %s doesn't exist or multiple groups same name", groupName),
                        ServerUtils.getUpn(groupName, domainName));
            }
            else
            {
                String servicePrincipalsDN = getServicePrincipalsDN(getDomain());

                // We found the group
                LdapValue[] values =
                        entries[0].getAttributeValues(ATTR_NAME_MEMBER);

                if (values != null)
                {
                   for (LdapValue val : values)
                   {

                       String[] solutionAttrNames =
                               { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_NAME_CERT,
                                       ATTR_SVC_DESCRIPTION,
                                       ATTR_NAME_ACCOUNT_FLAGS };

                       // We only want solution user objects
                       String solutionFilter = SVC_ALL_PRINC_QUERY;

                       // Search from Users by default
                       String solutionSearchBaseDn = val.toString();
                       try {
                           solutionMessage =
                                   connection.search(solutionSearchBaseDn,
                                           LdapScope.SCOPE_BASE, solutionFilter,
                                           solutionAttrNames, false);

                           ILdapEntry[] solutionEntries = solutionMessage.getEntries();

                           if (solutionEntries == null || solutionEntries.length == 0)
                           {
                               // This isn't a solution user
                               continue;
                           } else if (solutionEntries.length != 1)
                           {
                               throw new IllegalStateException(
                                       "More than one solution user found");
                           }

                           boolean isExternal = true;
                           // check whether this member solution user lives in ldu, only then it is internal
                           if (servicePrincipalsDN != null && !servicePrincipalsDN.isEmpty() &&
                               solutionSearchBaseDn.toLowerCase().contains(servicePrincipalsDN.toLowerCase()))
                           {
                               isExternal= false;
                           }

                           String accountName =
                                   getStringValue(solutionEntries[0]
                                           .getAttributeValues(ATTR_NAME_ACCOUNT));

                           String upn =
                                   getOptionalStringValue(solutionEntries[0]
                                           .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                           String description =
                                   getOptionalStringValue(solutionEntries[0]
                                           .getAttributeValues(ATTR_SVC_DESCRIPTION));

                           if (containsSearchString(accountName, searchString) ||
                               containsSearchString(upn, searchString) ||
                               containsSearchString(description, searchString))
                           {
                               PrincipalId principal = null;

                               principal = this.getPrincipalId(upn, accountName, domainName);

                               LdapValue[] certValue =
                                       solutionEntries[0]
                                               .getAttributeValues(ATTR_NAME_CERT);

                               if (certValue == null || certValue[0] == null)
                               {
                                   throw new IllegalStateException(
                                               "Certificate content should exist.");
                               }

                               X509Certificate cert = ServerUtils.getCertificateValue(certValue);

                               SolutionDetail detail = new SolutionDetail(cert, description);

                               int flag = getOptionalIntegerValue(
                                                   entries[0].getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                                   0);

                               boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                               SolutionUser solution =
                                           new SolutionUser(principal, this.getPrincipalAliasId(accountName), null,
                                                   detail, disabled, isExternal);

                               solutions.add(solution);
                           }
                       } catch (com.vmware.identity.interop.ldap.NoSuchObjectLdapException ne) {
                           logger.warn(String.format("Group member with attribute [%s] does not exist.", solutionSearchBaseDn), ne);
                           continue; // skip deleted or external members
                       } finally {
                           if (null != solutionMessage)
                           {
                               solutionMessage.close();
                           }
                       }
                   }
                }
            }
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return solutions;
    }

    @Override
    public SolutionUser findServicePrincipalByCertDn(String subjectDN)
            throws Exception
    {
        ValidateUtil.validateNotEmpty(subjectDN, "subjectDN");

        return findServicePrincipalByCertDnInContainer(subjectDN,
                getServicePrincipalsDN(getDomain()), false);
    }

    @Override
    public SolutionUser findServicePrincipalByCertDnInExternalTenant(
            String subjectDN) throws Exception
    {
        ValidateUtil.validateNotEmpty(subjectDN, "sabjectDN");

        return findServicePrincipalByCertDnInContainer(subjectDN,
                getTenantSearchBaseRootDN(), true);
    }

    private SolutionUser findServicePrincipalByCertDnInContainer(
            String subjectDN, String containerDn, boolean isExternal) throws Exception
    {
        SolutionUser result = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            ValidateUtil.validateNotEmpty(containerDn, "container Dn");

            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_NAME_CERT, ATTR_SVC_DESCRIPTION,
                            ATTR_NAME_ACCOUNT_FLAGS };

            String filter =
                    String.format(SVC_PRINC_QUERY_BY_SUBJECT_DN,
                            LdapFilterString.encode(subjectDN));

            ILdapMessage message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0)
                {
                    // According to admin interface, we should return null
                    // if not found
                    return null;
                } else if (entries.length != 1)
                {
                    throw new IllegalStateException(
                            "More than one solution user found");
                }

                PrincipalId principal = null;

                String username =
                        getStringValue(entries[0]
                                .getAttributeValues(ATTR_NAME_ACCOUNT));

                String upn =
                        getOptionalStringValue(entries[0]
                                .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                principal = this.getPrincipalId(upn, username, getDomain());

                //                LdapValue[] aliasValue =
                //                            entries[0].getAttributeValues(ATTR_NAME_ALIAS);
                //
                //                if (aliasValue != null)
                //                {
                //                    alias = new PrincipalId(
                //                                    getStringValue(aliasValue),
                //                                    getDomain());
                //                }

                LdapValue[] certValue =
                        entries[0].getAttributeValues(ATTR_NAME_CERT);

                if (certValue == null || certValue[0] == null)
                {
                    throw new IllegalStateException(
                                "Certificate content should exist.");
                }

                X509Certificate cert = ServerUtils.getCertificateValue(certValue);

                String description =
                            getOptionalStringValue(entries[0]
                                    .getAttributeValues(ATTR_SVC_DESCRIPTION));

                SolutionDetail detail =
                            new SolutionDetail(cert, description);

                int flag =
                            getOptionalIntegerValue(
                                    entries[0]
                                            .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                    0);

                boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                result = new SolutionUser(principal, this.getPrincipalAliasId(username), null, detail, disabled, isExternal);
            } finally
            {
                message.close();
            }
        }

        return result;
    }

    private PrincipalId addUserInternal(String accountName, PersonDetail detail,
            LdapValue pwd, String hashingAlgorithm, String extIdpEntityId, String extUserId)
            throws Exception
    {
        final String domainName = getDomain();
        final PrincipalId newUserUpn;

        if(ServerUtils.isNullOrEmpty(detail.getUserPrincipalName()) == false)
        {
            ValidateUtil.validateUpn(detail.getUserPrincipalName(), "userPrincipalName");
            String[] parts = detail.getUserPrincipalName().split("@");
            newUserUpn = new PrincipalId(parts[0], ValidateUtil.getCanonicalUpnSuffix(parts[1]));
        }
        else
        {
            newUserUpn = new PrincipalId(accountName, domainName);
        }

        this.validateUpnFromThisDomain(newUserUpn);

        // if custom upn suffix matches alias, then user name must match the account name
        if ( ( newUserUpn.getDomain().equalsIgnoreCase(this.getAlias()) &&
             (newUserUpn.getName().equalsIgnoreCase(accountName) == false ) )
           )
        {
            throw new InvalidArgumentException(
                String.format(
                    "User AccountName must match name in userPrincipalName when domainNameSuffix within userPrincipalName matches an alias. AccountName=[%s], userPrincipalName=[%s]",
                    accountName,
                    detail.getUserPrincipalName()
                )
            );
        }

        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            if ( existsPrincipal(accountName, newUserUpn, connection) )
            {
                // There exists a user or group already with the same name
                // Due to limitation on SSO1 interface, this is forbidden.
                // In another word, we can not have a user, solution user or group
                // with the same name.
                throw new InvalidPrincipalException(
                        String.format(
                                "Another user or group already exists with the same name: accountName=%s, principal=%s@%s",
                                accountName, newUserUpn.getName(), newUserUpn.getDomain()), ServerUtils.getUpn(newUserUpn));
            }

            // We are creating user into Users now
            // If user needs to be created somewhere else, code to be updated
            String dn = String.format("CN=%s,%s", accountName, getUsersDN(domainName));

            LdapMod objectClass = null;
            LdapMod attrName = null;
            LdapMod attrAccount = null;
            LdapMod attrLastName = null;
            LdapMod attrFirstName = null;
            LdapMod attrEmail = null;
            LdapMod attrDescription = null;
            LdapMod attrAccountFlag = null;
            LdapMod attrPassword = null;
            LdapMod attrPasswordSchema = null;
            LdapMod attrUpn = null;

            objectClass =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_OBJECTCLASS,
                            new LdapValue[] { LdapValue
                                    .fromString(ATTR_NAME_USER), });
            attributeList.add(objectClass);

            attrName =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_CN,
                            new LdapValue[] { LdapValue.fromString(accountName) });
            attributeList.add(attrName);

            attrAccount =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_ACCOUNT,
                            new LdapValue[] { LdapValue.fromString(accountName) });
            attributeList.add(attrAccount);

            attrUpn =
               new LdapMod(LdapModOperation.ADD, ATTR_USER_PRINCIPAL_NAME,
                  new LdapValue[] {
                        LdapValue.fromString(
                           doesUpnMatchRegisteredSuffix(newUserUpn) ? newUserUpn.getUPN() : GetUPN(newUserUpn)
                        )
                     });
            attributeList.add(attrUpn);

            String lastName = detail.getLastName();
            if (!ServerUtils.isNullOrEmpty(lastName))
            {
                attrLastName =
                        new LdapMod(LdapModOperation.ADD, ATTR_LAST_NAME,
                                new LdapValue[] { LdapValue
                                        .fromString(lastName) });
                attributeList.add(attrLastName);
            }

            String firstName = detail.getFirstName();
            if (!ServerUtils.isNullOrEmpty(firstName))
            {
                attrFirstName =
                        new LdapMod(LdapModOperation.ADD, ATTR_FIRST_NAME,
                                new LdapValue[] { LdapValue
                                        .fromString(firstName) });
                attributeList.add(attrFirstName);
            }

            String email = detail.getEmailAddress();
            if (!ServerUtils.isNullOrEmpty(email))
            {
                attrEmail =
                        new LdapMod(LdapModOperation.ADD, ATTR_EMAIL_ADDRESS,
                                new LdapValue[] { LdapValue.fromString(email) });
                attributeList.add(attrEmail);
            }

            String desc = detail.getDescription();
            if (!ServerUtils.isNullOrEmpty(desc))
            {
                attrDescription =
                        new LdapMod(LdapModOperation.ADD, ATTR_DESCRIPTION,
                                new LdapValue[] { LdapValue.fromString(desc) });
                attributeList.add(attrDescription);
            }

            // default enabled and not locked when adding a user
            int flag = 0;

            attrAccountFlag =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_ACCOUNT_FLAGS,
                            new LdapValue[] { LdapValue.fromString(Integer
                                    .toString(flag)) });
            attributeList.add(attrAccountFlag);

            if (hashingAlgorithm != null && !hashingAlgorithm.isEmpty())
            {
                attrPasswordSchema =
                        new LdapMod(
                                LdapModOperation.ADD,
                                ATTR_NAME_PASSWORD_SCHEMA,
                                new LdapValue[] { LdapValue
                                        .fromString(new String(hashingAlgorithm)) });
                attributeList.add(attrPasswordSchema);
            }

            if (pwd != null) {
                attrPassword =
                        new LdapMod(LdapModOperation.ADD, ATTR_USER_PASSWORD,
                                new LdapValue[] {pwd});
                attributeList.add(attrPassword);
            }

            connection.addObject(dn,
                    attributeList.toArray(new LdapMod[attributeList.size()]));

            if (extIdpEntityId != null && !extIdpEntityId.isEmpty()) {
                ArrayList<LdapMod> additionalAttributeList = new ArrayList<LdapMod>();
                // add vmwExternalIdpUser auxiliary class to jit user
                LdapMod auxObjectClass =
                        new LdapMod(LdapModOperation.ADD, ATTR_NAME_OBJECTCLASS,
                                new LdapValue[] { LdapValue
                                        .fromString(ATTR_NAME_JIT_USER) });
                additionalAttributeList.add(auxObjectClass);

                // add vmwSTSEntityId attribute to jit user
                LdapMod extIdpEntityIdAttr =
                        new LdapMod(LdapModOperation.ADD, ATTR_EXTERNAL_IDP_ID,
                                new LdapValue[] {LdapValue.fromString(extIdpEntityId)});
                additionalAttributeList.add(extIdpEntityIdAttr);

                ValidateUtil.validateNotEmpty(extUserId, "External User Id must be provided for JIT provisioning.");

                // add ext user id attribute to jit user
                LdapMod extUserIdAttr =
                        new LdapMod(LdapModOperation.ADD, ATTR_JIT_USER_ID,
                                new LdapValue[] {LdapValue.fromString(extUserId)});
                additionalAttributeList.add(extUserIdAttr);

                connection.modifyObject(dn,
                        additionalAttributeList.toArray(new LdapMod[additionalAttributeList.size()]));
            }

            return this.getPrincipalId(GetUPN(newUserUpn), accountName, domainName);
        } catch (AlreadyExistsLdapException e)
        {
            // The user already exists.
            throw new InvalidPrincipalException(
                String.format(
                    "user account name=%s, upn=%s@%s already exists",
                    accountName,
                    newUserUpn.getName(),
                    newUserUpn.getDomain()
                ),
                ServerUtils.getUpn(newUserUpn),
                e
            );
        }
    }

    @Override
    public PrincipalId addUser(String accountName, PersonDetail detail,
            char[] password) throws Exception
    {
        ValidateUtil.validateNotEmpty(accountName, "accountName");

        return addUserInternal(accountName, detail,
              password == null ? null : LdapValue.fromString(new String(password)), null, null, null);
    }

    /**
     * Adds a regular/jit user to the system domain.
     * If extIdpEntityId is non-null, this user is a jit user.
     * extUserId must be provided for jit user provisioning.
     *
     * @param accountName Account name of regular/jit user. required, non-null,
     * @param detail Detailed information about the user. required, non-null.
     * @param password User's password
     * @param extIdpEntityId ExternalIDP entity ID. If it is non-null, add jit user.
     * @param extUserId External User's ID. Required attribute for jit user.
     * @return Principal id of the regular user after it has been created.
     * @throws Exception
     */
    @Override
    public PrincipalId addUser(String accountName, PersonDetail detail,
            char[] password, String extIdpEntityId, String extUserId) throws Exception
    {
        ValidateUtil.validateNotEmpty(accountName, "accountName");
        ValidateUtil.validateNotNull(detail, "personDetail");

        if (!ValidateUtil.isEmpty(extIdpEntityId)) {
            ValidateUtil.validateNotEmpty(extUserId, "External Idp user Id.");
        }

        return addUserInternal(accountName, detail,
              password == null ? null : LdapValue.fromString(new String(password)), null, extIdpEntityId, extUserId);
    }

    @Override
    public PrincipalId addUser(String accountName, PersonDetail detail,
            byte[] hashedPassword, String hashingAlgorithm) throws Exception
    {
        ValidateUtil.validateNotNull(accountName, "accountName");

        ValidateUtil.validateNotNull(hashingAlgorithm,
                "User Password HashingAlgorithm");

        if (!HashingAlgorithmType.isValidHashingAlgorithm(hashingAlgorithm))
        {
            throw new IllegalArgumentException(String.format(
                    "Unsupported password hashing algorithm - %s is given",
                    hashingAlgorithm));
        }

        return addUserInternal(accountName, detail,
              new LdapValue(hashedPassword),
              hashingAlgorithm, null, null);
    }

    private void validateUpnFromThisDomain(PrincipalId upn) throws InvalidArgumentException
    {
        if (upn != null)
        {
            if(this.belongsToThisIdentityProvider(upn.getDomain()) == false)
            {
                throw new InvalidArgumentException(
                    String.format(
                        "Invalid principal name %s@%s. Unrecognized upn suffix.",
                        upn.getName(),
                        upn.getDomain()
                    )
                );
            }
        }
    }

    private boolean doesUpnMatchRegisteredSuffix(PrincipalId upn)
    {
        boolean result = false;
        if (upn != null)
        {
            result = ( this.getUpnSuffixes() != null ) &&
                     ( this.getUpnSuffixes().contains(ValidateUtil.getCanonicalUpnSuffix(upn.getDomain())) );
        }
        return result;
    }

    @Override
    public boolean enableUserAccount(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        boolean changed = false;

        try
        {
            // locate user in 'CN=users' container
            changed = enableUserAccountInContainer(id, getUsersDN(getDomain()));
        } catch (InvalidPrincipalException ex)
        {
            // If not found, locate user in ldus container (solution users)
            changed = enableUserAccountInContainer(id, getServicePrincipalsDN(getDomain()));
        }

        return changed;
    }

    private boolean enableUserAccountInContainer(PrincipalId id,
            String containerDn) throws Exception
    {

        ILdapMessage message = null;
        boolean changed = false;
        String userDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_ACCOUNT_FLAGS };

            String filter = buildQueryByUserORSrvFilter(id);

            message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                throw new InvalidPrincipalException(String.format(
                        "user or solution %s doesn't exist.", id.getName()), id.getUPN());
            }
            else if (entries.length == 1)
            {
                userDn = entries[0].getDN();
            } else
            {
                // This should not happen.
                throw new IllegalStateException(
                        "Invalid state in enableUserAccount.");
            }

            int currentFlag =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                            0);

            boolean disabled = ((currentFlag & USER_ACCT_DISABLED_FLAG) != 0);

            if (disabled)
            {
                int newFlag = currentFlag & ~USER_ACCT_DISABLED_FLAG;

                LdapMod attrFlag =
                        new LdapMod(LdapModOperation.REPLACE,
                                ATTR_NAME_ACCOUNT_FLAGS,
                                new LdapValue[] { new LdapValue(newFlag) });

                LdapMod attributes[] = { attrFlag };

                // Update with new member list
                connection.modifyObject(userDn, attributes);
                changed = true;
            }
        } catch (NumberFormatException e)
        {
            // This shouldn't happen unless the account flag isn't a parsable.
            throw e;
        } catch (NoSuchObjectLdapException e)
        {
            throw new InvalidPrincipalException(String.format(
                    "user or solution %s doesn't exist.", id.getName()), id.getUPN());
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return changed;
    }

    @Override
    public boolean disableUserAccount(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        boolean changed = false;

        try
        {
            // locate user in 'CN=users' container
            changed =
                    disableUserAccountInContainer(id, getUsersDN(getDomain()));
        } catch (InvalidPrincipalException ex)
        {
            // If not found, locate user in ldus container (solution users)
            changed =disableUserAccountInContainer(id, getServicePrincipalsDN(getDomain()));
        }

        return changed;
    }

    private boolean disableUserAccountInContainer(PrincipalId id,
            String containerDn) throws Exception
    {
        ILdapMessage message = null;
        boolean changed = false;
        String userDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_ACCOUNT_FLAGS };

            String filter = buildQueryByUserORSrvFilter(id);

            message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                throw new InvalidPrincipalException(String.format(
                        "user or solution %s doesn't exist.", id.getName()), id.getUPN());
            }
            else if (entries.length == 1)
            {
                userDn = entries[0].getDN();
            } else
            {
                // This should not happen.
                throw new IllegalStateException(
                        "Invalid state in disableUserAccount.");
            }

            int currentFlag =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                            0);

            boolean disabled = ((currentFlag & USER_ACCT_DISABLED_FLAG) != 0);

            if (!disabled)
            {
                int newFlag = currentFlag | USER_ACCT_DISABLED_FLAG;

                LdapMod attrFlag =
                        new LdapMod(LdapModOperation.REPLACE,
                                ATTR_NAME_ACCOUNT_FLAGS,
                                new LdapValue[] { new LdapValue(newFlag) });

                LdapMod attributes[] = { attrFlag };

                // Update with new member list
                connection.modifyObject(userDn, attributes);
                changed = true;
            }
        } catch (NumberFormatException e)
        {
            // This shouldn't happen unless the account flag isn't a parsable.
            throw e;
        } catch (NoSuchObjectLdapException e)
        {
            throw new InvalidPrincipalException(String.format(
                    "user or solution %s doesn't exist.", id.getName()), id.getUPN());
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return changed;
    }

    @Override
    public boolean unlockUserAccount(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        ILdapMessage message = null;
        boolean changed = false;
        String userDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_ACCOUNT_FLAGS };

            String filter = buildQueryByUserFilter(id);

            String domainName = getDomain();

            // Search from Users by default
            String searchBaseDn = getUsersDN(domainName);
            message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length != 1)
            {
                throw new InvalidPrincipalException(String.format(
                        "user %s doesn't exist or multiple users same name",
                        id.getName()), id.getUPN());
            } else
            {
                userDn = entries[0].getDN();
            }

            int currentFlag =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                            0);

            boolean locked = ((currentFlag & USER_ACCT_LOCKED_FLAG) != 0);

            if (locked)
            {
                int newFlag = currentFlag & ~USER_ACCT_LOCKED_FLAG;

                LdapMod attrFlag =
                        new LdapMod(LdapModOperation.REPLACE,
                                ATTR_NAME_ACCOUNT_FLAGS,
                                new LdapValue[] { new LdapValue(newFlag) });

                LdapMod attributes[] = { attrFlag };

                // Update with new member list
                connection.modifyObject(userDn, attributes);
                changed = true;
            }
        } catch (NumberFormatException e)
        {
            // This shouldn't happen unless the account flag isn't a parsable.
            throw e;
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }

        return changed;
    }

    @Override
    public boolean addUserToGroup(PrincipalId userId, String groupName)
            throws Exception
    {
        boolean added = false;

        String userDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            final String userDomainName = userId.getDomain();

            if (this.belongsToThisIdentityProvider(userDomainName))
            {
                // Find the user
                String[] attrNames = { ATTR_NAME_CN };

                String filter = buildQueryByUserORSrvFilter(userId);

                // for now all users created under Users
                // Check if searchDn should be revised
                String searchDn = getTenantSearchBaseRootDN();

                ILdapMessage userMessage =
                        connection.search(searchDn, LdapScope.SCOPE_SUBTREE,
                                filter, attrNames, false);

                try
                {
                    ILdapEntry[] entries = userMessage.getEntries();

                    if (entries == null || entries.length != 1)
                    {
                        // Use doesn't exist or multiple user same name
                        throw new InvalidPrincipalException(
                                String.format(
                                        "user %s doesn't exist or multiple users with same name",
                                        userId.getName()), userId.getUPN());
                    } else
                    {
                        userDn = entries[0].getDN();
                    }
                } finally
                {
                    userMessage.close();
                }
            }
            // Handle FSPs
            else
            {
                userDn = userId.getName();
            }

            // Find the group
            String[] groupAttrNames = { ATTR_NAME_CN, ATTR_NAME_MEMBER };

            String filter = buildQueryByGroupFilter(groupName);

            String searchDn = getTenantSearchBaseRootDN();

            ILdapMessage groupMessage =
                    connection.search(searchDn, LdapScope.SCOPE_SUBTREE,
                            filter, groupAttrNames, false);

            try
            {
                ILdapEntry[] entries = groupMessage.getEntries();

                // Check if multiple group same name but different level
                // We probably should check the searchbaseDN
                if (entries == null || entries.length != 1)
                {
                    throw new InvalidPrincipalException(
                            String.format(
                                    "group %s doesn't exist or multiple groups same name",
                                    groupName), ServerUtils.getUpn(groupName, getDomain()));
                }

                // We found the group
                LdapValue[] values =
                        entries[0].getAttributeValues(ATTR_NAME_MEMBER);

                //check whether member already exist
                if (null != values)
                {
                    for (LdapValue val : values)
                    {
                        // Not to add the member if already there at this point of check.
                        // there could be race condition where this existing user is
                        // about to be deleted by another thread, but this should not be
                        // an applicable scenario at least for now.
                        //
                        // This rare race condition, if becomes applicable scenario in the future,
                        // could be mitigate by client-initiated retry upon receiving the
                        // thrown exception.
                        if (0 == val.toString().compareToIgnoreCase(userDn))
                        {
                            // Admin api states if already in the group
                            // should return false.
                            // Therefore, setting false here for the found
                            // case.
                            throw new MemberAlreadyExistException(
                                    String.format(
                                            "group %s currently has user %s as its member",
                                            groupName, userDn));
                        }
                    }
                }

                LdapMod attributes[] = { new LdapMod(LdapModOperation.ADD, ATTR_NAME_MEMBER,
                                                     LdapValue.fromString(userDn)) };

                // add new member to the group
                try
                {
                    connection.modifyObject(entries[0].getDN(), attributes);
                }
                catch (AttributeOrValueExistsLdapException aovele)
                {
                    logger.info(
                        String.format(
                            "member [%s] already exists in group [%s] -- just added by a different operation",
                            userDn, entries[0].getDN()));
                    // we will return true in this case --- deemed as successful operation.
                }
                added = true;
            } finally
            {
                groupMessage.close();
            }
        }

        return added;
    }

    @Override
    public PrincipalId addGroup(String groupName, GroupDetail detail)
            throws Exception
    {
        ValidateUtil.validateNotEmpty(groupName, "groupName");

        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();

        final PrincipalId newGroupId = new PrincipalId(groupName, this.getDomain());
        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            if (existsPrincipal(groupName, newGroupId, connection))
            {
                // There exists a user or group already with the same name
                // Due to limitation on SSO1 interface, this is forbidden.
                // In another word, we can not have a user, solution user or group
                // with the same name.
                throw new InvalidPrincipalException(
                        String.format(
                                "Another user or group %s already exists with the same name",
                                groupName), newGroupId.getUPN());
            }

            String domainName = getDomain();

            if (domainName == null || domainName.isEmpty())
            {
                throw new IllegalStateException(
                        "No domain name found for identity provider");
            }

            String dn =
                    String.format("CN=%s,%s", groupName,
                            getDomainDN(domainName));

            LdapMod objectClass =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_OBJECTCLASS,
                            new LdapValue[] { LdapValue
                                    .fromString(ATTR_NAME_GROUP), });
            attributeList.add(objectClass);

            LdapMod attrName =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_GROUP_NAME,
                            new LdapValue[] { LdapValue.fromString(groupName) });
            attributeList.add(attrName);

            LdapMod attrCN =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_CN,
                            new LdapValue[] { LdapValue.fromString(groupName) });
            attributeList.add(attrCN);

            LdapMod attrAccount =
                    new LdapMod(LdapModOperation.ADD, ATTR_NAME_ACCOUNT,
                            new LdapValue[] { LdapValue.fromString(groupName) });
            attributeList.add(attrAccount);

            String desc = detail.getDescription();
            if (null != desc && !desc.isEmpty())
            {
                LdapMod attrDescription =
                        new LdapMod(LdapModOperation.ADD, ATTR_DESCRIPTION,
                                new LdapValue[] { LdapValue.fromString(desc) });
                attributeList.add(attrDescription);
            }

            connection.addObject(dn,
                    attributeList.toArray(new LdapMod[attributeList.size()]));

            return newGroupId;
        } catch (AlreadyExistsLdapException e)
        {
            // The group already exists.
            throw new InvalidPrincipalException(String.format(
                    "group %s already exists", groupName), newGroupId.getUPN(), e);
        }
    }

    @Override
    public boolean addGroupToGroup(PrincipalId groupId, String groupName)
            throws Exception
    {
        ValidateUtil.validateNotNull(groupId, "groupId");
        ValidateUtil.validateNotEmpty(groupName, "groupName");

        boolean added = false;
        String subGroupDn = null;
        String filter = null;
        String searchDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            if (this.isSameDomainUpn(groupId))
            {
                // Find the group to add
                String[] attrNames = { ATTR_NAME_CN };

                filter = buildQueryByGroupFilter(groupId.getName());

                // Check if searchDn should be revised
                searchDn = getTenantSearchBaseRootDN();

                ILdapMessage subGroupMessage =
                        connection.search(searchDn, LdapScope.SCOPE_SUBTREE,
                                filter, attrNames, false);

                try
                {
                    ILdapEntry[] entries = subGroupMessage.getEntries();

                    if (entries == null || entries.length != 1)
                    {
                        // Group doesn't exist or multiple user same name
                        throw new InvalidPrincipalException(
                                String.format(
                                        "group %s doesn't exist or multiple users with same name",
                                        groupId.getName()), ServerUtils.getUpn(groupId));
                    }

                    subGroupDn = entries[0].getDN();
                } finally
                {
                    subGroupMessage.close();
                }
            }
            // Handle FSPs
            else
            {
                subGroupDn = groupId.getName();
            }

            // Find the group to add a sub-group to
            String[] groupAttrNames = { ATTR_NAME_CN, ATTR_NAME_MEMBER };

            filter = buildQueryByGroupFilter(groupName);

            searchDn = getTenantSearchBaseRootDN();

            ILdapMessage groupMessage =
                    connection.search(searchDn, LdapScope.SCOPE_SUBTREE,
                            filter, groupAttrNames, false);

            try
            {
                ILdapEntry[] entries = groupMessage.getEntries();

                // Check if multiple group same name but different level
                // We probably should check the searchbaseDN
                if (entries == null || entries.length != 1)
                {
                    throw new InvalidPrincipalException(
                            String.format(
                                    "group %s doesn't exist or multiple groups same name",
                                    groupName), ServerUtils.getUpn(groupName, getDomain()) );
                }

                // to prevent adding group to itself.
                if ( subGroupDn.equalsIgnoreCase(entries[0].getDN()) )
                {
                    throw new InvalidArgumentException(
                        String.format("Group %s cannot be added to itself.", groupId.getName() )
                    );
                }

                // We found the group
                LdapValue[] values =
                        entries[0].getAttributeValues(ATTR_NAME_MEMBER);
                LdapMod attrMember = null;
                ArrayList<LdapValue> newValues = new ArrayList<LdapValue>();

                if (null == values)
                {
                    // This is the first sub-item for that group
                    // add the new member to that group
                    // Attribute "member" is optional in Lotus
                    newValues.add(LdapValue.fromString(subGroupDn));

                    attrMember =
                            new LdapMod(LdapModOperation.ADD, ATTR_NAME_MEMBER,
                                    newValues.toArray(new LdapValue[newValues
                                            .size()]));
                } else
                {
                    for (LdapValue val : values)
                    {
                        String memberDn = val.toString();

                        // Not to add the member already in the group
                        if (0 == memberDn.compareToIgnoreCase(subGroupDn))
                        {
                            // Admin api states if already in the group
                            // should return false.
                            // Therefore, setting true here for the found
                            // case.
                            throw new MemberAlreadyExistException(
                                    String.format(
                                            "group %s already has group %s as its member",
                                            groupName, subGroupDn));
                        }

                        newValues.add(val);
                    }

                    // add the new member to that group
                    newValues.add(LdapValue.fromString(subGroupDn));

                    attrMember =
                            new LdapMod(LdapModOperation.REPLACE,
                                    ATTR_NAME_MEMBER,
                                    newValues.toArray(new LdapValue[newValues
                                            .size()]));
                }

                LdapMod attributes[] = { attrMember };

                // Update with new member list
                // Today Lotus allows circular groups such as GroupA
                // contains GroupB and GroupB can contain GroupA again.
                // This isn't allowed for the admin api. Need to revisit to
                // see if check needs to be done here or can get the feature
                // from lotus.
                connection.modifyObject(entries[0].getDN(), attributes);

                added = true;
            } finally
            {
                groupMessage.close();
            }
        }

        return added;
    }

    @Override
    public boolean removeFromGroup(PrincipalId principalId, String groupName)
            throws Exception
    {
        ValidateUtil.validateNotNull(principalId, "principalId");
        ValidateUtil.validateNotEmpty(groupName, "groupName");

        String principalDn = null;
        ILdapMessage principalMessage = null;
        ILdapMessage groupMessage = null;
        boolean removed = false;
        String searchDn = null;
        String filter = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            final String userDomainName = principalId.getDomain();

            if (this.belongsToThisIdentityProvider(userDomainName))
            {
                // Find the user
                String[] attrNames = { ATTR_NAME_ACCOUNT };

                // Looks like the SSO1 admin interface does not differentiate
                // whether it is a person user or solution user or a group. This seems will
                // create a problem of if a person user has same name as solution
                // user or group.
                // In that case, we do not not know which one to delete.
                // For now we will throw error on such case, ie, if more than one users
                // come back from the search.
                filter =
                        buildQueryByUserORSrvORGroupFilter(principalId);

                // Check if searchDn should be revised
                searchDn = getTenantSearchBaseRootDN();

                principalMessage =
                        connection.search(searchDn, LdapScope.SCOPE_SUBTREE,
                                filter, attrNames, false);

                ILdapEntry[] entries = principalMessage.getEntries();

                if (entries == null || entries.length != 1)
                {
                    // Use doesn't exist or multiple user same name
                    throw new InvalidPrincipalException(
                            String.format(
                                    "user or group %s doesn't exist or multiple users with same name",
                                    principalId.getName()), principalId.getUPN());
                }

                principalDn = entries[0].getDN();
            }
            // Handle FSPs
            else
            {
                principalDn = principalId.getName();
            }

            // Find the group
            String[] groupAttrNames = { ATTR_NAME_CN, ATTR_NAME_MEMBER };

            filter = buildQueryByGroupFilter(groupName);

            searchDn = getTenantSearchBaseRootDN();

            groupMessage =
                    connection.search(searchDn, LdapScope.SCOPE_SUBTREE,
                            filter, groupAttrNames, false);

            ILdapEntry[] groupEntries = groupMessage.getEntries();

            // Check if multiple group same name but different level
            // We probably should check the searchbaseDN
            if (groupEntries == null || groupEntries.length != 1)
            {
                throw new InvalidPrincipalException(String.format(
                        "group %s doesn't exist or multiple groups same name",
                        groupName),
                        ServerUtils.getUpn(groupName, getDomain()));
            }

            // We found the group
            LdapValue[] values =
                    groupEntries[0].getAttributeValues(ATTR_NAME_MEMBER);

            boolean notIncluded = true;
            if (null != values)
            {
               for (LdapValue val : values)
               {
                  if (0 == val.toString().compareToIgnoreCase(principalDn))
                  {
                     notIncluded = false;
                     break;
                  }
               }
            }

            if (notIncluded)
            {
               logger.info(String.format(
                     "principalDn [%s] is not a member for group [%s], skipping LdapMod Op",
                     principalDn, groupName));
            }
            else
            {
               try {
                  LdapMod[] mod =
                     {new LdapMod(LdapModOperation.DELETE, ATTR_NAME_MEMBER, principalDn)};
                  connection.modifyObject(groupEntries[0].getDN(), mod);
                  removed = true;
               }
               catch (NoSuchAttributeLdapException e)
               {
                  logger.info(
                        String.format(
                            "principalDn [%s] is not a member for group [%s] -- just deleted by a different operation",
                            principalDn, groupName));
                  removed = false;
               }
            }
        }
        finally {
            if (principalMessage != null)
            {
                principalMessage.close();
            }
            if (groupMessage != null)
            {
                groupMessage.close();
            }
        }

        return removed;
    }

    @Override
    public PrincipalId updatePersonUserDetail(String accountName,
            PersonDetail detail) throws Exception
    {
        ValidateUtil.validateNotNull(detail, "detail");

        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String domainName = getDomain();

            final PrincipalId userId = new PrincipalId(accountName, domainName);
            String[] attrNames = { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME };
            String foundAccountName = null;
            String upn = null;

            String filter = buildQueryByUserFilter(userId);

            message =
                    connection.search(getTenantSearchBaseRootDN(),
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            String userDn = null;
            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length == 1)
            {
                // found the user
                userDn = entries[0].getDN();
                foundAccountName = getStringValue(entries[0].getAttributeValues(ATTR_NAME_ACCOUNT));
                upn = getOptionalStringValue(entries[0].getAttributeValues(ATTR_USER_PRINCIPAL_NAME));
            } else
            {
                // The user doesn't exist.
                // There shouldn't be the case that there will be two users
                // with same account name in lotus, this should be prevented
                // when adding a user.
                throw new InvalidPrincipalException(String.format(
                        "user %s@%s does not exists", userId.getName(), userId.getDomain()), userId.getUPN());
            }

            String lastName = detail.getLastName();
            if (lastName != null && lastName.trim().isEmpty())
            {
                lastName = null;
            }
            updateAttribute(ATTR_LAST_NAME, lastName, attributeList);

            String firstName = detail.getFirstName();
            if (firstName != null && firstName.trim().isEmpty())
            {
                firstName = null;
            }
            updateAttribute(ATTR_FIRST_NAME, firstName, attributeList);

            String email = detail.getEmailAddress();
            if (email != null && email.trim().isEmpty())
            {
                email = null;
            }
            updateAttribute(ATTR_EMAIL_ADDRESS, email, attributeList);

            String desc = detail.getDescription();
            if (desc != null && desc.trim().isEmpty())
            {
                desc = null;
            }
            updateAttribute(ATTR_DESCRIPTION, desc, attributeList);

            if (attributeList.size() > 0)
            {
                // Only update when necessary
                connection.modifyObject(userDn, attributeList
                        .toArray(new LdapMod[attributeList.size()]));
            }

            return this.getPrincipalId(upn, foundAccountName, domainName);
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }
    }

    @Override
    public PrincipalId updateServicePrincipalDetail(String userName,
            SolutionDetail detail) throws Exception
    {
        return updateServicePrincipalDetailInContainer(userName, detail,
                getServicePrincipalsDN(getDomain()));
    }

    @Override
    public PrincipalId updateServicePrincipalDetailInExternalTenant(
            String userName, SolutionDetail detail) throws Exception
    {
        return updateServicePrincipalDetailInContainer(userName, detail,
                getTenantSearchBaseRootDN());
    }

    private PrincipalId updateServicePrincipalDetailInContainer(
            String userName, SolutionDetail detail, String containerDn)
            throws Exception
    {
        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();
        ILdapMessage message = null;
        String userDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            ValidateUtil.validateNotEmpty(containerDn, "container Dn");
            String domainName = getDomain();

            String[] attrNames = { ATTR_NAME_ACCOUNT };

            String filter = buildQueryBySrvFilter(new PrincipalId(userName, this.getDomain()));

            message =
                    connection.search(containerDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length == 1)
            {
                // found the user
                userDn = entries[0].getDN();
            } else
            {
                // The user doesn't exist.
                // There shouldn't be the case that there will be two users
                // with same account name in lotus, this should be prevented
                // when adding a user.
                throw new InvalidPrincipalException(String.format(
                        "user %s already exists", userName), String.format("%s@%s", userName, getDomain()));
            }

            // We might need to delete that attribute for such case?
            // For now just use REPLACE.
            String desc = detail.getDescription();
            if (null != desc && !desc.isEmpty())
            {
                LdapMod attrDescription =
                        new LdapMod(LdapModOperation.REPLACE,
                                ATTR_SVC_DESCRIPTION,
                                new LdapValue[] { LdapValue.fromString(desc) });
                attributeList.add(attrDescription);
            }

            X509Certificate cert = detail.getCertificate();

            if (cert != null)
            {
                Principal subjectDN = cert.getSubjectX500Principal();

                // vmwSTSSubjectDn is a MUST attribute
                ValidateUtil.validateNotNull(subjectDN, "Solution userCertificate subjectDn");
                ValidateUtil.validateNotEmpty(subjectDN.getName(), "Solution userCertificate subjectDn");

                LdapMod attrSubjectDN = new LdapMod(LdapModOperation.REPLACE,
                                                    ATTR_NAME_SUBJECTDN,
                                                    new LdapValue[] { LdapValue.fromString(subjectDN.getName()) });
                attributeList.add(attrSubjectDN);

                byte[] certBytes = cert.getEncoded();

                assert (certBytes != null);

                LdapMod attrCert =
                        new LdapMod(LdapModOperation.REPLACE, ATTR_NAME_CERT,
                                new LdapValue[] { new LdapValue(certBytes) });
                attributeList.add(attrCert);
            }
            else
            {
                throw new InvalidArgumentException(String.format(
                        "updateServicePrincipal for user %s fails: solution must have certificate", userName));
            }

            if (attributeList.size() > 0)
            {
                // Only update when necessary
                connection.modifyObject(userDn, attributeList
                        .toArray(new LdapMod[attributeList.size()]));
            }

            return new PrincipalId(userName, domainName);
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }
    }

    @Override
    public PrincipalId updateGroupDetail(String groupName, GroupDetail detail)
            throws Exception
    {
        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();
        ILdapMessage message = null;
        String groupDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String domainName = getDomain();

            String[] attrNames = { ATTR_NAME_ACCOUNT };

            String filter = buildQueryByGroupFilter(groupName);

            message =
                    connection.search(getTenantSearchBaseRootDN(),
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length == 1)
            {
                // found the group
                groupDn = entries[0].getDN();
            } else
            {
                // The user doesn't exist.
                // There shouldn't be the case that there will be two users
                // with same account name in lotus, this should be prevented
                // when adding a user.
                throw new InvalidPrincipalException(String.format(
                        "user %s doesn't exist", groupName)
                        , ServerUtils.getUpn(groupName, domainName));
            }

            LdapMod attrDescription = null;

            // We might need to delete that attribute for such case.
            // For now just use REPLACE.
            String desc = detail.getDescription();
            if (null != desc && !desc.isEmpty())
            {
                attrDescription =
                        new LdapMod(LdapModOperation.REPLACE, ATTR_DESCRIPTION,
                                new LdapValue[] { LdapValue.fromString(desc) });
                attributeList.add(attrDescription);
            }

            if (attributeList.size() > 0)
            {
                // Only update when necessary
                connection.modifyObject(groupDn, attributeList
                        .toArray(new LdapMod[attributeList.size()]));
            }

            return new PrincipalId(groupName, domainName);
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }
    }

    @Override
    public void resetUserPassword(String accountName, char[] newPassword)
            throws Exception
    {
        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();
        ILdapMessage message = null;
        String userDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            final PrincipalId userId = new PrincipalId(accountName, this.getDomain());
            String[] attrNames = { ATTR_NAME_ACCOUNT };

            String filter = buildQueryByUserFilter(userId);

            message =
                    connection.search(getTenantSearchBaseRootDN(),
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length == 1)
            {
                // found the user
                userDn = entries[0].getDN();
            } else
            {
                // The user doesn't exist.
                // There shouldn't be the case that there will be two users
                // with same account name in lotus, this should be prevented
                // when adding a user.
                throw new InvalidPrincipalException(String.format(
                        "user %s@%s doesn't exist", userId.getName(), userId.getDomain()), userId.getUPN());
            }

            // This interface requires no authentication, just replace
            // Password policy isn't implemented because of Lotus support not
            // available yet.
            LdapMod attrPassword =
                    new LdapMod(LdapModOperation.REPLACE, ATTR_USER_PASSWORD,
                            new LdapValue[] {
                            // password is char array
                            LdapValue.fromString(new String(newPassword)) });
            attributeList.add(attrPassword);

            if (attributeList.size() > 0)
            {
                // Only update when necessary
                connection.modifyObject(userDn, attributeList
                        .toArray(new LdapMod[attributeList.size()]));
            }
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }
    }

    @Override
    public void resetUserPassword(String accountName, char[] currentPassword,
            char[] newPassword) throws Exception
    {
        ILdapMessage message = null;
        String userDn = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            final String domainName = getDomain();
            final PrincipalId userId = new PrincipalId(accountName, domainName);

            String[] attrNames = { ATTR_NAME_ACCOUNT };

            String filter = buildQueryByUserFilter( userId );

            message =
                    connection.search(getTenantSearchBaseRootDN(),
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length == 1)
            {
                // found the user
                userDn = entries[0].getDN();
            } else
            {
                // The user doesn't exist.
                // There shouldn't be the case that there will be two users
                // with same account name in lotus, this should be prevented
                // when adding a user.
                throw new InvalidPrincipalException(String.format(
                        "user %s@%s doesn't exist", userId.getName(), userId.getDomain()), userId.getUPN());
            }
        }
        finally
        {
            if (null != message)
            {
                message.close();
                message = null;
            }
        }

        ILdapConnectionEx connection = null;
        try
        {
            connection = this.getConnection(userDn, new String(currentPassword), AuthenticationType.PASSWORD, false);
            ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();
            LdapMod attrPassword =
                    new LdapMod(LdapModOperation.REPLACE, ATTR_USER_PASSWORD,
                            new LdapValue[] {
                            // password is char array
                            LdapValue.fromString(new String(newPassword)) });
            attributeList.add(attrPassword);

            // Only update when necessary
            connection.modifyObject(userDn, attributeList
                    .toArray(new LdapMod[attributeList.size()]));
        }
        catch (InvalidCredentialsLdapException ex)
        {
            logger.error("Original login seems invalid.", ex);
            // According to admin interface, if not valid password,
            // should throw this.
            throw new InvalidPrincipalException(String.format(
                    "Invalid login credential for user %s@%s", accountName,
                    this.getDomain()), String.format("%s@%s", accountName,
                    getDomain()));
        } finally
        {
            if ( connection != null )
            {
                connection.close();
            }
        }
    }

    private boolean isPasswordExpired(PrincipalId userId)
    {
        boolean result = false;
        try {
            this.checkUserAccountFlags(userId);
        }
        catch (PasswordExpiredException e) {
            result = true;
        } catch (Exception e) {
            result = false;
        }
        return result;
    }

    @Override
    public void deletePrincipal(String accountName) throws Exception
    {
        // Delete a principal in system domain(lotus). We have made sure that through
        // the add interfaces, not to allow creating user, solution user and group with
        // same name in the Castle 2.0 system.
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            final PrincipalId principal = new PrincipalId(accountName, this.getDomain());
            String[] attrNames = { ATTR_NAME_ACCOUNT };

            String filter = buildQueryByUserORSrvORGroupFilter(principal);

            message =
                    connection.search(getTenantSearchBaseRootDN(),
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                // According to the interface, we need to throw this exception
                // if not found.
                throw new InvalidPrincipalException(String.format(
                        "Principal name: %s@%s doesn't exist.", principal.getName(), principal.getDomain()), principal.getUPN());
            }
            else if (entries.length == 1)
            {
                String dn = entries[0].getDN();
                connection.deleteObject(dn);
            } else
            {
                // This should not happen.
                throw new IllegalStateException(
                        "Invalid state in deletePrincipal.");
            }
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }
    }

    @Override
    public void deleteJitUsers(String extIdpEntityId) throws Exception
    {
        ILdapMessage message = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_ACCOUNT };
            String filter =  String.format(JIT_USER_QUERY,
                    LdapFilterString.encode(extIdpEntityId));

            message =
                    connection.search(getTenantSearchBaseRootDN(),
                            LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length > 0)
            {
                for (ILdapEntry e : entries) {
                    connection.deleteObject(e.getDN());
                    logger.info("Deleted JIT user : "
                    + e.getAttributeValues(ATTR_NAME_ACCOUNT)[0].getString());
                }
            }
        } finally
        {
            if (null != message)
            {
                message.close();
            }
        }
    }

    @Override
    public PasswordPolicy getPasswordPolicy() throws Exception
    {
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    return readPasswordPolicy(pooledConnection.getConnection()).policy;
	}
    }

    @Override
    public void setPasswordPolicy(PasswordPolicy policy) throws Exception
    {
        writePasswordPolicy(policy);
    }

    String constructFilterToFindFsps(List<String> fspIds)
    {
        String filter = null;
        if (fspIds != null && fspIds.size() > 0)
        {
            if (fspIds.size() == 1)
            {
                StringBuilder filterSb = new StringBuilder();
                filterSb.append("(&(objectClass=foreignSecurityPrincipal)(");
                filterSb.append(ATTR_OBJECT_ID_PREFIX);
                filterSb.append(LdapFilterString.encode(fspIds.get(0)));
                filterSb.append("))");

                filter = filterSb.toString();
            }
            else
            {
                StringBuilder filterSb = new StringBuilder();
                filterSb.append("(&(objectClass=foreignSecurityPrincipal)(|");
                for (String fspId : fspIds)
                {
                    filterSb.append("(");
                    filterSb.append(ATTR_OBJECT_ID_PREFIX);
                    filterSb.append(LdapFilterString.encode(fspId));
                    filterSb.append(")");
                }
                filterSb.append("))");

                filter = filterSb.toString();
            }
        }

        return filter;
    }

    private
    String constructFilterForFindFspGroups(List<String> fspDns)
    {
        String filter = null;
        if (fspDns != null && fspDns.size() > 0)
        {
            if (fspDns.size() == 1)
            {
                StringBuilder filterSb = new StringBuilder();
                filterSb.append("(&(objectClass=group)(member=");
                filterSb.append(LdapFilterString.encode(fspDns.get(0)));
                filterSb.append("))");

                filter = filterSb.toString();
            }
            else
            {
                StringBuilder filterSb = new StringBuilder();
                filterSb.append("(&(objectClass=group)(|");
                for (String fspDn : fspDns)
                {
                    filterSb.append("(member=");
                    filterSb.append(LdapFilterString.encode(fspDn));
                    filterSb.append(")");
                }
                filterSb.append("))");

                filter = filterSb.toString();
            }
        }

        return filter;
    }
    private interface IGroupCallback<T>


    {
        T processGroupEntry(ILdapEntry entry);
    }

    private <T> List<T> getGroupsForFsps(
        List<String> fspIds, String[] groupAttributeNames,
        IGroupCallback<T> groupCallBack) throws Exception
    {
        List<T> groups = new ArrayList<T>();
        ILdapMessage fspsMessage = null;
        ILdapMessage fspGroupsMessage = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_ACCOUNT };

            // (1) retrieves a list of FSPs DNs if found any
            String filterFsps = constructFilterToFindFsps(fspIds);

            if (ServerUtils.isNullOrEmpty(filterFsps) == false)
            {

                fspsMessage = connection.search(getTenantSearchBaseRootDN(), LdapScope.SCOPE_SUBTREE,
                                                filterFsps, attrNames, false);
                ILdapEntry[] entriesFsps = fspsMessage.getEntries();
                List<String> fspDns = new ArrayList<String>();
                if (entriesFsps != null && entriesFsps.length > 0)
                {
                    for (ILdapEntry entry : entriesFsps)
                    {
                        if (entry == null) continue;
                        fspDns.add(entry.getDN());
                    }
                }

                // (2) retrieves groups that have fspDns as member
                String filterFspGroups = constructFilterForFindFspGroups(fspDns);

                if (ServerUtils.isNullOrEmpty(filterFspGroups) == false)
                {
                    fspGroupsMessage =
                            connection.search(getTenantSearchBaseRootDN(), LdapScope.SCOPE_SUBTREE,
                                              filterFspGroups, groupAttributeNames, false);
                    ILdapEntry[] entriesFspGroups = fspGroupsMessage.getEntries();
                    T groupInstance = null;
                    if (entriesFspGroups != null && entriesFspGroups.length > 0)
                    {
                        for (ILdapEntry entry : entriesFspGroups)
                        {
                            if (entry != null)
                            {
                                groupInstance = groupCallBack.processGroupEntry(entry);
                                if ( groupInstance != null )
                                {
                                    groups.add(groupInstance);
                                }
                            }
                        }
                    }

                    // TODO
                    // If we want to support nested groups in system domain
                    // For each group in groupNames, resolves its group membership
                }
            }
        }
        finally
        {
            if (null != fspsMessage)
            {
                fspsMessage.close();
            }
        }

        return groups;
    }

    // return a list of group names who has FSP members in fspIds
    // GroupName is in domainFqdn\\groupName format
    @Override
    public List<String> findGroupsForFsps(List<String> fspIds) throws Exception
    {
        return getGroupsForFsps(
            fspIds,
            new String[] {ATTR_NAME_ACCOUNT},
            new IGroupCallback<String>()
            {
                @Override
                public String processGroupEntry(ILdapEntry entry)
                {
                    String groupName = null;
                    if (entry != null)
                    {
                        String accountName = getStringValue(entry.getAttributeValues(ATTR_NAME_ACCOUNT));
                        String groupDomainName = ServerUtils.getDomainFromDN(entry.getDN());
                        groupName = groupDomainName + "\\" + accountName;
                    }
                    return groupName;
                }
            }
        );
    }

    // return a list of groups who has FSP members in fspIds
    @Override
    public List<Group> findGroupObjectsForFsps(List<String> fspIds) throws Exception
    {
        return getGroupsForFsps(
            fspIds,
            new String[] { ATTR_DESCRIPTION, ATTR_NAME_CN },
            new IGroupCallback<Group>()
            {
                @Override
                public Group processGroupEntry(ILdapEntry entry)
                {
                    Group group = null;

                    if(entry != null)
                    {
                        logger.trace(String.format("Processing group entry:%s", entry.getDN()));
                        String groupName =
                                getStringValue(entry
                                        .getAttributeValues(ATTR_NAME_CN));

                        String description =
                                getOptionalStringValue(entry
                                        .getAttributeValues(ATTR_DESCRIPTION));

                        GroupDetail detail = new GroupDetail(description);

                        PrincipalId groupId = new PrincipalId(groupName, ServerUtils.getDomainFromDN(entry.getDN()));

                        group = new Group(groupId, getPrincipalAliasId(groupName), null/*sid*/, detail);
                    }
                    return group;
                }
            }
        );
    }

    private void writePasswordPolicy(PasswordPolicy policy) throws Exception
    {
	try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            PasswordPolicyInfo curPolicy = readPasswordPolicy(connection);

            assert(curPolicy != null);

            if (!isEquals(curPolicy.policy, policy))
            {
                ArrayList<LdapMod> modList = new ArrayList<LdapMod>();

                buildPasswordPolicyModList(curPolicy.policy, policy, modList);

                connection.modifyObject(curPolicy.dn,
                        modList.toArray(new LdapMod[modList.size()]));
            }
        }
    }

    @Override
    public LockoutPolicy getLockoutPolicy() throws Exception
    {
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    return readLockoutPolicy(pooledConnection.getConnection()).policy;
	}
    }

    @Override
    public void setLockoutPolicy(LockoutPolicy policy) throws Exception
    {
        writeLockoutPolicy(policy);
    }

    private void writeLockoutPolicy(LockoutPolicy policy) throws Exception
    {
	try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            LockoutPolicyInfo curPolicy = readLockoutPolicy(connection);

            assert( curPolicy != null);

            if (!isEquals(curPolicy.policy, policy))
            {
                ArrayList<LdapMod> modList = new ArrayList<LdapMod>();

                String description = policy.getDescription();

                if (description != null && !description.isEmpty())
                {
                    modList.add(new LdapMod(LdapModOperation.REPLACE,
                            ATTR_DESCRIPTION, new LdapValue[] { new LdapValue(
                                    description) }));
                } else if (curPolicy.policy.getDescription() != null)
                {
                    modList.add(new LdapMod(LdapModOperation.DELETE,
                            ATTR_DESCRIPTION, (LdapValue[]) null));
                }

                modList.add(new LdapMod(LdapModOperation.REPLACE,
                        ATTR_NAME_PASSWORD_MAX_FAILED_ATTEMPTS,
                        new LdapValue[] { new LdapValue(policy
                                .getMaxFailedAttempts()) }));

                modList.add(new LdapMod(LdapModOperation.REPLACE,
                        ATTR_NAME_PASSWORD_FAILED_INTERVAL_SECS,
                        new LdapValue[] { new LdapValue(policy
                                .getFailedAttemptIntervalSec()) }));

                modList.add(new LdapMod(LdapModOperation.REPLACE,
                        ATTR_NAME_PASSWORD_CHANGE_AUTO_UNLOCK_INTERVAL_SECS,
                        new LdapValue[] { new LdapValue(policy
                                .getAutoUnlockIntervalSec()) }));

                connection.modifyObject(curPolicy.dn,
                        modList.toArray(new LdapMod[modList.size()]));
            }
        }
    }

    @Override
    public boolean isObjectIdCandidate(String candidate)
    {
        return candidate.isEmpty() ? false : candidate.startsWith(ATTR_OBJECT_ID_PREFIX);
    }

    @Override
    public String getObjectId(String candidate)
    {
        return (null == candidate) ? null : candidate.substring(ATTR_OBJECT_ID_PREFIX.length());
    }

    @Override
    public String getObjectIdName(String objectId)
    {
        ValidateUtil.validateNotNull(objectId, "objectId");

        return String.format("%s%s", ATTR_OBJECT_ID_PREFIX, objectId );
    }

    private PasswordPolicyInfo readPasswordPolicy(ILdapConnectionEx connection) throws Exception
    {
        String[] attrNames =
                { ATTR_DESCRIPTION, ATTR_NAME_PASSWORD_HISTORY_COUNT,
                        ATTR_NAME_PASSWORD_LIFETIME_DAYS,
                        ATTR_NAME_PASSWORD_MAX_LENGTH,
                        ATTR_NAME_PASSWORD_MIN_LENGTH,
                        ATTR_NAME_PASSWORD_ALPHABET_COUNT,
                        ATTR_NAME_PASSWORD_MIN_LOWERCASE_COUNT,
                        ATTR_NAME_PASSWORD_MIN_UPPERCASE_COUNT,
                        ATTR_NAME_PASSWORD_MIN_NUMBER_COUNT,
                        ATTR_NAME_PASSWORD_MIN_SPECIAL_CHAR_COUNT,
                        ATTR_NAME_MAX_IDENTICAL_ADJACENT_CHAR_COUNT };

        String filter = "(objectclass=vmwPasswordPolicy)";

        ILdapMessage message =
                connection.search(getTenantSearchBaseRootDN(),
                        LdapScope.SCOPE_ONE_LEVEL, filter, attrNames, false);

        try
        {
            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                throw new IDMException("Default password policy is not found");
            }

            if (entries.length > 1)
            {
                throw new IllegalStateException(
                        "Invalid number of password policy objects found");
            }

            String description =
                    getOptionalStringValue(entries[0]
                            .getAttributeValues(ATTR_DESCRIPTION));

            int passwdHistoryCount =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_HISTORY_COUNT),
                            0);

            int passwdLifetimeDays =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_LIFETIME_DAYS),
                            0);

            int passwdMaxLength =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_MAX_LENGTH),
                            0);

            int passwdMinLength =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_MIN_LENGTH),
                            0);

            int passwdMinAlphaCount =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_ALPHABET_COUNT),
                            0);

            int passwdMinUpperCaseCount =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_MIN_UPPERCASE_COUNT),
                            0);

            int passwdMinLowerCaseCount =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_MIN_LOWERCASE_COUNT),
                            0);

            int passwdMinNumericCount =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_MIN_NUMBER_COUNT),
                            0);

            int passwdMinSpecialCharCount =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_MIN_SPECIAL_CHAR_COUNT),
                            0);

            int passwdMaxAdjacentCharCount =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_MAX_IDENTICAL_ADJACENT_CHAR_COUNT),
                            0);

            PasswordPolicyInfo result =
                    new PasswordPolicyInfo(new PasswordPolicy(
                            // Change empty description to a space in order to support vCAC's non-optional PasswordPolicy description
                            (description == null || description.isEmpty()) ? " " : description,
                            passwdHistoryCount, passwdMinLength,
                            passwdMaxLength, passwdMinAlphaCount,
                            passwdMinUpperCaseCount, passwdMinLowerCaseCount,
                            passwdMinNumericCount, passwdMinSpecialCharCount,
                            passwdMaxAdjacentCharCount, passwdLifetimeDays),
                            entries[0].getDN());
            return result;
        } finally
        {
            message.close();
        }
    }

    protected void buildPasswordPolicyModList(PasswordPolicy curPolicy,
            PasswordPolicy newPolicy, ArrayList<LdapMod> modList)
    {
        String description = newPolicy.getDescription();

        if (description != null && !description.isEmpty())
        {
            modList.add(new LdapMod(LdapModOperation.REPLACE, ATTR_DESCRIPTION,
                    new LdapValue[] { new LdapValue(description) }));
        } else if (curPolicy.getDescription() != null)
        {
            modList.add(new LdapMod(LdapModOperation.DELETE, ATTR_DESCRIPTION,
                    (LdapValue[]) null));
        }

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_HISTORY_COUNT,
                new LdapValue[] { new LdapValue(newPolicy
                        .getProhibitedPreviousPasswordsCount()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_LIFETIME_DAYS,
                new LdapValue[] { new LdapValue(newPolicy
                        .getPasswordLifetimeDays()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_MAX_IDENTICAL_ADJACENT_CHAR_COUNT,
                new LdapValue[] { new LdapValue(newPolicy
                        .getMaximumAdjacentIdenticalCharacterCount()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_MIN_NUMBER_COUNT,
                new LdapValue[] { new LdapValue(newPolicy
                        .getMinimumNumericCount()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_MIN_SPECIAL_CHAR_COUNT,
                new LdapValue[] { new LdapValue(newPolicy
                        .getMinimumSpecialCharacterCount()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_MAX_LENGTH, new LdapValue[] { new LdapValue(
                        newPolicy.getMaximumLength()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_MIN_LENGTH, new LdapValue[] { new LdapValue(
                        newPolicy.getMinimumLength()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_ALPHABET_COUNT,
                new LdapValue[] { new LdapValue(newPolicy
                        .getMinimumAlphabetCount()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_MIN_LOWERCASE_COUNT,
                new LdapValue[] { new LdapValue(newPolicy
                        .getMinimumLowercaseCount()) }));

        modList.add(new LdapMod(LdapModOperation.REPLACE,
                ATTR_NAME_PASSWORD_MIN_UPPERCASE_COUNT,
                new LdapValue[] { new LdapValue(newPolicy
                        .getMinimumUppercaseCount()) }));
    }

    private LockoutPolicyInfo readLockoutPolicy(ILdapConnectionEx connection)
            throws Exception
    {
        String[] attrNames =
                { ATTR_DESCRIPTION, ATTR_NAME_PASSWORD_MAX_FAILED_ATTEMPTS,
                        ATTR_NAME_PASSWORD_FAILED_INTERVAL_SECS,
                        ATTR_NAME_PASSWORD_CHANGE_AUTO_UNLOCK_INTERVAL_SECS };

        String filter = "(objectclass=vmwLockoutPolicy)";

        ILdapMessage message =
                connection.search(getTenantSearchBaseRootDN(),
                        LdapScope.SCOPE_ONE_LEVEL, filter, attrNames, false);

        try
        {
            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length == 0)
            {
                throw new IDMException("No lockout policy defined");
            }

            if (entries.length > 1)
            {
                throw new IllegalStateException(
                        "Invalid number of password policy objects found");
            }

            String description =
                    getOptionalStringValue(entries[0]
                            .getAttributeValues(ATTR_DESCRIPTION));

            int passwdMaxFailedAttempts =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_MAX_FAILED_ATTEMPTS),
                            0);

            int passwdFailedIntervalSecs =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_FAILED_INTERVAL_SECS),
                            0);

            int passwdAutoUnlockIntervalSecs =
                    getOptionalIntegerValue(
                            entries[0]
                                    .getAttributeValues(ATTR_NAME_PASSWORD_CHANGE_AUTO_UNLOCK_INTERVAL_SECS),
                            0);

            LockoutPolicyInfo result =
                    new LockoutPolicyInfo(new LockoutPolicy(
                            // Change empty description to a space in order to support vCAC's non-optional LockoutPolicy description
                            (description == null || description.isEmpty()) ? " " : description,
                            passwdFailedIntervalSecs, passwdMaxFailedAttempts,
                            passwdAutoUnlockIntervalSecs), entries[0].getDN());

            return result;
        } finally
        {
            message.close();
        }
    }

    protected String getTenantSearchBaseRootDN()
    {
        String domainName = getDomain();

        if (domainName == null || domainName.isEmpty())
        {
            throw new IllegalStateException(
                    "No domain name found for identity provider");
        }

        return getDomainDN(domainName);
    }

    private String buildQueryByUserORSrvORGroupFilter(PrincipalId principalName)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String escapedPrincipalName = LdapFilterString.encode(GetUPN(principalName));
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedsAMAccountName = LdapFilterString.encode(principalName.getName());
            return String.format(USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT,
                    escapedPrincipalName, escapedsAMAccountName);
        }
        else
        {
            return String.format(USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL,
                    escapedPrincipalName);

        }
    }

    private String buildQueryExistenceCheckFilterFilter(String userAccountName, PrincipalId principalName)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String escapedPrincipalName = LdapFilterString.encode(GetUPN(principalName));
        String escapedDefaultUpn = LdapFilterString.encode(GetUPN(new PrincipalId(userAccountName, this.getDomain())));
        String escapedAccountName = LdapFilterString.encode(userAccountName);
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedAccountNameFromUpn = LdapFilterString.encode(principalName.getName());
            return String.format(PRINCIPAL_EXISTENCE_CHECK_DEFAULT_UPN_DOMAIN,
                    escapedPrincipalName, escapedDefaultUpn, escapedAccountName, escapedAccountNameFromUpn );
        }
        else
        {
            return String.format(PRINCIPAL_EXISTENCE_CHECK_CUSTOM_UPN_DOMAIN,
                    escapedPrincipalName, escapedDefaultUpn, escapedAccountName);

        }
    }

    private String buildQueryByUserORSrvFilter(PrincipalId principalName)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String escapedPrincipalName = LdapFilterString.encode(GetUPN(principalName));
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedsAMAccountName = LdapFilterString.encode(principalName.getName());
            return String.format(USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT,
                    escapedPrincipalName, escapedsAMAccountName);
        }
        else
        {
            return String.format(USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL,
                escapedPrincipalName);
        }
    }

    private String buildQueryByUserFilter(PrincipalId principalName)
    {
        return buildQueryByUserFilter(principalName, null);
    }

    private String buildQueryByUserFilter(PrincipalId principalName, String additionalFilter)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String escapedPrincipalName = LdapFilterString.encode(GetUPN(principalName));
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedsAMAccountName = LdapFilterString.encode(principalName.getName());
            return String.format(USER_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCT,
                    escapedPrincipalName, escapedsAMAccountName, additionalFilter!=null? additionalFilter : "");
        }
        else
        {
            return String.format(USER_PRINC_QUERY_BY_USER_PRINCIPAL,
                    escapedPrincipalName, additionalFilter!=null? additionalFilter : "");
        }
    }

    private String buildQueryBySrvFilter(PrincipalId principalName)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String escapedPrincipalName = LdapFilterString.encode(GetUPN(principalName));
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedsAMAccountName = LdapFilterString.encode(principalName.getName());
            return String.format(SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT,
                    escapedPrincipalName, escapedsAMAccountName);
        }
        else
        {
            return String.format(SVC_PRINC_QUERY_BY_USER_PRINCIPAL,
                    escapedPrincipalName);
        }
    }

    private String buildQueryByGroupFilter(String principalName)
    {
        return String.format(GROUP_PRINC_QUERY_BY_ACCOUNT,
                LdapFilterString.encode(principalName));
    }

    private String buildQueryByAttributeFilter(String attribute, String attributeValue)
    {
        return String.format(USER_OR_SVC_PRINC_QUERY_BY_ATTRIBUTE,
                LdapFilterString.encode(attribute),
                LdapFilterString.encode(attributeValue));
    }

    private String buildQueryByContainerName(String containerName) {
        return String.format(CONTAINER_QUERY_BY_NAME, containerName);
    }


    private boolean existsPrincipal(String accountName, PrincipalId principalName,
            ILdapConnectionEx connection) throws Exception
    {
        ValidateUtil.validateNotEmpty(accountName, "accountName");
        ValidateUtil.validateNotNull(principalName, "principalName");

        boolean exist = false;
        ILdapMessage principalMessage = null;

        try
        {
            String[] attrNames = { ATTR_NAME_ACCOUNT };

            // exists must check group by samAccount
            // + user (including service principal) by
            // samaccount + upn + if upn suffix = domain => upnname within samaccount
            String filter = this.buildQueryExistenceCheckFilterFilter(accountName, principalName);

            String domainName = getDomain();

            // Search from domain by default
            // since we need to get group as well
            // check if need to change.
            String searchBaseDn = getDomainDN(domainName);

            principalMessage =
                    connection.search(searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            ILdapEntry[] principalEntries = principalMessage.getEntries();

            if (principalEntries == null || principalEntries.length == 0)
            {
                exist = false;
            } else
            {
                exist = true;
            }
        } finally
        {
            if (null != principalMessage)
            {
                principalMessage.close();
            }
        }

        return exist;
    }

    private void updateAttribute(String name, String value,
            ArrayList<LdapMod> attributeList)
    {
        LdapMod attrLastName;
        LdapModOperation updateOperation =
                (null == value) ? LdapModOperation.DELETE
                        : LdapModOperation.REPLACE;
        LdapValue[] updateToValues =
                (null == value) ? null : new LdapValue[] { LdapValue
                        .fromString(value) };
        attrLastName = new LdapMod(updateOperation, name, updateToValues);
        attributeList.add(attrLastName);
    }

    private Group findGroup(ILdapConnectionEx connection, String domainName,
            LdapValue val, String searchString, String[] groupAttributes, Matcher matcher)
    {
        Group subGroup = null;

        // We only want group objects
        String subGroupFilter = GROUP_ALL_PRINC_QUERY;

        String subGroupSearchBaseDn = val.toString();
        ILdapMessage subGroupMessage =
                connection.search(subGroupSearchBaseDn, LdapScope.SCOPE_BASE,
                        subGroupFilter, GROUP_ALL_ATTRIBUTES, false);
        try
        {
            ILdapEntry[] subGroupEntries = subGroupMessage.getEntries();

            if (subGroupEntries != null)
            {
                if (subGroupEntries.length != 1)
                {
                    throw new IllegalStateException(
                            "More than one object was found");
                }

                String resultObjectSid =
                        getStringValue(subGroupEntries[0]
                                .getAttributeValues(ATTR_NAME_OBJECTSID));

                if (matcher.matches(subGroupEntries[0], groupAttributes, searchString))
                {
                    // we should change to use AccountName which
                    // is
                    // supposed to be unique. For now use CN
                    String groupName =
                            getStringValue(subGroupEntries[0]
                                    .getAttributeValues(ATTR_NAME_CN));

                    String description =
                            getOptionalStringValue(subGroupEntries[0]
                                    .getAttributeValues(ATTR_DESCRIPTION));

                    GroupDetail detail = new GroupDetail(description);

                    PrincipalId pid = new PrincipalId(groupName, domainName);

                    subGroup = new Group(pid, this.getPrincipalAliasId(groupName), resultObjectSid, detail);
                }
            }
        } finally
        {
            subGroupMessage.close();
        }

        return subGroup;
    }

    private PersonUser findUser(ILdapConnectionEx connection, String domainName,
            LdapValue val, String searchString, String[] userAttributes, Matcher matcher)
    {
        PersonUser user = null;

        // We only want user objects
        String userFilter = USER_ALL_PRINC_QUERY;

        // Search from Users by default
        String userSearchBaseDn = val.toString();

        ILdapMessage userMessage =
                connection.search(userSearchBaseDn, LdapScope.SCOPE_BASE,
                        userFilter, USER_ALL_ATTRIBUTES, false);

        try
        {
            ILdapEntry[] userEntries = userMessage.getEntries();

            if (userEntries != null)
            {
                if (userEntries.length != 1)
                {
                    throw new IllegalStateException(
                            "More than one user was found");
                }

                if (matcher.matches(userEntries[0], userAttributes, searchString))
                {

                    String resultObjectSid =
                            getStringValue(userEntries[0]
                                    .getAttributeValues(ATTR_NAME_OBJECTSID));

                    String accountName =
                            getStringValue(userEntries[0]
                                    .getAttributeValues(ATTR_NAME_ACCOUNT));

                    String upn =
                            getOptionalStringValue(userEntries[0]
                                    .getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                    String description =
                            getOptionalStringValue(userEntries[0]
                                    .getAttributeValues(ATTR_DESCRIPTION));

                    String firstName =
                            getOptionalStringValue(userEntries[0]
                                    .getAttributeValues(ATTR_FIRST_NAME));

                    String lastName =
                            getOptionalStringValue(userEntries[0]
                                    .getAttributeValues(ATTR_LAST_NAME));

                    String email =
                            getOptionalStringValue(userEntries[0]
                                    .getAttributeValues(ATTR_EMAIL_ADDRESS));

                    PersonDetail detail =
                            new PersonDetail.Builder().firstName(firstName)
                                    .lastName(lastName).emailAddress(email)
                                    .userPrincipalName(upn)
                                    .description(description).build();

                    PrincipalId pid =
                            this.getPrincipalId(upn, accountName, domainName);

                    int flag =
                            getOptionalIntegerValue(
                                    userEntries[0]
                                            .getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                                    0);

                    boolean locked = ((flag & USER_ACCT_LOCKED_FLAG) != 0);
                    boolean disabled = ((flag & USER_ACCT_DISABLED_FLAG) != 0);

                    user =
                            new PersonUser(pid, this.getPrincipalAliasId(accountName), resultObjectSid, detail,
                                    disabled, locked);
                }
            }
        } finally
        {
            if (userMessage != null)
            {
                userMessage.close();
            }
        }

        return user;
    }

    private static <T> boolean isEquals(T src, T dst)
    {
        return (src == dst) || (src == null && dst == null)
                || (src != null && src.equals(dst));
    }

    private long getPwdLifeTime()
    {
        long pwdLifeTime;
        PasswordPolicy pwdPolicy = null;
        try {
            pwdPolicy = getPasswordPolicy();
        } catch (Exception e) {
            logger.info(String
                    .format("Failed to retrieve password policy for system provider - %s", this.getDomain()));
        }

        pwdLifeTime =
                (pwdPolicy == null) ? PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE
                        : (pwdPolicy.getPasswordLifetimeDays() * TimeUnit.DAYS.toSeconds(1L));
        return pwdLifeTime;
    }

    private final class PasswordPolicyInfo
    {
        private final PasswordPolicy policy;
        private final String dn;

        private PasswordPolicyInfo(PasswordPolicy policy, String dn)
        {
            this.policy = policy;
            this.dn = dn;
        }
    }

    private final class LockoutPolicyInfo
    {
        private final LockoutPolicy policy;
        private final String dn;

        private LockoutPolicyInfo(LockoutPolicy policy, String dn)
        {
            this.policy = policy;
            this.dn = dn;
        }
    }

    @Override
    public boolean registerExternalIDPUser(String fspUserUPN) throws Exception
    {
        // add the FSP userId to default group
	try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String groupDN =
                    String.format(
                            "cn=%s, %s",
                            IdentityManager.WELLKNOWN_EXTERNALIDP_USERS_GROUP_NAME,
                            getStoreDataEx().getGroupBaseDn());

            String[] groupAttrNames = { ATTR_NAME_CN, ATTR_NAME_MEMBER };

            ILdapMessage message =
                    connection.search(groupDN, LdapScope.SCOPE_BASE, GROUP_ALL_PRINC_QUERY,
                            groupAttrNames, false);
            try
            {
                ILdapEntry[] entries = message.getEntries();
                if (entries == null || entries.length != 1)
                {
                    //something must have gone wrong...the group exists by default.
                    throw new IllegalStateException(String.format(
                            "group %s does not exist or duplicated", groupDN));
                }

                LdapValue[] values =
                        entries[0].getAttributeValues(ATTR_NAME_MEMBER);

                //check whether member already exist
                if (null != values)
                {
                    for (LdapValue val : values)
                    {
                        // Not to add the member if already there at this point of check.
                        // there could be race condition where this existing user is
                        // about to be deleted by another thread.
                        //
                        // This rare race condition, if becomes applicable scenario in the future,
                        // could be mitigate by client-initiated retry upon receiving the
                        // thrown exception.
                        if (0 == val.toString().compareToIgnoreCase(fspUserUPN))
                        {
                            throw new MemberAlreadyExistException(
                                    String.format(
                                            "group %s currently has user %s as its member",
                                            IdentityManager.WELLKNOWN_EXTERNALIDP_USERS_GROUP_NAME,
                                            fspUserUPN));
                        }
                    }
                }

                LdapMod attributes[] =
                        { new LdapMod(LdapModOperation.ADD, ATTR_NAME_MEMBER,
                                LdapValue.fromString(fspUserUPN)) };

                // add new member to the group
                try
                {
                    connection.modifyObject(entries[0].getDN(), attributes);
                } catch (AttributeOrValueExistsLdapException aovele)
                {
                    logger.info(String
                            .format("member [%s] already exists in group [%s] -- just added by a different operation",
                                    fspUserUPN, entries[0].getDN()));
                    // we will return true in this case --- deemed as successful operation.
                    return true;
                }
                return true;
            } finally
            {
                message.close();
            }
        }
    }

    @Override
    public boolean removeExternalIDPUser(String fspId) throws Exception
    {
        // remove the FSP userId from default group
	try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String groupDN =
                    String.format("cn=%s, %s",
                            IdentityManager.WELLKNOWN_EXTERNALIDP_USERS_GROUP_NAME,
                            getStoreDataEx().getGroupBaseDn());
            String[] groupAttrNames = {ATTR_NAME_CN, ATTR_NAME_MEMBER };
            ILdapMessage message =
                    connection.search(groupDN, LdapScope.SCOPE_BASE, GROUP_ALL_PRINC_QUERY,
                            groupAttrNames, false);

            try {
                ILdapEntry[] groupEntries = message.getEntries();

                if (groupEntries == null || groupEntries.length != 1)
                {
                    throw new IllegalStateException(String.format(
                            "group %s does not exist or duplicated", groupDN));
                }
                // We found the group
                LdapValue[] values =
                        groupEntries[0].getAttributeValues(ATTR_NAME_MEMBER);

                boolean memberFound = false;

                if (null != values)
                {
                   for (LdapValue val : values)
                   {
                       String memberDn = val.toString();

                       if (0 == memberDn.compareToIgnoreCase(fspId))
                       {
                           memberFound = true;
                           break;
                       }
                   }
                }

                if (memberFound)
                {
                    // If last sub-item in the group, we should remove
                    // the member attribute since it is optional.
                    LdapMod mod =
                          new LdapMod(LdapModOperation.DELETE,
                                  ATTR_NAME_MEMBER,
                                  values.length == 1? null: fspId);
                    // Update with new member list
                    connection.modifyObject(groupEntries[0].getDN(), mod);
                }
                else
                {
                    throw new InvalidPrincipalException(
                          String.format(
                                  "external IDP user %s is not registered",
                                  fspId), fspId);
                }
            }
            finally
            {
                message.close();
            }
        }

        return true;
    }

    @Override
    public PrincipalId findExternalIDPUserRegistration(PrincipalId pId)
            throws Exception
    {
        final String targetMemberStr =
                String.format("%s%s@%s", ATTR_OBJECT_ID_PREFIX,
                        pId.getName(), pId.getDomain());
        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_CN, ATTR_NAME_MEMBER };
            String searchBaseDn =
                    String.format("cn=%s, %s",
                            IdentityManager.WELLKNOWN_EXTERNALIDP_USERS_GROUP_NAME,
                            getStoreDataEx().getGroupBaseDn());

            ILdapMessage message =
                    connection.search(searchBaseDn, LdapScope.SCOPE_BASE,
                            GROUP_ALL_PRINC_QUERY, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();
                if (entries == null || entries.length != 1)
                {//group doesn't exist or duplicated group name
                    throw new IllegalStateException("default group for external IDP registration does not exist");
                }
                LdapValue[] values = entries[0].getAttributeValues(ATTR_NAME_MEMBER);
                if (values != null)
                {
                    for (LdapValue val : values)
                    {
                        if (val.toString().equalsIgnoreCase(targetMemberStr))
                        {
                            return pId;
                        }
                    }
                }
            } finally
            {
                message.close();
            }
        }

        return null; //not found
    }

    @Override
    public PrincipalId findActiveUser(String attributeName,
            String attributeValue) throws Exception
    {
        Validate.notEmpty(attributeName, "attributeName");
        Validate.notEmpty(attributeValue, "attributeValue");

        PrincipalId userId = null;

        String filter = null;
        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_NAME_ACCOUNT_FLAGS };

            filter = buildQueryByAttributeFilter(attributeName, attributeValue);

            String searchBaseDn = getUsersDN(this.getDomain());

            ILdapMessage message =
                connection.search(
                    searchBaseDn, LdapScope.SCOPE_SUBTREE,
                            filter, attrNames, false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if ( (entries != null) && (entries.length > 1) )
                {//pass in informational data
                    throw new InvalidPrincipalException(
                        String.format("Cannot uniquely identify user principal by [%s]", filter), filter
                    );
                }
                else if ( ( entries != null) && (entries.length == 1) )
                {
                    String accountName =
                        getStringValue(entries[0].getAttributeValues(ATTR_NAME_ACCOUNT));

                    String userPrincipalName =
                            getOptionalStringValue(entries[0].getAttributeValues(ATTR_USER_PRINCIPAL_NAME));

                    int currentFlag =
                        getOptionalIntegerValue(
                            entries[0].getAttributeValues(ATTR_NAME_ACCOUNT_FLAGS),
                            0
                        );

                    if ( IsAccountActive(currentFlag) )
                    {
                        userId = this.getPrincipalId(userPrincipalName, accountName, this.getDomain());
                    }
                    else
                    {
                        throw new InvalidPrincipalException(String.format(
                                "User account '%s@%s' is not active. ",
                                accountName, this.getDomain()), String.format(
                                "%s@%s", accountName, getDomain()));
                    }
                }

            } finally
            {
                message.close();
            }
        }
        catch (NoSuchObjectLdapException e)
        {
            logger.debug(
                String.format(
                    "findActiveUser([%s], [%s]) failed with [%s]",
                    ((attributeName!= null)? attributeName : "(NULL)"),
                    ((attributeValue!=null)?attributeValue:"(NULL)"),
                    e.getMessage()
                ),
                e
            );
            throw new InvalidPrincipalException(String.format(
                "Failed to find active user with error [%s]", e.getMessage()), filter);
        }

        return userId;
    }

    /*
     * VmwareDirectoryProvider does not support certificate authentication via altSecurityIdentities.
     * So this impl does not support return additionalAttribute.
     * Thus it ignore the userDomain input. Instead route to findActiveUser() call.
     */
    @Override
    public UserSet findActiveUsersInDomain(String attributeName, String attributeValue
            , String userDomain, String additionalAttribute)
            throws Exception {
        UserSet result = new UserSet();
        PrincipalId pid = findActiveUser(attributeName, attributeValue);
        if (pid != null) {
            result.put(pid, null);
        }
        return result;
    }

    @Override
    public String getMappingSamlAttributeForGroupMembership()
    {
        Map<String, String> attrMap = getStoreDataEx().getAttributeMap();

        for (String samlAttr : attrMap.keySet())
        {
            if (ATTR_NAME_MEMBEROF.equalsIgnoreCase(attrMap.get(samlAttr)))
            {
                return samlAttr;
            }
        }
        return null;
    }

    @Override
    public Collection<SecurityDomain> getDomains() {
        Collection<SecurityDomain> domains = new HashSet<SecurityDomain>();
        domains.add(new SecurityDomain(super.getDomain(), super.getAlias()));
        return domains;
    }

    @Override
    public Group getEveryoneGroup()
    {
        return this._everyoneGroup;
    }

    protected PrincipalId getPrincipalId(String upn, String accountName, String domainName)
    {
        return ServerUtils.getPrincipalId(upn, accountName, domainName);
    }

    protected PrincipalId getPrincipalAliasId(String accountName)
    {
        return ServerUtils.getPrincipalAliasId(accountName, this.getAlias());
    }

    @Override
    public synchronized String getDomainId() throws Exception {

        if (_domainId == null)
        {
            try (PooledLdapConnection pooledConnection = borrowConnection())
            {
                ILdapConnectionEx connection = pooledConnection.getConnection();
                String[] attrNames = { ATTR_OBJECT_GUID };
                String searchBaseDn = ServerUtils.getDomainDN(getDomain());
                String query = "(objectclass=dcObject)";

                ILdapMessage message =
                        connection.search(
                                    searchBaseDn,
                                    LdapScope.SCOPE_BASE,
                                    query,
                                    attrNames,
                                    false);

                try
                {
                    ILdapEntry[] entries = message.getEntries();

                    if (entries == null || entries.length != 1)
                    {
                        throw new IllegalStateException("failed to query domain GUID");
                    }

                    LdapValue[] values = entries[0].getAttributeValues(ATTR_OBJECT_GUID);
                    if (values == null || values.length != 1)
                    {
                        throw new IllegalStateException("failed to query domain GUID");
                    }

                    _domainId = values[0].toString();

                }
                finally
                {
                    message.close();
                }
            }
        }

        return _domainId;
    }

    @Override
    public String getSiteId() throws Exception {

	try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String   siteName = getSiteName(connection);

            String[] attrNames = { ATTR_OBJECT_GUID };
            String   query     = "(objectclass=*)";

            String searchBaseDn = String.format(
                                        "CN=%s,CN=Sites,CN=Configuration,%s",
                                        siteName,
                                        ServerUtils.getDomainDN(getDomain()));

            ILdapMessage message = connection.search(
                                                searchBaseDn,
                                                LdapScope.SCOPE_BASE,
                                                query,
                                                attrNames,
                                                false);

            try
            {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length != 1)
                {
                    throw new IllegalStateException("failed to query site id");
                }

                LdapValue[] values = entries[0].getAttributeValues(ATTR_OBJECT_GUID);
                if (values == null || values.length != 1)
                {
                    throw new IllegalStateException("failed to query site id");
                }

                String siteId = values[0].toString().trim();

                if (siteId == null || siteId.length() == 0)
                {
                    throw new IllegalStateException("found an invalid site id");
                }

                return siteId;
            }
            finally
            {
                message.close();
            }
        }
    }

    private String getSiteName(ILdapConnectionEx connection)
    {
        String[] attrNames = { ATTR_SITE };
        String searchBaseDn = "";
        String query = "(objectclass=*)";

        ILdapMessage message =
                connection.search(
                            searchBaseDn,
                            LdapScope.SCOPE_BASE,
                            query,
                            attrNames,
                            false);

        try
        {
            ILdapEntry[] entries = message.getEntries();

            if (entries == null || entries.length != 1)
            {
                throw new IllegalStateException("failed to query site id");
            }

            LdapValue[] values = entries[0].getAttributeValues(ATTR_SITE);
            if (values == null || values.length != 1)
            {
                throw new IllegalStateException("failed to query site id");
            }

            String siteName = values[0].toString().trim();

            if (siteName == null || siteName.length() == 0)
            {
                throw new IllegalStateException("found an invalid site id");
            }

            return siteName;
        }
        finally
        {
            message.close();
        }
    }

    protected static String getUsersDN(String domain)
    {
        return String.format("CN=Users,%s", getDomainDN(domain));
    }

    protected static String getServicePrincipalsDN(String domain)
    {
        return String.format("CN=ServicePrincipals,%s", getDomainDN(domain));
    }

    protected static String getDomainDN(String domain)
    {
        return ServerUtils.getDomainDN(domain);
    }

    protected static String getDomainFromDN(String dn)
    {
        return ServerUtils.getDomainFromDN(dn);
    }

    protected static abstract class Matcher {

        public boolean matches(ILdapEntry entry, String[] attributes, String searchString)
        {
            if (entry != null && attributes != null && searchString != null)
            {
                for (String attribute : attributes)
                {
                    String val = getOptionalStringValue(entry.getAttributeValues(attribute));
                    if (stringMatches(val, searchString))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        protected abstract boolean stringMatches(String source, String match);
    }

    protected static Matcher getStringContainsMatcher()
    {
        if (containsMatcher == null)
        {
            containsMatcher = new Matcher()
            {

                @Override
                protected boolean stringMatches(String source, String match)
                {
                    if (source == null)
                    {
                        return false;
                    }

                     if (match.isEmpty())
                     {
                         return true;
                     }

                     // Requirement is case-insensitive search
                     return source.toLowerCase().contains(match.toLowerCase());
                }
            };
        }

        return containsMatcher;
    }

    protected static Matcher getStringStartsWithMatcher()
    {
        if (startsWithMatcher == null)
        {
            startsWithMatcher = new Matcher()
            {

                @Override
                protected boolean stringMatches(String source, String match)
                {
                    if (source == null)
                    {
                        return false;
                    }

                     if (match.isEmpty())
                     {
                         return true;
                     }

                     // Requirement is case-insensitive search
                     return source.toLowerCase().startsWith(match.toLowerCase());
                }
            };
        }

        return startsWithMatcher;
    }

    protected static void ensureAttributeSubset(String[] subset, String[] container)
    {
        HashSet<String> superset = new HashSet<String>(Arrays.asList(container));
        for (String s : subset)
        {
            Validate.isTrue(superset.contains(s), String.format("Attribute '%s' is not contained within the superset", s));
        }
    }

    @Override
    public List<VmHostData> getComputers(boolean getDCOnly) throws Exception {
       List<VmHostData> systems;
       try (PooledLdapConnection pooledConnection = borrowConnection())
       {
          ILdapConnectionEx connection = pooledConnection.getConnection();
          String domainDN = getDomainDN(getDomain());

          // Get the DCs
          String dcSearchDomain = String.format("ou=Domain Controllers,%s", domainDN);
          String dcQuery = "(objectclass=Computer)";
          systems = getVmHosts(connection, ATTR_NAME_ACCOUNT, dcSearchDomain, dcQuery, true);

          if (!getDCOnly) {
             String computerSearchDomain = String.format("ou=Computers,%s", domainDN);
             String computerQuery = "(objectclass=Computer)";
             systems.addAll(getVmHosts(connection, ATTR_NAME_ACCOUNT, computerSearchDomain, computerQuery, false));
          }

       }

       if (systems == null) {
          systems = new ArrayList<VmHostData>();
       }

       return systems;
    }

    @Override
    protected ILdapConnectionEx getConnection(Collection<String> connectStrs, boolean useGcPort)
          throws Exception
    {
       return this.getConnection(
             connectStrs,
             getUsername(),
             getPassword(),
             getAuthType(),
             useGcPort);
    }

    @Override
    protected ILdapConnectionEx getConnection(boolean useGcPort)
          throws Exception
    {
       return this.getConnection(
             getUsername(),
             getPassword(),
             getAuthType(),
             useGcPort);
    }

    @Override
    protected ILdapConnectionEx getConnection()
       throws Exception
    {
       return this.getConnection(
             getUsername(),
             getPassword(),
             getAuthType(),
             false);
    }

    private String getUsername()
    {
       String username;
       if (_isSystemDomainProvider) {
          username = IdmServerConfig.getInstance().getDirectoryConfigStoreUserName();
       } else {
          username = this.getStoreDataEx().getUserName();
       }

       return username;
    }

    private String getPassword()
    {
       String password;
       if (_isSystemDomainProvider) {
          password = IdmServerConfig.getInstance().getDirectoryConfigStorePassword();
       } else {
          password = this.getStoreDataEx().getPassword();
       }

       return password;
    }

    private AuthenticationType getAuthType()
    {
       AuthenticationType authType;
       if (_isSystemDomainProvider) {
           authType = IdmServerConfig.getInstance().getDirectoryConfigStoreAuthType();
       } else {
           authType = this.getStoreDataEx().getAuthenticationType();
       }

       return authType;
    }

    /**
     * Retrieve a list of VmHostData from an LDAP connection.
     *
     * @param connection
     *   open ldap connection
     * @param account
     *   account name to query
     * @param searchBaseDn
     *   search domain
     * @param query
     *   query
     * @param isDC
     *   flag to indicate whether the hosts being fetched are domain controllers or not
     *
     * @return list of VmHostData detailing the results of the LDAP query
     */
    private List<VmHostData> getVmHosts(ILdapConnectionEx connection, String account, String searchBaseDn, String query, boolean isDC) {
       List<VmHostData> hosts = new ArrayList<VmHostData>();
       String[] attrNames = { account };

       ILdapMessage message = connection.search(
             searchBaseDn,
             LdapScope.SCOPE_ONE_LEVEL,
             query,
             attrNames,
             false);

       try {
          ILdapEntry[] entries = message.getEntries();
          if (entries != null) {
             for (ILdapEntry entry : entries) {
                LdapValue[] values = entry.getAttributeValues(account);
                if (values == null || values.length != 1)
                {
                    throw new IllegalStateException("Failed to query host name");
                }

                String hostName = values[0].toString().trim();

                if (hostName == null || hostName.length() == 0)
                {
                    throw new IllegalStateException("Found an invalid host name");
                }

                VmHostData host = new VmHostData(hostName, isDC);

                hosts.add(host);
             }
          }
       } finally {
          message.close();
       }

       return hosts;
    }

    private String getUserDn(PrincipalId principal, boolean srpNotEnabledUserOnly) throws Exception{
        String userDn = null;
        ILdapMessage message = null;
        final String vmwSRPSecretAttrName = "vmwSRPSecret";
        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String domainName = getDomain();
            String searchBaseDn = getDomainDN(domainName);
            String filter = buildQueryByUserFilter(principal);
            String[] attrs = new String[] { vmwSRPSecretAttrName };
            message = connection.search(searchBaseDn,
                    LdapScope.SCOPE_SUBTREE, filter, attrs, true);
            ILdapEntry[] entries = message.getEntries();
            if (entries == null || entries.length == 0) {
                throw new NoSuchUserException(principal.getName());
            } else if (entries.length != 1) {
                throw new IllegalStateException("Internal error : duplicate entries were found");
            } else {
                userDn = entries[0].getDN();
                LdapValue[] values = entries[0].getAttributeValues(vmwSRPSecretAttrName);
                if(srpNotEnabledUserOnly && values != null && values.length > 0) {
                    // return null as the caller doesn't want the userDn if its vmwSRPSecret attribute is set.
                    userDn = null;
                }
            }
            return userDn;
        } finally {
            if(message != null)
                message.close();
        }
    }

    @Override
    public String getStoreUPNAttributeName() {
        return ATTR_USER_PRINCIPAL_NAME;
    }

    @Override
    public boolean doesContainerExist(String containerName) throws Exception {
        try (PooledLdapConnection pooledConnection = borrowConnection()) {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String base = String.format("cn=%s,%s", containerName, getDomainDN(getDomain()));

            String[] attributes = { ATTR_NAME_CN };

            String filter = buildQueryByContainerName(containerName);
            ILdapMessage message = connection.search(base, LdapScope.SCOPE_BASE, filter, attributes, true);

            try {
                ILdapEntry[] entries = message.getEntries();
                if (entries == null || entries.length == 0) {
                    return false;
                } else {
                    return true;
                }
            } finally {
                message.close();
            }
        } catch (NoSuchObjectLdapException e) {
            return false;
        }
    }

    @Override
    public void addContainer(String containerName) throws Exception {
        ValidateUtil.validateNotEmpty(containerName, "containerName");

        try (PooledLdapConnection pooledConnection = borrowConnection()) {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            String domain = getDomain();

            if (domain == null || domain.isEmpty()) {
                throw new IllegalStateException("No domain name found for identity provider");
            }

            String dn = String.format("CN=%s,%s", containerName, getDomainDN(domain));

            ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();
            LdapMod objectClass = new LdapMod(LdapModOperation.ADD, ATTR_NAME_OBJECTCLASS,
                    new LdapValue[] { LdapValue.fromString(ATTR_NAME_CONTAINER) });
            attributeList.add(objectClass);

            LdapMod attrCn = new LdapMod(LdapModOperation.ADD, ATTR_NAME_CN,
                    new LdapValue[] { LdapValue.fromString(containerName) });
            attributeList.add(attrCn);

            connection.addObject(dn, attributeList.toArray(new LdapMod[attributeList.size()]));
        } catch (AlreadyExistsLdapException e) {
            throw new ContainerAlreadyExistsException(
                    "Another container with the name '" + containerName + "' already exists");
        }
    }

    private PooledLdapConnection borrowConnection() throws Exception {
        return borrowConnection(getStoreDataEx().getConnectionStrings(), getUsername(), getPassword(), getAuthType(), false);
    }

}
