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

import java.net.URISyntaxException;
import java.text.ParseException;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;
import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.OIDCClient;

/**
 * @author Yehia Zayour
 */
public class ClientInfoRetriever {
    private final IdmClient idmClient;

    public ClientInfoRetriever(IdmClient idmClient) {
        Validate.notNull(idmClient);
        this.idmClient = idmClient;
    }

    public OIDCClientInformation retrieveClientInfo(String tenant, ClientID clientId) throws ServerException {
        Validate.notEmpty(tenant, "tenant");
        Validate.notNull(clientId, "clientId");

        OIDCClient client;
        try {
            client = this.idmClient.getOIDCClient(tenant, clientId.getValue());
        } catch (NoSuchOIDCClientException e) {
            throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription("unregistered client"), e);
        } catch (Exception e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("idm error while retrieving client info"), e);
        }

        OIDCClientInformation clientInfo;
        try {
            clientInfo = OIDCClientUtils.convertToOIDCClientInformation(client);
        } catch (URISyntaxException | ParseException | com.nimbusds.oauth2.sdk.ParseException e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("error while constructing client info"), e);
        };
        return clientInfo;
    }
}
