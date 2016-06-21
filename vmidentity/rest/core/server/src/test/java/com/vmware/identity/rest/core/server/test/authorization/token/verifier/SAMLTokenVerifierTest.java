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
package com.vmware.identity.rest.core.server.test.authorization.token.verifier;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Date;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.UriInfo;

import org.junit.Test;

import com.vmware.identity.rest.core.server.authorization.token.saml.SAMLToken;
import com.vmware.identity.rest.core.server.authorization.verifier.saml.SAMLTokenVerifier;
import com.vmware.identity.rest.core.server.test.util.SAMLTokenTestUtil;
import com.vmware.identity.rest.core.server.util.VerificationUtil;
import com.vmware.identity.rest.core.util.RequestSigner;

public class SAMLTokenVerifierTest {

    private static final String TEST_SAML_BEARER_FILENAME = "samltoken.xml";
    private static final String TEST_SAML_HOK_FILENAME = "samltoken_hok.xml";
    private static final long SKEW_TIME = 10 * 60 * 1000;

    @Test
    public void testVerification_Bearer() throws Exception {
        SAMLToken token = new SAMLToken(SAMLTokenTestUtil.getSAMLToken(TEST_SAML_BEARER_FILENAME));

        SAMLTokenVerifier verifier = new SAMLTokenVerifier(null, null, SKEW_TIME, SAMLTokenTestUtil.getCertificate());
        verifier.verify(token);
    }

    @Test
    public void testVerification_HOK() throws Exception {
        SAMLToken token = new SAMLToken(SAMLTokenTestUtil.getSAMLToken(TEST_SAML_HOK_FILENAME));
        SAMLTokenTestUtil.setConfirmationCertificate(token.getSAMLToken(), SAMLTokenTestUtil.getCertificate());

        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());
        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, SAMLTokenTestUtil.getPrivateKey());

        SAMLTokenVerifier verifier = new SAMLTokenVerifier(signedString, context, SKEW_TIME, SAMLTokenTestUtil.getCertificate());
        verifier.verify(token);
    }

    private static ContainerRequestContext createMockRequest(String uri, String method, String entity, MediaType mediaType, Date date) throws URISyntaxException {
        UriInfo uriInfo = createMock(UriInfo.class);
        expect(uriInfo.getRequestUri()).andReturn(new URI(uri)).anyTimes();
        replay(uriInfo);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getMethod()).andReturn(method).anyTimes();
        expect(context.hasEntity()).andReturn(!entity.isEmpty()).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(entity.getBytes())).anyTimes();
        context.setEntityStream(isA(InputStream.class));
        expectLastCall().anyTimes();
        expect(context.getMediaType()).andReturn(mediaType).anyTimes();
        expect(context.getDate()).andReturn(date).anyTimes();
        expect(context.getUriInfo()).andReturn(uriInfo).anyTimes();
        replay(context);

        return context;
    }

}
