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
package com.vmware.identity.rest.idm.data;

import java.util.Collection;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code RelyingPartyDTO} class contains the details of a SAML 2.0 Relying Party.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = RelyingPartyDTO.Builder.class)
public class RelyingPartyDTO extends DTO {

    private final String name;
    private final String url;
    private final Collection<SignatureAlgorithmDTO> signatureAlgorithms;
    private final Collection<AssertionConsumerServiceDTO> assertionConsumerServices;
    private final Collection<AttributeConsumerServiceDTO> attributeConsumerServices;
    private final Collection<ServiceEndpointDTO> singleLogoutServices;
    private final CertificateDTO certificate;
    private final String defaultAssertionConsumerService;
    private final String defaultAttributeConsumerService;
    private final Boolean authnRequestsSigned;

    /**
     * Constructs a {@code RelyingPartyDTO} with its various details.
     *
     * @param name name of the relying party.
     * @param url url for the relying party.
     * @param signatureAlgorithms the collection of signature algorithm used by the
     *  relying party.
     * @param assertionConsumerServices the collection of assertion consumer services.
     * @param attributeConsumerServices the collection attribute consumer services.
     * @param singleLogoutServices the collection of single logout services.
     * @param certificate the certificate used by the relying party.
     * @param defaultAssertionConsumerService the default assertion consumer service.
     * @param defaultAttributeConsumerService the default attribute consumer service.
     * @param authnRequestsSigned a boolean indicating whether authentication requests
     *  are signed.
     */
    public RelyingPartyDTO(String name, String url, Collection<SignatureAlgorithmDTO> signatureAlgorithms,
            Collection<AssertionConsumerServiceDTO> assertionConsumerServices, Collection<AttributeConsumerServiceDTO> attributeConsumerServices,
            Collection<ServiceEndpointDTO> singleLogoutServices, CertificateDTO certificate, String defaultAssertionConsumerService,
            String defaultAttributeConsumerService, Boolean authnRequestsSigned) {
        this.name = name;
        this.url = url;
        this.signatureAlgorithms = signatureAlgorithms;
        this.assertionConsumerServices = assertionConsumerServices;
        this.attributeConsumerServices = attributeConsumerServices;
        this.singleLogoutServices = singleLogoutServices;
        this.certificate = certificate;
        this.defaultAssertionConsumerService = defaultAssertionConsumerService;
        this.defaultAttributeConsumerService = defaultAttributeConsumerService;
        this.authnRequestsSigned = authnRequestsSigned;
    }

    /**
     * Get the name of the relying party.
     *
     * @return the name of the relying party.
     */
    public String getName() {
        return name;
    }

    /**
     * Get the URL of the relying party.
     *
     * @return the URL of the relying party.
     */
    public String getUrl() {
        return url;
    }

    /**
     * Get the collection of signature algorithms used by the relying party.
     *
     * @return a collection of {@link SignatureAlgorithmDTO}.
     */
    public Collection<SignatureAlgorithmDTO> getSignatureAlgorithms() {
        return signatureAlgorithms;
    }

    /**
     * Get the collection of assertion consumer services registered with the relying party.
     *
     * @return a collection of {@link AssertionConsumerServiceDTO}.
     */
    public Collection<AssertionConsumerServiceDTO> getAssertionConsumerServices() {
        return assertionConsumerServices;
    }

    /**
     * Get the collection of attribute consumer services in use by the relying party.
     *
     * @return a collection of {@link AttributeConsumerServiceDTO}.
     */
    public Collection<AttributeConsumerServiceDTO> getAttributeConsumerServices() {
        return attributeConsumerServices;
    }

    /**
     * Get the collection of single logout services registered with the relying party.
     *
     * @return a collection of {@link ServiceEndpointDTO}.
     */
    public Collection<ServiceEndpointDTO> getSingleLogoutServices() {
        return singleLogoutServices;
    }

    /**
     * Get the certificate registered with the relying party.
     *
     * @return a {@link CertificateDTO}.
     */
    public CertificateDTO getCertificate() {
        return certificate;
    }

    /**
     * Get the default assertion consumer service.
     *
     * @return a string indicating the default assertion consumer service.
     */
    public String getDefaultAssertionConsumerService() {
        return defaultAssertionConsumerService;
    }

    /**
     * Get the default attribute consumer service.
     *
     * @return a string indicating the default attribute consumer service.
     */
    public String getDefaultAttributeConsumerService() {
        return defaultAttributeConsumerService;
    }

    /**
     * Check whether authentication requests are signed.
     *
     * @return {@code true} if authentication requests are signed, {@code false} otherwise.
     */
    public Boolean isAuthnRequestsSigned() {
        return authnRequestsSigned;
    }

    /**
     * Creates an instance of the {@link RelyingPartyDTO.Builder} class.
     *
     * @return a new {@code RelyingPartyDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for
     * usage when constructing the object from its JSON string and thus only accepts
     * content for the canonical fields of the JSON representation. Other constructors
     * may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown=true)
    @JsonPOJOBuilder
    public static class Builder {

        private String name;
        private String url;
        private Collection<SignatureAlgorithmDTO> signatureAlgorithms;
        private Collection<AssertionConsumerServiceDTO> assertionConsumerServices;
        private Collection<AttributeConsumerServiceDTO> attributeConsumerServices;
        private Collection<ServiceEndpointDTO> singleLogoutServices;
        private CertificateDTO certificate;
        private String defaultAssertionConsumerService;
        private String defaultAttributeConsumerService;
        private Boolean authnRequestsSigned;

        public Builder withName(String name) {
            this.name = name;
            return this;
        }

        public Builder withUrl(String url) {
            this.url = url;
            return this;

        }

        public Builder withSignatureAlgorithms(Collection<SignatureAlgorithmDTO> signatureAlgorithms) {
            this.signatureAlgorithms = signatureAlgorithms;
            return this;
        }

        public Builder withAssertionConsumerServices(Collection<AssertionConsumerServiceDTO> assertionConsumerServices) {
            this.assertionConsumerServices = assertionConsumerServices;
            return this;
        }

        public Builder withAttributeConsumerServices(Collection<AttributeConsumerServiceDTO> attributeConsumerServices) {
            this.attributeConsumerServices = attributeConsumerServices;
            return this;

        }

        public Builder withSingleLogoutServices(Collection<ServiceEndpointDTO> singleLogoutServices) {
            this.singleLogoutServices = singleLogoutServices;
            return this;
        }

        public Builder withCertificate(CertificateDTO certificate) {
            this.certificate = certificate;
            return this;
        }

        public Builder withDefaultAssertionConsumerService(String defaultAssertionConsumerService) {
            this.defaultAssertionConsumerService = defaultAssertionConsumerService;
            return this;
        }

        public Builder withDefaultAttributeConsumerService(String defaultAttributeConsumerService) {
            this.defaultAttributeConsumerService = defaultAttributeConsumerService;
            return this;
        }

        public Builder withAuthnRequestsSigned(Boolean authnRequestsSigned) {
            this.authnRequestsSigned = authnRequestsSigned;
            return this;
        }

        public RelyingPartyDTO build() {
            return new RelyingPartyDTO(name, url, signatureAlgorithms, assertionConsumerServices, attributeConsumerServices, singleLogoutServices, certificate, defaultAssertionConsumerService, defaultAttributeConsumerService, authnRequestsSigned);
        }
    }

}
