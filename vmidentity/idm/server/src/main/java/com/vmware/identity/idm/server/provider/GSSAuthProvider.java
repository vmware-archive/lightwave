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

package com.vmware.identity.idm.server.provider;

import java.util.Collection;
import java.util.Hashtable;
import java.util.Set;

import javax.security.auth.login.LoginException;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.performance.IIdmAuthStatRecorder;
import com.vmware.identity.idm.server.performance.PerformanceMonitorFactory;
import com.vmware.identity.interop.idm.AuthResult;
import com.vmware.identity.interop.idm.AuthenticationContext;
import com.vmware.identity.interop.idm.IIdmClientLibrary;
import com.vmware.identity.interop.idm.IdmClientLibraryFactory;
import com.vmware.identity.performanceSupport.IIdmAuthStat;

public class GSSAuthProvider implements IGssAuthIdentityProvider {

    private static final String __NAME__ = "__GSS_AUTH_PROVIDER__";

    private static final GSSAuthProvider _instance = new GSSAuthProvider();

    private final Hashtable<String, AuthenticationContext> _contextCache;

    public static GSSAuthProvider getInstance() {
        return _instance;
    }

    private GSSAuthProvider()
    {
        _contextCache = new Hashtable<String, AuthenticationContext>();
    }

    @Override
    public String getName() {
        return __NAME__;
    }

    @Override
    public String getDomain() {
        return __NAME__;
    }

    @Override
    public String getAlias() {
        return null;
    }

    @Override
    public Set<String> getRegisteredUpnSuffixes() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Collection<SecurityDomain> getDomains() {
        throw new UnsupportedOperationException();
    }

    @Override
    public PrincipalId authenticate(PrincipalId principal, String password)
            throws LoginException {
        throw new UnsupportedOperationException();
    }

    @Override
    public Collection<AttributeValuePair> getAttributes(
            PrincipalId principalId, Collection<Attribute> attributes)
            throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public PersonUser findUser(PrincipalId id) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public PersonUser findUserByObjectId(String userObjectId) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<PersonUser> findUsers(String searchString, String domainName,
            int limit) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<PersonUser> findUsersByName(String searchString, String domainName,
            int limit) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<PersonUser> findUsersInGroup(PrincipalId groupId, String searchString,
           int limit) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<PersonUser> findUsersByNameInGroup(PrincipalId groupId,
          String searchString, int limit)
                throws Exception {
       throw new UnsupportedOperationException();
    }

    @Override
    public Set<PersonUser> findDisabledUsers(String searchString, int limit)
            throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<PersonUser> findLockedUsers(String searchString, int limit)
            throws Exception {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public PrincipalGroupLookupInfo findDirectParentGroups(PrincipalId principalId)
            throws Exception {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public PrincipalGroupLookupInfo findNestedParentGroups(PrincipalId userId)
            throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Group findGroup(PrincipalId groupId) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Group findGroupByObjectId(String groupObjectId) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<Group> findGroups(String searchString, String domainName,
            int limit) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<Group> findGroupsByName(String searchString, String domainName,
            int limit) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<Group> findGroupsInGroup(PrincipalId groupId, String searchString,
            int limit) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<Group> findGroupsByNameInGroup(PrincipalId groupId,
          String searchString, int limit)
                throws Exception {
       throw new UnsupportedOperationException();
    }

    @Override
    public SearchResult find(String searchString, String domainName, int limit)
            throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public SearchResult findByName(String searchString, String domainName, int limit)
            throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean IsActive(PrincipalId id) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public void checkUserAccountFlags(PrincipalId principalId)
            throws IDMException {
        throw new UnsupportedOperationException();
    }

    @Override
    public GSSAuthResult authenticate(String contextId, byte[] gssTicket)
            throws LoginException {

        ValidateUtil.validateNotEmpty(contextId, "Context Id");
        ValidateUtil.validateNotEmpty(gssTicket, "GSS Blob");

        IIdmAuthStatRecorder recorder = PerformanceMonitorFactory.createIdmAuthStatRecorderInstance(
                this.getName(),
                this.getClass().getName(),
                this.getName(),
                0,
                IIdmAuthStat.ActivityKind.AUTHENTICATE,
                IIdmAuthStat.EventLevel.INFO,
                contextId);
        recorder.start();

        IIdmClientLibrary idmAdapter =
                            IdmClientLibraryFactory.getInstance().getLibrary();

        AuthenticationContext context = _contextCache.get(contextId);
        if (context == null)
        {
            context = idmAdapter.CreateAuthenticationContext();
            _contextCache.put(contextId,  context);
        }

        try
        {
            AuthResult result = idmAdapter.AuthenticateBySSPI(context, gssTicket);

            switch (result.getStatus())
            {
                case COMPLETE:

                    com.vmware.identity.interop.idm.UserInfo userInfo =
                                            idmAdapter.getUserInfo(context);

                    context = _contextCache.remove(contextId);
                    context.close();

                    recorder.end();
                    return new GSSAuthResult(
                                    contextId,
                                    userInfo);

                case CONTINUE_NEEDED:

                    recorder.end();
                    return new GSSAuthResult(contextId, result.getGssBLOB());

                case ERROR:
                case INITIAL:
                default:

                    throw new LoginException();
            }
        }
        catch(Exception e)
        {
            context = _contextCache.remove(contextId);
            if (context != null)
            {
                context.close();
            }

            throw e;
        }
    }

    @Override
    public PrincipalId findActiveUser(String attributeName, String attributeValue) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public UserSet findActiveUsersInDomain(String attributeName, String attributeValue
            , String userDomain, String additionalAttribute)
            throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    public String getStoreUPNAttributeName() {
        throw new UnsupportedOperationException();
    }

    @Override
    public String getStoreUserHintAttributeName() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean getCertificateMappingUseUPN() {
        throw new UnsupportedOperationException();
    }

}
