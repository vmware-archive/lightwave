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

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code OIDCClientDTO} class contains the details of an
 * OpenID Connect client.
 *
 * @see <a href=https://tools.ietf.org/html/rfc6749#section-2>
 *  The OAuth 2.0 Authorization Framework - Client Registration
 *  </a>
 * @see <a href=http://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata>
 *  OpenID Connect Dynamic Client Registration 1.0 incorporating errata set 1 - Client Metadata
 *  </a>
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = OIDCClientDTO.Builder.class)
public class OIDCClientDTO extends DTO {

    private final String clientId;
    private final OIDCClientMetadataDTO oidcClientMetadataDTO;

    /**
     * Constructs an {@code OIDCClientDTO} with a client identifier and metadata.
     *
     * @param clientId identifier for the OIDC client.
     * @param oidcClientMetadataDTO metadata for the OIDC client.
     */
    public OIDCClientDTO(String clientId, OIDCClientMetadataDTO oidcClientMetadataDTO) {
        this.clientId = clientId;
        this.oidcClientMetadataDTO = oidcClientMetadataDTO;
    }

    /**
     * Get the client identifier for the OIDC client.
     *
     * @return the client identifier for the OIDC client.
     */
    public String getClientId() {
        return this.clientId;
    }

    /**
     * Get the metadata for the OIDC client.
     *
     * @return the metadata for the OIDC client.
     */
    public OIDCClientMetadataDTO getOIDCClientMetadataDTO() {
        return this.oidcClientMetadataDTO;
    }

    /**
     * Creates an instance of the {@link OIDCClientDTO.Builder} class.
     *
     * @return a new {@code OIDCClientDTO.Builder}.
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

        private String clientId;
        private OIDCClientMetadataDTO oidcClientMetadataDTO;

        public Builder withClientId(String clientId) {
            this.clientId = clientId;
            return this;
        }

        public Builder withOIDCClientMetadataDTO(OIDCClientMetadataDTO oidcClientMetadataDTO) {
            this.oidcClientMetadataDTO = oidcClientMetadataDTO;
            return this;
        }

        public OIDCClientDTO build() {
            return new OIDCClientDTO(clientId, oidcClientMetadataDTO);
        }
    }

}
