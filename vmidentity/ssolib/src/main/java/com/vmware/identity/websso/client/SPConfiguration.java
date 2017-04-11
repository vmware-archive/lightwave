/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client;

import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.util.List;

/**
 * Service provider configuration data structure.
 * 
 */
public class SPConfiguration {

    private String alias;

    private String entityID;

    private boolean authnRequestsSigned;

    private PrivateKey signingPrivateKey;

    private X509Certificate signingCertificate;

    private String signingAlgorithm; // algorithm url string

    private List<String> nameIDFormats;

    private List<AssertionConsumerService> assertionConsumerServices;

    private List<SingleLogoutService> singleLogoutServices;

    /**
     * Construct SPConfiguration object. This object correspond to SAML metadata
     * protocal SP EntityDescriptorType.
     * 
     * @param alias
     *            Required.
     * @param entityID
     *            Required.
     * @param authnRequestsSigned
     *            optional. if true, idp will expect the request to be signed.
     * @param signingPrivateKey
     *            Optional. used to sign messages.
     * @param signingCertificate
     *            Optional. Not used by client lib. Rather have it for
     *            completeness of SP config storage.
     * @param signingAlgorithmName
     *            Optional.
     * @param assertionConsumerServices
     *            Required.
     * @param singleLogoutServices
     *            optional.
     * @param nameIDFormats
     *            Optional. name id formats expected by idp. Not avail means no
     *            restriction.
     */
    public SPConfiguration(String alias, String entityID, boolean authnRequestsSigned, PrivateKey signingPrivateKey,
            X509Certificate signingCertificate, String signingAlgorithmName, List<String> nameIDFormats,
            List<AssertionConsumerService> assertionConsumerServices, List<SingleLogoutService> singleLogoutServices) {

        this.alias = alias;
        this.entityID = entityID;
        this.authnRequestsSigned = authnRequestsSigned;
        this.signingPrivateKey = signingPrivateKey;
        this.signingCertificate = signingCertificate;
        this.nameIDFormats = nameIDFormats;
        this.signingAlgorithm = signingAlgorithmName;
        this.assertionConsumerServices = assertionConsumerServices;
        this.singleLogoutServices = singleLogoutServices;
    }

    public void setAlias(String alias) {
        this.alias = alias;
    }

    /**
     * SP name
     */
    public String getAlias() {
        return alias;
    }

    /**
     * SP Entity URL
     */
    public void setEntityID(String entityID) {
        this.entityID = entityID;
    }

    /**
     * Entity ID
     */
    public String getEntityID() {
        return entityID;
    }

    public void setAuthnRequestsSigned(boolean authnRequestsSigned) {
        this.authnRequestsSigned = authnRequestsSigned;
    }

    /**
     * Whether the request is signed.
     */
    public boolean isAuthnRequestsSigned() {

        return authnRequestsSigned;
    }

    /**
     * SP signing key.
     */
    public void setSigningPrivateKey(PrivateKey signingPrivateKey) {
        this.signingPrivateKey = signingPrivateKey;
    }

    /**
     * SP signing key.
     */
    public PrivateKey getSigningPrivateKey() {
        return signingPrivateKey;
    }

    /**
     * Algorithm uri
     */
    public void setSigningAlgorithm(String signingAlgorithm) {
        this.signingAlgorithm = signingAlgorithm;
    }

    /**
     * Algorithm uri
     */
    public String getSigningAlgorithm() {
        return signingAlgorithm;
    }

    /**
     * @return the signingCertificate
     */
    public X509Certificate getSigningCertificate() {
        return signingCertificate;
    }

    /**
     * @param signingCertificate
     *            the signing Certificate for the SP.
     */
    public void setSigningCertificate(X509Certificate signingCertificate) {
        this.signingCertificate = signingCertificate;
    }

    /**
     * NameID format expect to be used in the athentication
     */
    public void setNameIDFormats(List<String> nameIDFormats) {
        this.nameIDFormats = nameIDFormats;
    }

    public List<String> getNameIDFormats() {
        return nameIDFormats;
    }

    /**
     * .All assertion consumer service end points
     */
    public void setAssertionConsumerServices(List<AssertionConsumerService> assertionConsumerServices) {
        this.assertionConsumerServices = assertionConsumerServices;
    }

    /**
     * .All assertion consumer service end points
     */
    public List<AssertionConsumerService> getAssertionConsumerServices() {
        return assertionConsumerServices;
    }

    /**
     * .All slo end points
     */
    public void setSingleLogoutServices(List<SingleLogoutService> singleLogoutServices) {
        this.singleLogoutServices = singleLogoutServices;
    }

    /**
     * .All slo end points
     */
    public List<SingleLogoutService> getSingleLogoutServices() {
        return singleLogoutServices;
    }

}
