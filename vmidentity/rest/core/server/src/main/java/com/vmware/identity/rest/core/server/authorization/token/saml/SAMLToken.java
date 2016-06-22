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
package com.vmware.identity.rest.core.server.authorization.token.saml;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;
import com.vmware.identity.token.impl.SamlTokenImpl;
import com.vmware.vim.sso.PrincipalId;

public class SAMLToken implements AccessToken {

    private SamlTokenImpl saml;

    /**
     * Constructs a new {@link SAMLToken}.
     *
     * @param saml the SAML object that represents this token
     */
    public SAMLToken(SamlTokenImpl saml) {
        this.saml = saml;
    }

    public SamlTokenImpl getSAMLToken() {
        return saml;
    }

    @Override
    public List<String> getAudience() {
        return new ArrayList<String>(saml.getAudience());
    }

    @Override
    public String getIssuer() {
        return saml.getIssuerNameId().getValue();
    }

    @Override
    public String getRole() {
        return null;
    }

    @Override
    public List<String> getGroupList() {
        return getNetBiosGroupList(saml.getGroupList());
    }

    @Override
    public Date getIssueTime() {
        return saml.getStartTime();
    }

    @Override
    public Date getExpirationTime() {
        return saml.getExpirationTime();
    }

    @Override
    public String getSubject() {
        return PrincipalUtil.createUPN(saml.getSubject());
    }

    @Override
    public String getTokenType() {
        return TokenType.SAML.getJsonName();
    }

    private static List<String> getNetBiosGroupList(List<PrincipalId> principals) {
        List<String> upns = new ArrayList<String>(principals.size());

        for (PrincipalId principal : principals) {
            upns.add(PrincipalUtil.createNetBios(principal));
        }

        return upns;
    }

}
