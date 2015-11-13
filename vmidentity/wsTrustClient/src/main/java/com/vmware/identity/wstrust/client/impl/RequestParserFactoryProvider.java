/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import java.util.HashMap;
import java.util.Map;

import org.w3c.dom.Element;

import com.vmware.identity.wstrust.client.CertificateCredential;
import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.GSSCredential;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.TokenCredential;
import com.vmware.identity.wstrust.client.UsernamePasswordCredential;

class RequestParserFactoryProvider {

    final static String ACQUIRE_REQUEST_NAME = "acquire";
    final static String VALIDATE_REQUEST_NAME = "validate";
    final static String RENEW_REQUEST_NAME = "renew";

    private Map<Class<?>, RequestParserAbstractFactory<Element>> singleStepAcquireProviders;
    private Map<Class<?>, RequestParserAbstractFactory<GssResult>> multiStepAcquireProviders;
    private Map<Class<?>, RequestParserAbstractFactory<Boolean>> validateProviders;
    private Map<Class<?>, RequestParserAbstractFactory<Element>> renewProviders;

    public RequestParserFactoryProvider(SecurityTokenServiceConfig stsConfig) {

        singleStepAcquireProviders = new HashMap<>();
        singleStepAcquireProviders.put(UsernamePasswordCredential.class, new AcquireTokenByUserPassParserFactory(
                stsConfig));
        singleStepAcquireProviders.put(CertificateCredential.class, new AcquireTokenByCertificateParserFactory(
                stsConfig));
        singleStepAcquireProviders.put(TokenCredential.class, new AcquireTokenByTokenParserFactory(stsConfig));

        multiStepAcquireProviders = new HashMap<>();
        multiStepAcquireProviders.put(GSSCredential.class, new AcquireTokenByGSSParserFactory(stsConfig));

        validateProviders = new HashMap<>();
        validateProviders.put(TokenCredential.class, new ValidateTokenParserFactory(stsConfig));

        renewProviders = new HashMap<>();
        renewProviders.put(TokenCredential.class, new RenewTokenParserFactory(stsConfig));
    }

    public RequestParserAbstractFactory<Element> getSingleStepAcquireCredentialParser(
            Class<? extends Credential> credentialType) {
        if (!singleStepAcquireProviders.containsKey(credentialType))
            throw new IllegalArgumentException("No Credential Parser  Factory registered for: " + credentialType);

        return singleStepAcquireProviders.get(credentialType);
    }

    public RequestParserAbstractFactory<GssResult> getMultiStepAcquireCredentialParser(
            Class<? extends Credential> credentialType) {
        if (!multiStepAcquireProviders.containsKey(credentialType))
            throw new IllegalArgumentException("No Credential Parser  Factory registered for: " + credentialType);

        return multiStepAcquireProviders.get(credentialType);
    }

    public RequestParserAbstractFactory<Boolean> getValidateCredentialParser(Class<? extends Credential> credentialType) {
        if (!validateProviders.containsKey(credentialType))
            throw new IllegalArgumentException("No Credential Parser  Factory registered for: " + credentialType);

        return validateProviders.get(credentialType);
    }

    public RequestParserAbstractFactory<Element> getRenewCredentialParser(Class<? extends Credential> credentialType) {
        if (!renewProviders.containsKey(credentialType))
            throw new IllegalArgumentException("No Credential Parser  Factory registered for: " + credentialType);

        return renewProviders.get(credentialType);
    }
}
