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
 */

package com.vmware.identity.idm.server.provider.vmwdirectory;

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

import javax.security.auth.login.LoginException;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.PasswordExpiredException;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.IdentityManager;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.provider.BaseLdapProvider;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.ILdapConnectionProvider;
import com.vmware.identity.idm.server.provider.IPooledConnectionProvider;
import com.vmware.identity.idm.server.provider.NoSuchGroupException;
import com.vmware.identity.idm.server.provider.NoSuchUserException;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.UserSet;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.InsufficientRightsLdapException;
import com.vmware.identity.interop.ldap.InvalidCredentialsLdapException;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.IIdmAuthStat.EventLevel;

// VMwareDirectoryProvider should eventually inherit from this one.
public abstract class VMDirProvider extends BaseLdapProvider implements
        IIdentityProvider
{
    protected static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(VMDirProvider.class);


    protected static final String ATTR_TENANTIZED_USER_PRINCIPAL_NAME = "vmwSTSTenantizedUserPrincipalName";

    protected static final String GROUP_PRINC_QUERY_BY_OBJECTSID =
        "(&(objectSid=%s)(objectClass=group))";

    protected static final String GROUP_PRINC_QUERY_BY_ACCOUNT =
        "(&(sAMAccountName=%s)(objectClass=group))";

    // This is to get all group
    // We need this do to substring match on users and groups
    protected static final String GROUP_ALL_PRINC_QUERY =
            "(objectClass=group)";

    protected static final String GROUP_ALL_BY_ACCOUNT_NAME =
            "(&(objectClass=group)(|(sAMAccountName=%1$s*)(cn=%1$s*)))";

    protected static final String GROUP_ALL_QUERY =
            "(objectClass=group)";

    // Filter to get all groups that has a specific member directly
    protected static final String GROUP_DIRECT_MEMBER_QUERY =
            "(&(objectClass=group)(member=%s))";

    protected static final String ATTR_NAME_OBJECTCLASS = "objectclass";
    protected static final String ATTR_NAME_SERVICE =
            "vmwServicePrincipal";
    protected static final String ATTR_NAME_GROUP = "group";
    protected static final String ATTR_NAME_MEMBER = "member";
    protected static final String ATTR_NAME_MEMBEROF = "memberOf";
    protected static final String ATTR_NAME_CN = "cn";
    protected static final String ATTR_NAME_ACCOUNT = "sAMAccountName";
    protected static final String ATTR_NAME_ACCOUNT_FLAGS = "userAccountControl";
    protected static final String ATTR_LAST_NAME = "sn";
    protected static final String ATTR_FIRST_NAME = "givenName";
    protected static final String ATTR_EMAIL_ADDRESS = "mail";
    protected static final String ATTR_PWD_LAST_SET = "pwdLastSet";
    protected static final String ATTR_DESCRIPTION = "description";
    protected static final String ATTR_SUBJECT_TYPE = "subjectType";
    protected static final String ATTR_USER_PRINCIPAL_NAME = "userPrincipalName";
    protected static final String ATTR_NAME_OBJECTSID = "objectSid";

    static final String SPECIAL_NAME_TO_REQUEST_USERPASSWORD = "-";
    static final String ATTR_NAME_USERPASSWORD = "userPassword";

    protected static final int USER_ACCT_DISABLED_FLAG = 0x0002;
    protected static final int USER_ACCT_LOCKED_FLAG = 0x0010;
    protected static final int USER_ACCT_PASSWORD_EXPIRED_FLAG = 0x00800000;

    protected static final String ATTR_OBJECT_ID_PREFIX = "externalObjectId=";

    protected static String[] GROUP_ALL_ATTRIBUTES = { ATTR_NAME_CN, ATTR_NAME_MEMBER, ATTR_DESCRIPTION, ATTR_NAME_OBJECTSID };
    protected static String[] USER_ALL_ATTRIBUTES = {
        ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_FIRST_NAME, ATTR_LAST_NAME,
        ATTR_EMAIL_ADDRESS, ATTR_DESCRIPTION, ATTR_NAME_ACCOUNT_FLAGS,  ATTR_NAME_OBJECTSID,
        ATTR_TENANTIZED_USER_PRINCIPAL_NAME };
    protected static String[] GROUPS_BY_CRITERIA_ATTRIBUTES = { ATTR_NAME_CN, ATTR_DESCRIPTION };
    protected static String[] GROUPS_BY_CRITERIA_FOR_NAME_ATTRIBUTES = { ATTR_NAME_CN, ATTR_DESCRIPTION };
    protected static String[] USERS_BY_CRITERIA_ATTRIBUTES = {
        ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_DESCRIPTION, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };
    protected static String[] USERS_BY_CRITERIA_FOR_NAME_ATTRIBUTES = {
        ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_FIRST_NAME, ATTR_LAST_NAME,
        ATTR_EMAIL_ADDRESS, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };

    protected static Matcher containsMatcher = null;
    protected static Matcher startsWithMatcher = null;

    static {
        ensureAttributeSubset(GROUPS_BY_CRITERIA_ATTRIBUTES, GROUP_ALL_ATTRIBUTES);
        ensureAttributeSubset(GROUPS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, GROUP_ALL_ATTRIBUTES);
        ensureAttributeSubset(USERS_BY_CRITERIA_ATTRIBUTES, USER_ALL_ATTRIBUTES);
        ensureAttributeSubset(USERS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, USER_ALL_ATTRIBUTES);
    }

    protected final Set<String> _specialAttributes;

    protected VMDirProvider(String tenantName, IIdentityStoreData store)
    {
        this( tenantName, store, null, null);
    }

    protected VMDirProvider(
        String tenantName, IIdentityStoreData store,
        IPooledConnectionProvider pooledConnectionProvider,
        ILdapConnectionProvider ldapConnectionProvider)
    {
        super(tenantName, store, null, pooledConnectionProvider, ldapConnectionProvider);

        Validate.isTrue(
                getStoreDataEx().getProviderType() == IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                "IIdentityStoreData must represent a store of "
                        + "'IDENTITY_STORE_TYPE_VMWARE_DIRECTORY' type.");

        _specialAttributes = new HashSet<String>();
        _specialAttributes.add(ATTR_SUBJECT_TYPE);
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
    public String findNomalizedPrincipalId(PrincipalId id) throws Exception {
        ValidateUtil.validateNotNull(id, "id");
        String domainName = getDomain();

        if (!belongsToThisIdentityProvider(id.getDomain())) {
            return null;
        }

        String foundPrincipalId = null;

        try (PooledLdapConnection pooledConnection = borrowConnection()) {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames = { ATTR_NAME_OBJECTCLASS, ATTR_NAME_CN, ATTR_NAME_ACCOUNT,
                    ATTR_USER_PRINCIPAL_NAME, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };
            String filter = buildQueryByUserORSrvORGroupFilter(id);

            String searchBaseDn = getTenantSearchBaseRootDN();
            try (ILdapMessage message = connection.search(searchBaseDn,
                    LdapScope.SCOPE_SUBTREE, filter, attrNames, false)) {
                ILdapEntry[] entries = message.getEntries();

                if (entries == null || entries.length == 0) {
                    return null; // return null if not found
                } else if (entries.length != 1) {
                    throw new IllegalStateException("More than one object was found");
                }

                Collection<String> objectClasses = getStringValues(entries[0]
                        .getAttributeValues(ATTR_NAME_OBJECTCLASS));

                if (objectClasses.contains(ATTR_NAME_SERVICE)) {
                    // service accounts
                    String username = getStringValue(entries[0]
                            .getAttributeValues(ATTR_NAME_ACCOUNT));
                    foundPrincipalId = new PrincipalId(username, domainName).getUPN();
                } else if (objectClasses.contains(ATTR_NAME_GROUP)) {
                    // groups
                    String groupName = getStringValue(entries[0]
                            .getAttributeValues(ATTR_NAME_CN));
                    foundPrincipalId = String.format("%s\\%s", domainName, groupName);
                } else {
                    // regular users
                    foundPrincipalId = GetUpnAttributeValue(entries[0]);
                }
            }
        }

        return foundPrincipalId;
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
                            ATTR_NAME_OBJECTSID, ATTR_PWD_LAST_SET, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };

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

                String upn = GetUpnAttributeValue(entries[0]);

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
                            ATTR_PWD_LAST_SET, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };

            String filter =
                    String.format(USER_PRINC_QUERY_BY_OBJECTSID(), LdapFilterString.encode(userObjectSid));

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

            String upn = GetUpnAttributeValue(entries[0]);

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
        return findUsersInternal(USER_ALL_PRINC_QUERY(), searchDomainName, limit, searchString, USERS_BY_CRITERIA_ATTRIBUTES, getStringContainsMatcher());
    }

    @Override
    public Set<PersonUser> findUsersByName(String searchString, String searchDomainName, int limit) throws Exception
    {
        String filter = createSearchFilter(USER_ALL_QUERY(), USER_ALL_BY_ACCOUNT_NAME(), searchString);
        return findUsersInternal(filter, searchDomainName, limit, searchString, USERS_BY_CRITERIA_FOR_NAME_ATTRIBUTES, getStringStartsWithMatcher());
    }

    protected Set<PersonUser> findUsersInternal(String groupFilter, String searchDomainName, int limit, String searchString, String[] userAttributes, Matcher matcher) throws Exception
    {
        Set<PersonUser> users = new HashSet<PersonUser>();
        PersonUser user = null;

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_DESCRIPTION, ATTR_FIRST_NAME,
                            ATTR_LAST_NAME, ATTR_EMAIL_ADDRESS,
                            ATTR_NAME_ACCOUNT_FLAGS, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };

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

                        String upn = GetUpnAttributeValue(entry);

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

    protected Set<PersonUser> findUsersInGroupInternal(PrincipalId id, int limit, String searchString, String[] userAttributes, Matcher matcher) throws Exception
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
            String searchBaseDn = getGroupsDN(domainName);

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
        Set<Group> groups = null;

        int limitNew = limit< 0 ? -1: limit;
        int limitUsers = limitNew == -1? -1: (limitNew/2+limitNew%2);
        personUsers = this.findUsers(searchString, domainName, limitUsers);
        if (limitNew != -1)
        {
            limitNew = limitNew-((personUsers!=null)? personUsers.size():0);
        }
        if (limitNew != 0)
        {
            //find groups only if needed
            groups = this.findGroups(searchString, domainName, limitNew);
        }

        return new SearchResult(personUsers, null, groups);
    }

    @Override
    public SearchResult findByName(String searchString, String domainName, int limit) throws Exception
    {
        Set<PersonUser> personUsers = null;
        Set<Group> groups = null;

        personUsers = this.findUsersByName(searchString, domainName, limit/2);
        groups = this.findGroupsByName(searchString, domainName, limit/2+limit%2);

        return new SearchResult(personUsers, null, groups);
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
                            ATTR_NAME_ACCOUNT_FLAGS, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.
            String filter = USER_ALL_PRINC_QUERY();

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

                    String upn = GetUpnAttributeValue(entry);

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
                            ATTR_NAME_ACCOUNT_FLAGS, ATTR_TENANTIZED_USER_PRINCIPAL_NAME };

            // Lotus isn't supporting *substring* matching. It only
            // supports
            // *substring or substring* matching. However we need wide char on
            // both ends.
            String filter = USER_ALL_PRINC_QUERY();

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

                String upn = GetUpnAttributeValue(entry);

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
    public boolean IsActive(PrincipalId id) throws Exception
    {
        ValidateUtil.validateNotNull(id, "id");

        int accountFlags = retrieveUserAccountFlags(id);
        return IsAccountActive(accountFlags);
    }
    protected static boolean IsAccountActive(int accountFlags)
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

    protected int retrieveUserAccountFlags(PrincipalId id) throws IDMException
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

    protected int retrieveUserAccountFlagsInContainer(PrincipalId id,
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

        try (PooledLdapConnection pooledConnection = borrowConnection())
        {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String[] attrNames =
                    { ATTR_NAME_CN, ATTR_DESCRIPTION,
                            ATTR_NAME_OBJECTSID };

            String filter = buildQueryByGroupFilter(id.getName());


            // Search from domain dn by default
            String searchBaseDn = getGroupsDN(domainName);

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
            String searchBaseDn = getGroupsDN(domainName);

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

    protected Set<Group> findGroupsInternal(String groupFilter, String searchDomainName, int limit, String searchString, String[] groupAttributes, Matcher matcher) throws Exception
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
            String searchBaseDn = getGroupsDN(domainName);

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
            String domainBaseDn = getUsersDN(domainName);
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
                        String.format(FSP_PRINC_QUERY_BY_EXTERNALID(), LdapFilterString.encode(idName));
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
            String groupSearchBaseDn = getGroupsDN(domainName);

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
            String domainBaseDn = getUsersDN(domainName);
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
                        String.format(FSP_PRINC_QUERY_BY_EXTERNALID(), LdapFilterString.encode(idName));
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
                       if (groupDnInscope(groupDn)){

                            message =
                                    connection.search(groupSearchBaseDn,
                                            LdapScope.SCOPE_BASE, filter, attrNames,
                                            false);

                            ILdapEntry[] entries = message.getEntries();

                            // if we didn't find dn - continue it
                            // could have been deleted menahhile
                            // reading attribut and then looking up member is not atomic
                            if (entries != null && entries.length > 0)
                            {
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
                            else
                            {
                                logger.info("Unable to find group with Dn='%s'", groupDn);
                            }
                       }
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

    protected Set<Group> findGroupsInGroupInternal(PrincipalId id, int limit, String searchString, String[] groupAttributes, Matcher matcher) throws Exception
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
            String searchBaseDn = getGroupsDN(domainName);

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
                        AttributeValuePair pair = new AttributeValuePair();

                        pair.setAttrDefinition(attr);

                        String attrName = attrNames.get(iAttr);

                        if (ATTR_USER_PRINCIPAL_NAME.equalsIgnoreCase(attrName)) {

                            String upn = GetUpnAttributeValue(entries[0]);

                            // userPrincipalName and value un-set => set default
                            if (ServerUtils.isNullOrEmpty(upn)) {
                                upn = accountName + "@" + this.getDomain();
                            }
                            pair.getValues().add(upn);
                        }
                        else
                        {
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
                                            try {
                                                if (this.groupDnInscope(val)) {
                                                    Group group = findGroupByDN(connection,val);
                                                    pair.getValues().add(group.getNetbios());
                                                    pairGroupSids.getValues().add(group.getObjectId());
                                                }
                                            } catch (NoSuchGroupException ex) {
                                                logger.warn("Group {} not found.", val);
                                                continue;
                                            }
                                        } else
                                        {
                                            pair.getValues().add(val);
                                        }
                                    }
                                }
                            }
                        }

                        result.add(pair);
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

        ILdapMessage message = null;
        String groupName = null;
        String groupSid = null;

        try
        {
            message =
                    connection.search(groupDN, LdapScope.SCOPE_BASE, filter,
                            attrNames, false);
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

        } catch (InsufficientRightsLdapException | NoSuchObjectLdapException e)
        {
            logger.info("Unable to find a group with the DN {}", groupDN, e);
            throw new NoSuchGroupException();
        } finally
        {
            if (message != null)
            {
                message.close();
            }
        }

        // Also whether detail can be null or not
        PrincipalId gid = new PrincipalId(groupName, getDomainFromDN(groupDN));

        return new Group(gid, this.getPrincipalAliasId(groupName), groupSid, null);
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

    protected String buildQueryByUserORSrvORGroupFilter(PrincipalId principalName)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String upn = GetUPN(principalName);
        String escapedPrincipalName = LdapFilterString.encode(upn);
        String escapedTenantizedPrincipalName = LdapFilterString.encode(GetTenantizedUPN(upn));
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedsAMAccountName = LdapFilterString.encode(principalName.getName());
            return String.format(USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT(),
                    escapedPrincipalName, escapedsAMAccountName, escapedTenantizedPrincipalName);
        }
        else
        {
            return String.format(USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL(),
                    escapedPrincipalName, escapedTenantizedPrincipalName);

        }
    }

    protected String buildQueryByUserORSrvFilter(PrincipalId principalName)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String upn = GetUPN(principalName);
        String escapedPrincipalName = LdapFilterString.encode(upn);
        String escapedTenantizedPrincipalName = LdapFilterString.encode(GetTenantizedUPN(upn));
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedsAMAccountName = LdapFilterString.encode(principalName.getName());
            return String.format(USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT(),
                    escapedPrincipalName, escapedsAMAccountName, escapedTenantizedPrincipalName);
        }
        else
        {
            return String.format(USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL(),
                escapedPrincipalName, escapedTenantizedPrincipalName);
        }
    }

    protected String buildQueryByUserFilter(PrincipalId principalName)
    {
        return buildQueryByUserFilter(principalName, null);
    }

    protected String buildQueryByUserFilter(PrincipalId principalName, String additionalFilter)
    {
        ValidateUtil.validateNotNull(principalName, "principalName");

        String upn = GetUPN(principalName);
        String escapedPrincipalName = LdapFilterString.encode(upn);
        String escapedTenantizedPrincipalName = LdapFilterString.encode(GetTenantizedUPN(upn));
        if ( this.isSameDomainUpn(principalName) )
        {
            String escapedsAMAccountName = LdapFilterString.encode(principalName.getName());
            return String.format(USER_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCT(),
                    escapedPrincipalName, escapedsAMAccountName, escapedTenantizedPrincipalName, additionalFilter!=null? additionalFilter : "");
        }
        else
        {
            return String.format(USER_PRINC_QUERY_BY_USER_PRINCIPAL(),
                    escapedPrincipalName, escapedTenantizedPrincipalName, additionalFilter!=null? additionalFilter : "");
        }
    }

    protected String buildQueryByGroupFilter(String principalName)
    {
        return String.format(GROUP_PRINC_QUERY_BY_ACCOUNT,
                LdapFilterString.encode(principalName));
    }

    protected String buildQueryByAttributeFilter(String attribute, String attributeValue)
    {
        if (ATTR_USER_PRINCIPAL_NAME.equalsIgnoreCase(attribute))
        {
            return String.format(USER_OR_SVC_PRINC_QUERY_BY_UPN_ATTRIBUTE(),
                    LdapFilterString.encode(attributeValue),
                    LdapFilterString.encode(GetTenantizedUPN(attributeValue)));
        }
        else {
            return String.format(USER_OR_SVC_PRINC_QUERY_BY_ATTRIBUTE(),
                    LdapFilterString.encode(attribute),
                    LdapFilterString.encode(attributeValue));
        }
    }

    protected Group findGroup(ILdapConnectionEx connection, String domainName,
            LdapValue val, String searchString, String[] groupAttributes, Matcher matcher)
    {
        Group subGroup = null;

        // We only want group objects
        String subGroupFilter = GROUP_ALL_PRINC_QUERY;

        String subGroupSearchBaseDn = val.toString();
        if ( groupDnInscope( subGroupSearchBaseDn ) ){
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
        }
        return subGroup;
    }

    protected PersonUser findUser(ILdapConnectionEx connection, String domainName,
            LdapValue val, String searchString, String[] userAttributes, Matcher matcher)
    {
        PersonUser user = null;

        // We only want user objects
        String userFilter = USER_ALL_PRINC_QUERY();

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

                    String upn = GetUpnAttributeValue(userEntries[0]);

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
                    { ATTR_NAME_ACCOUNT, ATTR_USER_PRINCIPAL_NAME, ATTR_NAME_ACCOUNT_FLAGS,
                    ATTR_TENANTIZED_USER_PRINCIPAL_NAME };

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

                    String userPrincipalName = GetUpnAttributeValue(entries[0]);

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
    public Collection<SecurityDomain> getDomains() {
        Collection<SecurityDomain> domains = new HashSet<SecurityDomain>();
        domains.add(new SecurityDomain(super.getDomain(), super.getAlias()));
        return domains;
    }

    @Override
    public String getStoreUPNAttributeName() {
        return ATTR_USER_PRINCIPAL_NAME;
    }

    protected PrincipalId getPrincipalId(String upn, String accountName, String domainName)
    {
        return ServerUtils.getPrincipalId(upn, accountName, domainName);
    }

    protected PrincipalId getPrincipalAliasId(String accountName)
    {
        return ServerUtils.getPrincipalAliasId(accountName, this.getAlias());
    }

    protected boolean groupDnInscope(String groupDn)
    {
        // to keep b-c
        return true;
    }

    protected String getGroupsDN(String domain)
    {
        return getDomainDN(domain);
    }

    protected String getUsersDN(String domain)
    {
        return String.format("CN=Users,%s", getDomainDN(domain));
    }

    protected String getServicePrincipalsDN(String domain)
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

    protected long getPwdLifeTime()
    {
        return PersonDetail.UNSPECIFIED_PASSWORD_LIFE_TIME_VALUE;
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

    protected abstract String getUsername();

    protected abstract String getPassword();

    protected abstract AuthenticationType getAuthType();

    protected String getUserDn(PrincipalId principal, boolean srpNotEnabledUserOnly) throws Exception{
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

    protected PooledLdapConnection borrowConnection() throws Exception {
        return borrowConnection(getStoreDataEx().getConnectionStrings(), getUsername(), getPassword(), getAuthType(), false);
    }

    protected String GetUpnAttributeValue(ILdapEntry entry)
    {
        String upn =
            getOptionalStringValue(
                entry.getAttributeValues(ATTR_USER_PRINCIPAL_NAME));
        if (ServerUtils.isNullOrEmpty(upn))
        {
            upn =
                GetUPNFromTenantizedUpn(getOptionalStringValue(
                    entry.getAttributeValues(ATTR_TENANTIZED_USER_PRINCIPAL_NAME)));
        }

        return upn;
    }

    protected boolean isObjectIdCandidate(String candidate)
    {
        return false;
    }

    /**
     * arg1 - userPrincipalName
     * arg2 - tenantizedUserPrincipalName
     * arg3 - additional filter
     */
    protected abstract String USER_PRINC_QUERY_BY_USER_PRINCIPAL();

    /**
     * arg1 - userPrincipalName
     * arg2 - sAMAccountName
     * arg3 - tenantizedUserPrincipalName
     * arg4 - additional filter
     */
    protected abstract String USER_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCT();

    protected abstract String USER_PRINC_QUERY_BY_OBJECTSID();

    /**
    * arg1 - userPrincipalName
    * arg2 - tenantizedUserPrincipalName
    */
    protected abstract String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL();
    /**
     * arg1 - userPrincipalName
     * arg2 - sAMAccountName
     * arg3 - tenantizedUserPrincipalName
     */
    protected abstract String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT();

    protected abstract String USER_OR_SVC_PRINC_QUERY_BY_ATTRIBUTE();

    protected abstract String USER_OR_SVC_PRINC_QUERY_BY_UPN_ATTRIBUTE();

    /**
    * arg1 - userPrincipalName
    * arg2 - tenantizedUserPrincipalName
    */
    protected abstract String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL();
    /**
    * arg1 - userPrincipalName
    * arg2 - sAMAccountName
    * arg3 - tenantizedUserPrincipalName
    */
    protected abstract String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT();

    //protected static final String FSP_PRINC_QUERY_BY_EXTERNALID =
    //        "(&(%s)(objectClass=foreignSecurityPrincipal))";

    // This is to get all users under Users CN
    // We need this do to substring match on users and groups
    protected abstract String USER_ALL_PRINC_QUERY();

    protected abstract String USER_ALL_BY_ACCOUNT_NAME();

    protected abstract String USER_ALL_QUERY();

    protected abstract String FSP_PRINC_QUERY_BY_EXTERNALID();
}
