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

package com.vmware.identity.openidconnect.server;

import java.net.URI;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.protocol.URIUtils;

/**
 * @author Yehia Zayour
 */
public class ClientInfoRetriever {
    private final CasIdmClient idmClient;

    public ClientInfoRetriever(CasIdmClient idmClient) {
        Validate.notNull(idmClient);
        this.idmClient = idmClient;
    }

    public ClientInfo retrieveClientInfo(String tenant, ClientID clientId) throws ServerException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notNull(clientId, "clientId");

        OIDCClient client;
        try {
            client = this.idmClient.getOIDCClient(tenant, clientId.getValue());
        } catch (NoSuchOIDCClientException e) {
            throw new ServerException(ErrorObject.invalidClient("unregistered client"), e);
        } catch (Exception e) {
            throw new ServerException(ErrorObject.serverError("idm error while retrieving client info"), e);
        }

        return new ClientInfo(
                clientId,
                uriSetFromStringList(client.getRedirectUris()),
                uriSetFromStringList(client.getPostLogoutRedirectUris()),
                client.getLogoutUri() == null ? null : uriFromString(client.getLogoutUri()),
                client.getCertSubjectDN(),
                client.getAuthnRequestClientAssertionLifetimeMS());
    }

    private static Set<URI> uriSetFromStringList(List<String> stringList) throws ServerException {
        Set<URI> uriSet = new HashSet<URI>();
        if (stringList != null) {
            for (String s : stringList) {
                uriSet.add(uriFromString(s));
            }
        }
        return uriSet;
    }

    private static URI uriFromString(String s) throws ServerException {
        try {
            return URIUtils.parseURI(s);
        } catch (ParseException e) {
            throw new ServerException(ErrorObject.serverError("failed to construct ClientInfo uri"), e);
        }
    }
}