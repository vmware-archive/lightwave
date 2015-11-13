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
package com.vmware.identity.rest.core.server.test.authorization.token.builder;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Collection;

import org.apache.commons.codec.binary.Base64;
import org.junit.Test;

import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.saml.SAMLToken;
import com.vmware.identity.rest.core.server.authorization.token.saml.SAMLTokenBuilder;
import com.vmware.identity.rest.core.server.test.util.SAMLTokenTestUtil;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;
import com.vmware.identity.token.impl.SamlTokenImpl;

public class SAMLTokenBuilderTest extends AccessTokenBuilderTest {

    private static final String TEST_SAML_FILENAME = "samltoken_unverified.xml";

    @Test
    public void testBuilding() throws Exception {
        SamlTokenImpl saml = SAMLTokenTestUtil.getSAMLToken(TEST_SAML_FILENAME);
        SAMLTokenTestUtil.setValidated(saml);

        SAMLTokenBuilder builder = new SAMLTokenBuilder();

        AccessToken token = build(saml, TokenStyle.HEADER, TokenType.SAML,builder);

        assertTrue("Token is not a SAMLBearerToken", token instanceof SAMLToken);
    }

    public AccessToken build(SamlTokenImpl saml, TokenStyle style, TokenType type, SAMLTokenBuilder builder) throws NoSuchFieldException, SecurityException, IllegalArgumentException, IllegalAccessException {
        TokenInfo info = new TokenInfo(style, type, Base64.encodeBase64String(saml.toXml().getBytes()));

        AccessToken token = builder.build(info);
        SAMLTokenTestUtil.setValidated(((SAMLToken) token).getSAMLToken());

        assertEquals("Subject does not match", PrincipalUtil.createUPN(saml.getSubject()), token.getSubject());
        assertEquals("Issuer does not match", saml.getIssuerNameId().getValue(), token.getIssuer());
        assertEquals("Issued At does not match", saml.getStartTime(), token.getIssueTime());
        assertEquals("Expiration Time does not match", saml.getExpirationTime(), token.getExpirationTime());
        assertCollectionEquals("Audience is not contained in the token", saml.getAudience(), token.getAudience());

        return token;
    }

    private static void assertCollectionEquals(String message, Collection<?> expected, Collection<?> actual) {
        for (Object e : expected) {
            if (!actual.contains(e)) {
                fail(message);
            }
        }
    }

}
