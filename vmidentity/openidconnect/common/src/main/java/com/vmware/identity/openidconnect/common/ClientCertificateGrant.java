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

package com.vmware.identity.openidconnect.common;

import java.io.ByteArrayInputStream;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang3.Validate;

/**
 * @author Yehia Zayour
 */
public class ClientCertificateGrant extends AuthorizationGrant {
    private static final GrantType GRANT_TYPE = GrantType.CLIENT_CERTIFICATE;

    private final List<X509Certificate> clientCertificateChain;

    public ClientCertificateGrant(List<X509Certificate> clientCertificateChain) {
        super(GRANT_TYPE);

        Validate.notNull(clientCertificateChain, "clientCertificateChain");
        this.clientCertificateChain = clientCertificateChain;
    }

    public List<X509Certificate> getClientCertificateChain() {
        return this.clientCertificateChain;
    }

    @Override
    public Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("grant_type", GRANT_TYPE.getValue());

        StringBuilder sb = new StringBuilder();
        for (X509Certificate cert : this.clientCertificateChain) {
            byte[] certBytes;
            try {
                certBytes = cert.getEncoded();
            } catch (CertificateEncodingException e) {
                throw new IllegalArgumentException("failed to encode cert", e);
            }
            String certString64 = Base64Utils.encodeToString(certBytes);
            if (sb.length() > 0) {
                sb.append(' ');
            }
            sb.append(certString64);
        }

        parameters.put("client_certificate_chain", sb.toString());
        return parameters;
    }

    public static ClientCertificateGrant parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");

        GrantType grantType = GrantType.parse(ParameterMapUtils.getString(parameters, "grant_type"));
        if (grantType != GRANT_TYPE) {
            throw new ParseException("unexpected grant_type: " + grantType.getValue());
        }

        String clientCertificateChain = ParameterMapUtils.getString(parameters, "client_certificate_chain");
        String[] parts = clientCertificateChain.split(" ");

        List<X509Certificate> list = new ArrayList<X509Certificate>();
        for (String certString64 : parts) {
            byte[] certBytes = Base64Utils.decodeToBytes(certString64);
            ByteArrayInputStream inputStream = new ByteArrayInputStream(certBytes);
            try {
                CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
                X509Certificate cert = (X509Certificate) certFactory.generateCertificate(inputStream);
                list.add(cert);
            } catch (CertificateException e) {
                throw new ParseException("failed to parse client_certificate_chain parameter", e);
            }
        }

        return new ClientCertificateGrant(list);
    }
}