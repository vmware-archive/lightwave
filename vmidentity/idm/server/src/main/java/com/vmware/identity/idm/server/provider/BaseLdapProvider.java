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
 * LDAP Provider Base Class
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.identity.idm.server.provider;

import java.io.Closeable;
import java.net.URI;
import java.security.InvalidParameterException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.InvalidProviderException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.LdapCertificateValidationSettings;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.IdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.NoopIdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.PerformanceMonitorFactory;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapConnectionExWithGetConnectionString;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.IIdmAuthStat.EventLevel;
import com.vmware.identity.performanceSupport.LdapQueryStat;

public abstract class BaseLdapProvider implements IIdentityProvider
{
    private static final String LDAPS_SCHEMA = "ldaps";
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(BaseLdapProvider.class);
    final private IIdentityStoreData _storeData;
    final private IIdentityStoreDataEx _storeDataEx;
    final private Collection<X509Certificate> _tenantTrustedCertificates;
    final private String tenantName;

    @Override
    public Set<String> getRegisteredUpnSuffixes()
    {
        return this._storeDataEx.getUpnSuffixes();
    }

    @Override
    public String getDomain()
    {
        return this._storeData.getName();
    }

    @Override
    public String getStoreUserHintAttributeName() throws IDMException {
        return this.getStoreDataEx().getCertUserHintAttributeName();
    }

    @Override
    public boolean getCertificateMappingUseUPN() {
        return this.getStoreDataEx().getCertLinkingUseUPN();
    }

    public void probeConnectionSettings() throws Exception
    {
        for (String connectionStr : _storeDataEx.getConnectionStrings()) {
            ValidateUtil.validateNotEmpty(connectionStr, "connectionString");

            try (ILdapConnectionEx connection = getConnection(
                    Collections.unmodifiableList(Arrays.asList(connectionStr)), false)) {
                checkDn(connection);
            }
        }
    }

    protected void checkDn(ILdapConnectionEx connection) throws InvalidProviderException
    {
      String userBaseDn = _storeDataEx.getUserBaseDn();
      String groupBaseDn = _storeDataEx.getGroupBaseDn();
      if (!ServerUtils.isValidDN(connection, userBaseDn))
      {
          String msg = String.format("DN is invalid: [%s]", userBaseDn);
          throw new InvalidProviderException(msg, "userBaseDN", userBaseDn);
      }
      if (!ServerUtils.isValidDN(connection, groupBaseDn))
      {
          String msg = String.format("DN is invalid: [%s]", groupBaseDn);
          throw new InvalidProviderException(msg, "groupBaseDN", groupBaseDn);
      }
    }

    protected BaseLdapProvider(String tenantName, IIdentityStoreData storeData)
    {
       this(tenantName, storeData, null);
    }

    protected BaseLdapProvider(String tenantName, IIdentityStoreData storeData, Collection<X509Certificate> tenantTrustedCertificates)
    {
        ValidateUtil.validateNotNull( storeData, "storeData" );
        ValidateUtil.validateNotEmpty( storeData.getName(), "storeData.getName()" );
        ValidateUtil.validateNotNull(
                storeData.getExtendedIdentityStoreData(), "storeData.getExtendedIdentityStoreData()" );

        this._storeData = storeData;
        this._storeDataEx = storeData.getExtendedIdentityStoreData();
        this._tenantTrustedCertificates = tenantTrustedCertificates;
        this.tenantName = tenantName;
    }

    protected IIdentityStoreData getStoreData() { return this._storeData; };
    protected IIdentityStoreDataEx getStoreDataEx() { return this._storeDataEx; };

    protected
    ILdapConnectionEx
    getConnection(Collection<String> connectStrs, boolean useGcPort) throws Exception
    {
        return this.getConnection(
            connectStrs,
            this.getStoreDataEx().getUserName(),
            this.getStoreDataEx().getPassword(),
            this.getStoreDataEx().getAuthenticationType(),
            useGcPort
            );
    }

    protected
    ILdapConnectionEx
    getConnection(boolean useGcPort) throws Exception
    {
        return this.getConnection(
            this.getStoreDataEx().getUserName(),
            this.getStoreDataEx().getPassword(),
            this.getStoreDataEx().getAuthenticationType(),
            useGcPort
            );
    }

    protected
    ILdapConnectionEx
    getConnection() throws Exception
    {
        return this.getConnection(
            this.getStoreDataEx().getUserName(),
            this.getStoreDataEx().getPassword(),
            this.getStoreDataEx().getAuthenticationType(),
            false
            );
    }

    protected PooledLdapConnection borrowConnection(Collection<String> connStrings, boolean useGc) throws Exception {
	IIdentityStoreDataEx storeData = this.getStoreDataEx();
	return borrowConnection(connStrings, storeData.getUserName(), storeData.getPassword(),
		storeData.getAuthenticationType(), useGc);
    }

    protected PooledLdapConnection borrowConnection(Collection<String> connStrings, String userName, String password, AuthenticationType authType,
	    boolean useGc) throws Exception {

	Exception latestEx = null;
	final LdapConnectionPool ldapConnectionPool = LdapConnectionPool.getInstance();

	Collection<URI> connectionUris = ServerUtils.toURIObjects(connStrings);
	for (URI connectionString : connectionUris) {
	    PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(
		    connectionString.toString(), authType);
	    builder.setTenantName(tenantName)
	           .setUsername(userName)
	           .setPassword(password)
	           .setUseGCPort(useGc);

	    boolean isLdaps = connectionString.getScheme().equalsIgnoreCase(LDAPS_SCHEMA);
	    if (isLdaps) {
		builder.setIdsTrustedCertificates(this.getStoreDataEx().getCertificates());
		builder.setTenantTrustedCertificates(this._tenantTrustedCertificates).build();
	    }
	    PooledLdapConnectionIdentity pooledLdapConnectionIdentity = builder.build();

	    try {
		ILdapConnectionEx conn = ldapConnectionPool.borrowConnection(pooledLdapConnectionIdentity);
		return new PooledLdapConnection(conn, pooledLdapConnectionIdentity, ldapConnectionPool);
	    } catch (Exception e) {
		logger.error(e);
		latestEx = e;
	    }
	}

	throw latestEx;
    }

    protected static String getOptionalStringValue(LdapValue[] values)
    {
        if (values != null && values.length > 0)
        {
            return getStringValue(values);
        } else
        {
            return null;
        }
    }

    protected String getOptionalFirstStringValue(LdapValue[] values)
    {
        if (values != null && values.length > 0)
        {
            return getFirstStringValue(values);
        } else
        {
            return null;
        }
    }

    protected String getOptionalLastStringValue(LdapValue[] values)
    {
        if (values != null && values.length > 0)
        {
            return getLastStringValue(values);
        } else
        {
            return null;
        }
    }

    private String getLastStringValue(LdapValue[] values)
    {
        assert values != null;
        assert values.length > 0;

        int idx = values.length -1;
        return values[idx].toString();
    }

    protected int getOptionalIntegerValue(LdapValue[] values, int defaultVal)
    {
        int result = defaultVal;

        if (values != null)
        {
            if (values.length > 1)
            {
                throw new IllegalStateException(
                        String.format(
                                "Unexpected number of values found. Expected 1, Found %d",
                                values.length));
            }

            if ((values.length != 0) && !values[0].isEmpty())
            {
                result = values[0].getInt();
            }
        }

        return result;
    }

    protected long getOptionalLongValue(LdapValue[] values, long defaultVal)
    {
        long result = defaultVal;

        if (values != null)
        {
            if (values.length > 1)
            {
                throw new IllegalStateException(
                        String.format(
                                "Unexpected number of values found. Expected 1, Found %d",
                                values.length));
            }

            if ((values.length != 0) && !values[0].isEmpty())
            {
                result = values[0].getLong();
            }
        }

        return result;
    }

    protected static String getStringValue(LdapValue[] values)
    {
        if (values == null || values.length <= 0)
        {
            throw new InvalidParameterException("Null or empty values");
        }

        if (values.length != 1)
        {
            throw new IllegalStateException("It expects one value only");
        }

        return values[0].toString();
    }

    protected String getFirstStringValue(LdapValue[] values)
    {
        if (values == null || values.length <= 0)
        {
            throw new InvalidParameterException("Null or empty values");
        }

        return values[0].toString();
    }

    protected byte[] getBinaryBlobValue(LdapValue[] values)
    {
        if (values == null || values.length <= 0)
        {
            throw new InvalidParameterException("Null or empty values");
        }

        if (values.length != 1)
        {
            throw new IllegalStateException("It expects one value only");
        }

        return values[0].getValue();
    }

    protected Collection<String> getStringValues(LdapValue[] values)
    {
        if (values == null || values.length <= 0)
        {
            throw new InvalidParameterException("Null or empty values");
        }

        Collection<String> result = new ArrayList<String>(values.length);

        for (LdapValue val : values)
        {
            result.add(val.toString());
        }

        return result;
    }

    protected Collection<String> getOptionalStringValues(LdapValue[] values)
    {
        if (values != null && values.length > 0)
        {
            Collection<String> result = new ArrayList<String>(values.length);

            for (LdapValue val : values)
            {
                result.add(val.toString());
            }

            return result;
        }
        else
        {
            return Collections.emptyList();
        }
    }

    protected
    ILdapConnectionEx
    getConnection(String userName, String password, AuthenticationType authType, boolean useGcPort) throws Exception
    {
        return this.getConnection(
                this.getStoreDataEx().getConnectionStrings(),
                userName,
                password,
                authType,
                useGcPort
        );
    }

    protected
    ILdapConnectionEx
    getConnection(Collection<String> connectionStrings, String userName, String password, AuthenticationType authType, boolean useGcPort) throws Exception
    {
        return ServerUtils.getLdapConnectionByURIs(
                ServerUtils.toURIObjects(connectionStrings),
                userName,
                password,
                authType,
                useGcPort,
                new LdapCertificateValidationSettings(this.getStoreDataEx().getCertificates(), _tenantTrustedCertificates)
        );
    }

    @Override
    public String getName()
    {
        return this._storeData.getName();
    }

    @Override
    public String getAlias()
    {
        return this._storeDataEx.getAlias();
    }

    public Set<String> getUpnSuffixes()
    {
       return this._storeDataEx.getUpnSuffixes();
    }

    protected boolean containsSearchString(String source, String searchString)
    {
        // searchString already has null validation from upper layer.
        // NOTE: empty searchString is valid case
        if (source == null)
        {
            return false;
        }

        if (searchString.isEmpty())
        {
            return true;
        }

        // NOTE: Requirement is case insensitive search
        if (!source.isEmpty() && source.toLowerCase().indexOf(searchString.toLowerCase()) >= 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    protected boolean isSameDomainUpn( PrincipalId id)
    {
        return this.isSameDomainUpn(id.getDomain());
    }
    protected boolean isSameDomainUpn( String upnDomain )
    {
        return ( ( this.getDomain().equalsIgnoreCase(upnDomain) )
                 ||
                 ( (ServerUtils.isNullOrEmpty(this.getAlias()) == false)
                   &&
                   ( this.getAlias().equalsIgnoreCase(upnDomain) )
                 )
               );
    }
    protected String GetUPN(PrincipalId id)
    {
        if ( (ServerUtils.isNullOrEmpty(this.getAlias()) == false)
             &&
             ( this.getAlias().equalsIgnoreCase(id.getDomain())
             &&
             (this.getRegisteredUpnSuffixes() == null ||
              !this.getRegisteredUpnSuffixes().contains(ValidateUtil.getCanonicalUpnSuffix(id.getDomain())))
             )
           )
        {
            return id.getName() + "@" + this.getDomain();
        }
        else
        {
            return id.getName() + "@" + id.getDomain();
        }
    }

    protected boolean belongsToThisIdentityProvider( String principalDomainName )
    {
        return ( ( this.isSameDomainUpn(principalDomainName ) ) ||
                 ( (this.getRegisteredUpnSuffixes() != null) &&
                   ( this.getRegisteredUpnSuffixes().contains(ValidateUtil.getCanonicalUpnSuffix(principalDomainName)) )
                 )
               );
    }

    protected PrincipalId normalizeAliasInPrincipal(PrincipalId id)
    {
        if (this.getRegisteredUpnSuffixes() == null ||
            !this.getRegisteredUpnSuffixes().contains(ValidateUtil.getCanonicalUpnSuffix(id.getDomain())))
        {
            id = ServerUtils.normalizeAliasInPrincipal(id, this.getDomain(), this.getAlias());
        }

        return id;
    }

    // a non-null ILdapEntry is returned or InvalidPrincipalException is thrown
    protected
    ILdapEntry
    lookupAccountLdapEntry(ILdapEntry[] entries, PrincipalId id) throws InvalidPrincipalException
    {
        id = normalizeAliasInPrincipal(id);

        ILdapEntry accountLdapEntry = null;
        if (entries == null || entries.length == 0)
        {
           throw new InvalidPrincipalException(
                 String.format("Principal id not found: %s", id), id.getUPN());
        }
        else // even if entry is only 1, we should check that upn matches
        {
            boolean bDupExist = false;
            boolean bFound = false;

            // Pick the ldapEntry whose domain from ldapEntry DN matches with PrincipalId domain name
            for (ILdapEntry entry : entries)
            {
                String domainNameFromDn = ServerUtils.getDomainFromDN(entry.getDN());
                if (domainNameFromDn.equalsIgnoreCase(id.getDomain()))
                {
                    accountLdapEntry = entry;
                    if (!bFound)
                    {
                        bFound = true;
                    }
                    else
                    {
                        bDupExist = true;
                        break;
                    }
                }
            }
            if (accountLdapEntry == null)
            {
                   throw new InvalidPrincipalException(
                            String.format(
                                  "Principal id %s not found",
                                  id.getUPN()), id.getUPN());
            }

            if (bDupExist)
            {
                throw new InvalidPrincipalException(
                        String.format(
                              "Principal id %s multiple users or group with same name",
                              id.getUPN()), id.getUPN());
            }
        }

        return accountLdapEntry;
    }


    protected class AccountLdapEntryInfo
    {
        public ILdapEntry accountLdapEntry;
        private final ILdapMessage message_upn;
        private final ILdapMessage message_acct;

        public AccountLdapEntryInfo(ILdapEntry accountLdapEntry, ILdapMessage message_upn, ILdapMessage message_acct)
        {
            ValidateUtil.validateNotNull(accountLdapEntry, "Account ldap entry");
            this.accountLdapEntry = accountLdapEntry;
            this.message_upn = message_upn;
            this.message_acct = message_acct;
        }

        public void close_messages()
        {
           if (this.message_acct != null)
           {
               this.message_acct.close();
           }
           if (this.message_upn != null)
           {
               this.message_upn.close();
           }
           accountLdapEntry = null;
        }
    }

    protected class AccountLdapEntriesInfo implements Closeable
    {
        public Set<ILdapEntry> accountLdapEntries;
        private final ILdapMessage message;

        public AccountLdapEntriesInfo(Set<ILdapEntry> accountLdapEntries, ILdapMessage message)
        {
            ValidateUtil.validateNotNull(accountLdapEntries, "Account ldap entry");
            this.accountLdapEntries = accountLdapEntries;
            this.message = message;
        }

        @Override
        public void close() {
            if (this.message != null)
            {
                this.message.close();
            }
            accountLdapEntries = null;
        }
    }

    protected AccountLdapEntryInfo findAccountLdapEntry(
            ILdapConnectionEx connection, String filter_by_upn,
            String filter_by_acct, String baseDn, String attributes[],
            boolean attributesOnly, PrincipalId id)
            throws InvalidPrincipalException {

        return findAccountLdapEntry(connection, filter_by_upn, filter_by_acct,
                baseDn, attributes, attributesOnly, id, null);

    }

    // on success return non-null AccountLdapEntryInfo with non-null userLdapEntry
    // or exception is thrown
    protected
    AccountLdapEntryInfo
    findAccountLdapEntry(ILdapConnectionEx connection,
                         String filter_by_upn,
                         String filter_by_acct,
                         String baseDn,
                         String attributes[],
                         boolean attributesOnly,
                         PrincipalId id,
                         IIdmAuthStatRecorder authStatRecorder) throws InvalidPrincipalException
    {
        ILdapMessage message_acct = null;
        ILdapMessage message_upn = null;
        ILdapEntry userLdapEntry = null;

        try
        {
            long startTime = System.nanoTime();

            message_upn = connection.search(
                    baseDn,
                    LdapScope.SCOPE_SUBTREE,
                    filter_by_upn,
                    attributes,
                    attributesOnly);

            if (authStatRecorder != null) {
                authStatRecorder.add(new LdapQueryStat(filter_by_upn, baseDn,
                        getConnectionString(connection), TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime), 1));
            }

            boolean isMultiUpnsFound = false;

            try
            {
                ILdapEntry[] entries = message_upn.getEntries();
                if (entries == null || entries.length == 0 || entries.length > 1)
                {
                    if (entries != null && entries.length > 1)
                    {
                        isMultiUpnsFound = true;
                    }
                    throw new InvalidPrincipalException(
                              String.format("Principal id not found or multiple entries found for : %s", id), id.getUPN());
                }
                userLdapEntry = entries[0];
            }
            catch(InvalidPrincipalException ex)
            {
                if (isMultiUpnsFound)
                {
                    // if multiple UPNs are found, throw original exception
                    throw ex;
                }

                startTime = System.nanoTime(); // reset startTime

                // If not found with upn, Lookup user by samAccountName
                // (samAccountName is domain unique only, multi entries can exist in forest)
                //final String filter_by_acct = this.buildUserQueryWithAccountNameByPrincipalId(userId);
                message_acct = connection.search(
                          baseDn,
                          LdapScope.SCOPE_SUBTREE,
                          filter_by_acct,
                          attributes,
                          false);
                ILdapEntry[] entries = message_acct.getEntries();
                userLdapEntry = lookupAccountLdapEntry(entries, id);

                if (authStatRecorder != null) {
                    authStatRecorder.add(new LdapQueryStat(filter_by_acct, baseDn,
                            getConnectionString(connection), TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startTime), 1));
                }
            }
        }
        catch(Exception e)
        {
            throw new InvalidPrincipalException(
                    String.format("Failed to find Principal id : %s", id), id.getUPN());
        }

        return new AccountLdapEntryInfo(userLdapEntry, message_upn, message_acct);
    }

    /**
     * Extending findAccountLdapEntry, allowing multiple entries, if chosen, to
     * be returned instead of throwing if seeing more than one.
     *
     * @param connection
     * @param filter_by_attr
     *            filter with attribute that suppose to identity the user.
     * @param baseDn
     * @param attributes
     * @param attributesOnly
     * @param attrValue
     *            the attribute value associate to the filter
     *            allowMultipleEntries allow multiple entries to be found or
     *            not.
     * @param authStatRecorder
     *            can be null
     * @return AccountLdapEntryInfo if found
     * @throws InvalidPrincipalException
     *             if not found or more than allowed number of entries found.
     */
    protected AccountLdapEntriesInfo findAccountLdapEntries(ILdapConnectionEx connection, String filter_by_attr, String baseDn,
            String[] attributes, boolean attributesOnly, String attrValue, boolean allowMultipleEntries, IIdmAuthStatRecorder authStatRecorder)
            throws InvalidPrincipalException {
        ILdapMessage message = null;
        Set<ILdapEntry> entrySet = new HashSet<ILdapEntry>();
        ILdapEntry[] entries;

        try {
            long startTime = System.nanoTime();

            message = connection.search(baseDn, LdapScope.SCOPE_SUBTREE, filter_by_attr, attributes, attributesOnly);

            if (authStatRecorder != null) {
                authStatRecorder.add(new LdapQueryStat(filter_by_attr, baseDn, getConnectionString(connection), TimeUnit.NANOSECONDS.toMillis(System
                        .nanoTime() - startTime), 1));
            }

            try {
                entries = message.getEntries();
                if (entries == null || entries.length == 0) {
                    throw new InvalidPrincipalException(String.format("Principal with attribute value %s not found"), attrValue);
                } else if (!allowMultipleEntries && entries.length > 1) {
                    throw new InvalidPrincipalException(String.format("Principal can not be uniquely identified with attribute value %s"), attrValue);
                } else {
                    for (ILdapEntry entry : entries) {
                        entrySet.add(entry);
                    }
                }
            } catch (InvalidPrincipalException ex) {

                if (authStatRecorder != null) {
                    authStatRecorder.add(new LdapQueryStat(filter_by_attr, baseDn, getConnectionString(connection), TimeUnit.NANOSECONDS
                            .toMillis(System.nanoTime() - startTime), 1));
                }
                throw ex;
            }
        } catch (Exception e) {
            throw new InvalidPrincipalException(String.format("Failed to find Principal attribute value: %s", attrValue), attrValue);
        }

        return new AccountLdapEntriesInfo(entrySet, message);
    }

    /**
     *
     * @param connection
     * @param filter_by_attr
     *            filter with attribute that suppose to identity the user.
     * @param baseDn
     * @param attributes
     * @param attributesOnly
     * @param attrValue
     *            the attribute value associate to the filter
     * @param authStatRecorder
     *            can be null
     * @return AccountLdapEntriesInfo if found
     * @throws InvalidPrincipalException
     *             if not found or multiple entry were found.
     */
    protected AccountLdapEntriesInfo findAccountLdapEntry(ILdapConnectionEx connection, String filter_by_attr,
        String baseDn, String[] attributes, boolean attributesOnly, String attrValue,
        IIdmAuthStatRecorder authStatRecorder) throws InvalidPrincipalException {

        AccountLdapEntriesInfo entrySetInfo = findAccountLdapEntries(connection, filter_by_attr, baseDn, attributes, attributesOnly, attrValue,
                false,
                authStatRecorder);

        return entrySetInfo;
}

    protected class MemberDnsResult
    {
        public ArrayList<String> memberDns;
        public boolean bNoMoreEntries;

        public MemberDnsResult(ArrayList<String> memberDns, boolean bNoMoreEntries)
        {
           this.memberDns = memberDns;
           this.bNoMoreEntries = bNoMoreEntries;
        }
    }

    // This is used by AD and AD over Ldap to return member DNs found in a group in a range using a custom filter
    protected
    MemberDnsResult
    findMemberDnsInGroupInRange(ILdapConnectionEx connection, ILdapSchemaMapping adSchemaMapping, PrincipalId groupId, String searchDn, int currRange, int rangeSize)
    {
        final String ATTR_MEMBER = adSchemaMapping.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList);
        String attrMemberWithRange = String.format("%s;range=%d-%d", ATTR_MEMBER, (currRange-1)*rangeSize, currRange*rangeSize-1);
        String attrMemberWithRangeEnd = String.format("%s;range=%d-*", ATTR_MEMBER, (currRange-1)*rangeSize);
        String[] attrNames = { attrMemberWithRange };
        ArrayList<String> memberDNs = new ArrayList<String>();
        boolean bIsRangeEnd = false;

        String filter = String.format(adSchemaMapping.getGroupQueryByAccountName(), LdapFilterString.encode(groupId.getName()));

        ILdapMessage message = connection.search(searchDn, LdapScope.SCOPE_SUBTREE, filter, attrNames, false);

        try
        {
            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length > 0)
            {
                if (entries.length > 1)
                {
                    throw new IllegalStateException(
                         "Invalid number of groups found");
                }

                LdapValue[] values = entries[0].getAttributeValues(attrMemberWithRange);
                if (values == null)
                {
                    bIsRangeEnd = true;

                    // Prevent an exception in the event of an empty group
                    if (checkIfEntryHasAttribute(entries[0], attrMemberWithRangeEnd))
                    {
                        values = entries[0].getAttributeValues(attrMemberWithRangeEnd);
                    }

                }

                if (values != null)
                {
                    for (LdapValue val : values)
                    {
                        if (!val.isEmpty())
                        {
                            memberDNs.add(val.toString());
                        }
                    }
                }
            }
        }
        finally
        {
            message.close();
        }

        return new MemberDnsResult(memberDNs, bIsRangeEnd);
    }

    private boolean checkIfEntryHasAttribute(ILdapEntry entry, String attribute)
    {
        String[] attributes = entry.getAttributeNames();

        if (attributes != null)
        {
            for (String attr : attributes)
            {
                if (attr.equalsIgnoreCase(attribute))
                {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * Used to create a search filter that reverts to the default if the search string is null or empty
     *
     * @param defaultFilter
     *   default filter to default to in the event that searchString is null or empty
     * @param searchFilter
     *   search filter to use when searchString is neither null or empty
     * @param searchString
     *   search string to embed into the search filter
     * @return
     *   searchFilter with the searchString embedded into it
     */
    protected String createSearchFilter(String defaultFilter, String searchFilter, String searchString) {
        if (searchString == null || searchString.isEmpty())
        {
            return defaultFilter;
        }
        else
        {
            return String.format(searchFilter, LdapFilterString.encode(searchString));
        }
    }

    protected String getConnectionString(ILdapConnectionEx connection) {
        String connString = null;
        if (connection != null &&
            connection instanceof ILdapConnectionExWithGetConnectionString) {
            connString = ((ILdapConnectionExWithGetConnectionString) connection).getConnectionString();
        }
        return connString;
    }

    protected IIdmAuthStatRecorder createIdmAuthStatRecorderInstance(
            String tenantName, ActivityKind opType, EventLevel eventLevel, PrincipalId id) {
        return createIdmAuthStatRecorderInstance(
                tenantName, opType, eventLevel, id == null? null : id.getUPN());
    }

    protected IIdmAuthStatRecorder createIdmAuthStatRecorderInstance(
            String tenantName, ActivityKind opType, EventLevel eventLevel, String serIdentity) {
        if (PerformanceMonitorFactory.getPerformanceMonitor().getCache(tenantName).isEnabled()) {
            return new IdmAuthStatRecorder(
                    tenantName,
                    this.getClass().getSimpleName(),
                    this.getName(),
                    this._storeDataEx.getFlags(),
                    opType,
                    eventLevel,
                    serIdentity != null ? serIdentity : "",
                    PerformanceMonitorFactory.getPerformanceMonitor().summarizeLdapQueries(),
                    DiagnosticsContextFactory.getCurrentDiagnosticsContext().getCorrelationId());
        } else {
            return NoopIdmAuthStatRecorder.getInstance();
        }
    }

}
