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
import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code ExternalIDPDTO} class contains the details of an External Identity Provider.
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonDeserialize(builder=ExternalIDPDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class ExternalIDPDTO extends DTO {

    private final String entityID;
    private final String alias;
    private final Collection<String> nameIDFormats;
    private final Collection<ServiceEndpointDTO> ssoServices;
    private final Collection<ServiceEndpointDTO> sloServices;
    private final CertificateChainDTO signingCertificates;
    private final Map<String, String> subjectFormats;
    private final List<TokenClaimGroupDTO> tokenClaimGroups;
    private final Boolean jitEnabled;
    private final String upnSuffix;

    /**
     * Construct an {@code ExternalIDPDTO} with its various details.
     *
     * @param entityID the entity identifier.
     * @param alias the alias of the IDP.
     * @param nameIDFormats a collection of name identifier formats.
     * @param ssoServices a collection of endpoints to use for single sign-on.
     * @param sloServices a collection of endpoints to use for single log-off.
     * @param signingCertificates a chain of signing certificates.
     * @param subjectFormats a map of subject names to formats.
     * @param tokenClaimGroups a list of token claim groups.
     * @param jitEnabled enable or disable just-in-time provisioning.
     * @param upnSuffix a UPN suffix to use for this identity provider.
     */
    public ExternalIDPDTO(String entityID, String alias, Collection<String> nameIDFormats,
            Collection<ServiceEndpointDTO> ssoServices, Collection<ServiceEndpointDTO> sloServices,
            CertificateChainDTO signingCertificates, Map<String, String> subjectFormats,
            List<TokenClaimGroupDTO> tokenClaimGroups, Boolean jitEnabled, String upnSuffix) {
        this.entityID = entityID;
        this.alias = alias;
        this.nameIDFormats = nameIDFormats;
        this.ssoServices = ssoServices;
        this.sloServices = sloServices;
        this.signingCertificates = signingCertificates;
        this.subjectFormats = subjectFormats;
        this.tokenClaimGroups = tokenClaimGroups;
        this.jitEnabled = jitEnabled;
        this.upnSuffix = upnSuffix;
    }

    /**
     * Get the entity identifier.
     *
     * @return the entity identifier.
     */
    public String getEntityID() {
        return this.entityID;
    }

    /**
     * Get the alias.
     *
     * @return the alias.
     */
    public String getAlias() {
        return this.alias;
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
     * Get the list of token claim groups.
     *
     * @return the list of token claim groups.
     */
    public List<TokenClaimGroupDTO> getTokenClaimGroups() {
        return this.tokenClaimGroups;
    }

    /**
     * Check if just-in-time provisioning is enabled.
     *
     * @return {@code true} if JIT is enabled, {@code false} otherwise.
     */
    public Boolean isJitEnabled() {
        return this.jitEnabled;
    }

    /**
     * Get the UPN suffix.
     *
     * @return the UPN suffix for this identity provider.
     */
    public String getUpnSuffix() {
        return this.upnSuffix;
    }

    /**
     * Creates an instance of the {@link ExternalIDPDTO.Builder} class.
     *
     * @return a new {@code ExternalIDPDTO.Builder}.
     */
    public static Builder builder() {
        return new ExternalIDPDTO.Builder();
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
        private String entityID;
        private String alias;
        private Collection<String> nameIDFormats;
        private Collection<ServiceEndpointDTO> ssoServices;
        private Collection<ServiceEndpointDTO> sloServices;
        private CertificateChainDTO signingCertificates;
        private Map<String, String> subjectFormats;
        private List<TokenClaimGroupDTO> tokenClaimGroups;
        private Boolean jitEnabled;
        private String upnSuffix;

        public Builder withEntityID(String entityID) {
            this.entityID = entityID;
            return this;
        }

        public Builder withAlias(String alias) {
            this.alias = alias;
            return this;
        }

        public Builder withNameIDFormats(Collection<String> nameIDFormats) {
            this.nameIDFormats = nameIDFormats;
            return this;
        }

        public Builder withSsoServices(Collection<ServiceEndpointDTO> ssoServices) {
            this.ssoServices = ssoServices;
            return this;
        }

        public Builder withSloServices(Collection<ServiceEndpointDTO> sloServices) {
            this.sloServices = sloServices;
            return this;
        }

        public Builder withSigningCertificates(CertificateChainDTO signingCertificates) {
            this.signingCertificates = signingCertificates;
            return this;
        }

        public Builder withSubjectFormats(Map<String, String> subjectFormats) {
            this.subjectFormats = subjectFormats;
            return this;
        }

        public Builder withTokenClaimGroups(List<TokenClaimGroupDTO> tokenClaimGroups) {
            this.tokenClaimGroups = tokenClaimGroups;
            return this;
        }

        public Builder withJitEnabled(Boolean jitEnabled) {
            this.jitEnabled = jitEnabled;
            return this;
        }

        public Builder withUpnSuffix(String upnSuffix) {
            this.upnSuffix = upnSuffix;
            return this;
        }

        public ExternalIDPDTO build() {
            return new ExternalIDPDTO(entityID, alias, nameIDFormats, ssoServices, sloServices, signingCertificates, subjectFormats, tokenClaimGroups, jitEnabled, upnSuffix);
        }
    }

}
