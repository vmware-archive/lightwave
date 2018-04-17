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

import java.util.List;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code FederatedIdpPDTO} class contains the details of a Federated Identity Provider.
 *
 * @author Sriram Nambakam
 */
@JsonDeserialize(builder=FederatedIdpDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class FederatedIdpDTO extends DTO {

    private final String entityID;
    private final String protocol;
    private final String alias;
    private final FederatedOidcConfigDTO oidcConfig;
    private final FederatedSamlConfigDTO samlConfig;
    private final Boolean jitEnabled;
    private final String upnSuffix;
    private final List<TokenClaimGroupDTO> roleGroupMappings;
    private final Boolean multiTenant;

    /**
     * Construct an {@code FederatedIdpPDTO} with its various details.
     *
     * @param entityID the entity identifier.
     * @param protocol the protocol used by this External IDP
     * @param alias the alias of the IDP.
     * @param jitEnabled enable or disable just-in-time provisioning.
     * @param upnSuffix a UPN suffix to use for this identity provider.
     * @param roleGroupMap role group mapping for this identity provider.
     * */
    public FederatedIdpDTO(
            String entityID,
            String protocol,
            String alias,
            FederatedOidcConfigDTO oidcConfig,
            FederatedSamlConfigDTO samlConfig,
            Boolean jitEnabled,
            String upnSuffix,
            List<TokenClaimGroupDTO> roleGroupMappings
    ) {
        this(entityID, protocol, alias, oidcConfig, samlConfig, jitEnabled, 
                upnSuffix, roleGroupMappings, null);
    }

    /**
     * Construct an {@code FederatedIdpPDTO} with its various details.
     *
     * @param entityID the entity identifier.
     * @param protocol the protocol used by this External IDP
     * @param alias the alias of the IDP.
     * @param jitEnabled enable or disable just-in-time provisioning.
     * @param upnSuffix a UPN suffix to use for this identity provider.
     * @param roleGroupMap role group mapping for this identity provider.
     * @param multiTenant whether the idp config is multi-tenanted
     */
    public FederatedIdpDTO(
            String entityID,
            String protocol,
            String alias,
            FederatedOidcConfigDTO oidcConfig,
            FederatedSamlConfigDTO samlConfig,
            Boolean jitEnabled,
            String upnSuffix,
            List<TokenClaimGroupDTO> roleGroupMappings,
            Boolean multiTenant
    ) {
        this.entityID = entityID;
        this.protocol = protocol;
        this.alias = alias;
        this.oidcConfig = oidcConfig;
        this.samlConfig = samlConfig;
        this.jitEnabled = jitEnabled;
        this.upnSuffix = upnSuffix;
        this.roleGroupMappings = roleGroupMappings;
        this.multiTenant = multiTenant;
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
     * Get the protocol.
     *
     * @return the protocol.
     */
    public String getProtocol() {
        return this.protocol;
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
     * Check if the IDP is multi-tenant.
     *
     * @return
     */
    public Boolean isMultiTenant() {
        return this.multiTenant;
    }

    /**
     * Get the OIDC Configuration.
     *
     * This is available only if the protocol is set to OAUTH
     *
     * @return the OIDC Configuration for this identity provider
     */
    public FederatedOidcConfigDTO getOidcConfig() { return oidcConfig; }

    /**
     * Get the SAML Configuration.
     *
     * This is available only if the protocol is set to SAML
     *
     * @return the OIDC Configuration for this identity provider
     */
    public FederatedSamlConfigDTO getSamlConfig() { return samlConfig; }

    /**
     * Get the role group mappings for the idp.
     *
     * @return list of token role group mappings
     */
    public List<TokenClaimGroupDTO> getRoleGroupMappings (){ return roleGroupMappings; }

    /**
     * Creates an instance of the {@link FederatedIdpDTO.Builder} class.
     *
     * @return a new {@code FederatedIdpPDTO.Builder}.
     */
    public static Builder builder() {
        return new FederatedIdpDTO.Builder();
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
        private String protocol;
        private String alias;
        private FederatedOidcConfigDTO oidcConfig;
        private FederatedSamlConfigDTO samlConfig;
        private Boolean jitEnabled;
        private String upnSuffix;
        private List<TokenClaimGroupDTO> roleGroupMappings;
        private Boolean multiTenant;

        public Builder withEntityID(String entityID) {
            this.entityID = entityID;
            return this;
        }

        public Builder withProtocol(String protocol) {
            this.protocol = protocol;
            return this;
        }

        public Builder withAlias(String alias) {
            this.alias = alias;
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

        public Builder withOidcConfig(FederatedOidcConfigDTO oidcConfig) {
            this.oidcConfig = oidcConfig;
            return this;
        }

        public Builder withSamlConfig(FederatedSamlConfigDTO samlConfig) {
            this.samlConfig = samlConfig;
            return this;
        }

        public Builder withRoleGroupMappings(List<TokenClaimGroupDTO> roleGroupMappings) {
            this.roleGroupMappings = roleGroupMappings;
            return this;
        }

        public Builder withMultiTenant(Boolean multiTenant) {
            this.multiTenant = multiTenant;
            return this;
        }

        public FederatedIdpDTO build() {
            return new FederatedIdpDTO(
                            entityID,
                            protocol,
                            alias,
                            oidcConfig,
                            samlConfig,
                            jitEnabled,
                            upnSuffix,
                            roleGroupMappings,
                            multiTenant
            );
        }
    }
}
