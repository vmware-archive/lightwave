/*
 *  Copyright (c) 2017 VMware, Inc.  All Rights Reserved.
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
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;

/**
 * The {@code FederatedSamlConfigDTO} represents Configuration for a Federated SAML
 *
 * @author Sriram Nambakam
 */
@JsonDeserialize(builder=FederatedSamlConfigDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(JsonInclude.Include.NON_EMPTY)
public class FederatedSamlConfigDTO {

    private Collection<String> nameIDFormats;
    private final Collection<ServiceEndpointDTO> ssoServices;
    private final Collection<ServiceEndpointDTO> sloServices;
    private final CertificateChainDTO signingCertificates;
    private Map<String, String> subjectFormats;

    /**
     * Construct an {@code ExternalIDPDTO} with its various details.
     *
     * @param nameIDFormats a collection of name identifier formats.
     * @param ssoServices a collection of endpoints to use for single sign-on.
     * @param sloServices a collection of endpoints to use for single log-off.
     * @param signingCertificates a chain of signing certificates.
     * @param subjectFormats a map of subject names to formats.
     */
    public FederatedSamlConfigDTO(
            Collection<String> nameIDFormats,
            Collection<ServiceEndpointDTO> ssoServices,
            Collection<ServiceEndpointDTO> sloServices,
            CertificateChainDTO signingCertificates,
            Map<String, String> subjectFormats
            ) {
        this.nameIDFormats = nameIDFormats;
        this.ssoServices = ssoServices;
        this.sloServices = sloServices;
        this.signingCertificates = signingCertificates;
        this.subjectFormats = subjectFormats;
    }

    /**
     * Get the collection of name identifier formats.
     *
     * @return the collection of name identifier formats.
     */
    public Collection<String> getNameIDFormats() {
        return this.nameIDFormats;
    }

    /**
     * Get the collection of single sign-on service endpoints.
     *
     * @return the collection of single sign-on service endpoints.
     */
    public Collection<ServiceEndpointDTO> getSsoServices() {
        return this.ssoServices;
    }

    /**
     * Get the collection of single log-off service endpoints.
     *
     * @return the collection of single sign-on service endpoints.
     */
    public Collection<ServiceEndpointDTO> getSloServices() {
        return this.sloServices;
    }

    /**
     * Get the signing certificate chain.
     *
     * @return the signing certificate chain for the identity provider.
     */
    public CertificateChainDTO getSigningCertificates() {
        return this.signingCertificates;
    }

    /**
     * Get the map of subjects to formats.
     *
     * @return the map containing the subjects and their formats.
     */
    public Map<String, String> getSubjectFormats() {
        return this.subjectFormats;
    }

    /**
     * Creates an instance of the {@link FederatedSamlConfigDTO.Builder} class.
     *
     * @return a new {@code FederatedOidcConfigDTO.Builder}.
     */
    public static FederatedSamlConfigDTO.Builder builder() {
        return new FederatedSamlConfigDTO.Builder();
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
        private Collection<String> nameIDFormats;
        private Map<String, String> subjectFormats;
        private Collection<ServiceEndpointDTO> ssoServices;
        private Collection<ServiceEndpointDTO> sloServices;
        private CertificateChainDTO signingCertificates;

        public FederatedSamlConfigDTO.Builder withNameIDFormats(Collection<String> nameIDFormats) {
            this.nameIDFormats = nameIDFormats;
            return this;
        }

        public FederatedSamlConfigDTO.Builder withSsoServices(Collection<ServiceEndpointDTO> ssoServices) {
            this.ssoServices = ssoServices;
            return this;
        }

        public FederatedSamlConfigDTO.Builder withSloServices(Collection<ServiceEndpointDTO> sloServices) {
            this.sloServices = sloServices;
            return this;
        }

        public FederatedSamlConfigDTO.Builder withSigningCertificates(CertificateChainDTO signingCertificates) {
            this.signingCertificates = signingCertificates;
            return this;
        }

        public FederatedSamlConfigDTO.Builder withSubjectFormats(Map<String, String> subjectFormats) {
            this.subjectFormats = subjectFormats;
            return this;
        }

        public FederatedSamlConfigDTO build() {
            return new FederatedSamlConfigDTO(
                            nameIDFormats,
                            ssoServices,
                            sloServices,
                            signingCertificates,
                            subjectFormats
                        );
        }
    }
}
