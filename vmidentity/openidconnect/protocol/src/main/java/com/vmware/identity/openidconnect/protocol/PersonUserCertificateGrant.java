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

package com.vmware.identity.openidconnect.protocol;

import java.io.ByteArrayInputStream;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class PersonUserCertificateGrant extends AuthorizationGrant {
    private static final GrantType GRANT_TYPE = GrantType.PERSON_USER_CERTIFICATE;

    private final X509Certificate personUserCertificate;
    private final PersonUserAssertion personUserAssertion;

    public PersonUserCertificateGrant(X509Certificate personUserCertificate, PersonUserAssertion personUserAssertion) {
        super(GRANT_TYPE);

        Validate.notNull(personUserCertificate, "personUserCertificate");
        Validate.notNull(personUserAssertion, "personUserAssertion");
        this.personUserCertificate = personUserCertificate;
        this.personUserAssertion = personUserAssertion;
    }

    public X509Certificate getPersonUserCertificate() {
        return this.personUserCertificate;
    }

    public PersonUserAssertion getPersonUserAssertion() {
        return this.personUserAssertion;
    }

    @Override
    public Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();

        parameters.put("grant_type", GRANT_TYPE.getValue());

        byte[] certBytes;
        try {
            certBytes = this.personUserCertificate.getEncoded();
        } catch (CertificateEncodingException e) {
            throw new IllegalArgumentException("failed to encode person_user_certificate", e);
        }

        parameters.put("person_user_certificate", Base64Utils.encodeToString(certBytes));
        parameters.put("person_user_assertion", this.personUserAssertion.serialize());

        return parameters;
    }

    public static PersonUserCertificateGrant parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");

        GrantType grantType = GrantType.parse(ParameterMapUtils.getString(parameters, "grant_type"));
        if (grantType != GRANT_TYPE) {
            throw new ParseException("unexpected grant_type: " + grantType.getValue());
        }

        String personUserCertificateString = ParameterMapUtils.getString(parameters, "person_user_certificate");

        byte[] certBytes = Base64Utils.decodeToBytes(personUserCertificateString);
        ByteArrayInputStream inputStream = new ByteArrayInputStream(certBytes);

        X509Certificate personUserCertificate;
        try {
            CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
            personUserCertificate = (X509Certificate) certFactory.generateCertificate(inputStream);
        } catch (CertificateException e) {
            throw new ParseException("failed to parse person_user_certificate parameter", e);
        }

        PersonUserAssertion personUserAssertion = PersonUserAssertion.parse(parameters);

        return new PersonUserCertificateGrant(personUserCertificate, personUserAssertion);
    }
}