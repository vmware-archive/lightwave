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
 * The {@code AuthenticationPolicyDTO} class represents the per-tenant authentication policy
 * configuration details. This class contains the policy status as well as other details of
 * the authentication mechanisms (e.g. password, Windows, and/or certificate-based authentication).
 * 
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AuthenticationPolicyDTO.Builder.class)
public class AuthenticationPolicyDTO extends DTO {

    private final Boolean passwordBasedAuthenticationEnabled;
    private final Boolean windowsBasedAuthenticationEnabled;
    private final Boolean certificateBasedAuthenticationEnabled;
    private final ClientCertificatePolicyDTO clientCertificatePolicy;

    /**
     * Construct an {@code AuthenticationPolicyDTO} with different mechanisms enabled or disabled
     * and a client certificate policy.
     *
     * @param passwordBasedAuthentication enable or disable password-based authentication.
     * @param windowsBasedAuthenticationEnabled enable or disable Windows-based authentication.
     * @param certificateBasedAuthentication enable or disable certificate-based authentication.
     * @param clientCertificatePolicy the client certificate policy.
     */
    public AuthenticationPolicyDTO(Boolean passwordBasedAuthentication, Boolean windowsBasedAuthenticationEnabled, Boolean certificateBasedAuthentication, ClientCertificatePolicyDTO clientCertificatePolicy) {
        this.passwordBasedAuthenticationEnabled = passwordBasedAuthentication;
        this.windowsBasedAuthenticationEnabled = windowsBasedAuthenticationEnabled;
        this.certificateBasedAuthenticationEnabled = certificateBasedAuthentication;
        this.clientCertificatePolicy = clientCertificatePolicy;
    }

    /**
     * Check if password-based authentication is enabled.
     *
     * @return {@code true} if password-based authentication is enabled, {@code false} otherwise.
     */
    public Boolean isPasswordBasedAuthenticationEnabled() {
        return passwordBasedAuthenticationEnabled;
    }

    /**
     * Check if Windows-based authentication is enabled.
     *
     * @return {@code true} if Windows-based authentication is enabled, {@code false} otherwise.
     */
    public Boolean isWindowsBasedAuthenticationEnabled() {
        return windowsBasedAuthenticationEnabled;
    }

    /**
     * Check if certificate-based authentication is enabled.
     *
     * @return {@code true} if certificate-based authentication is enabled,
     * {@code false} otherwise.
     */
    public Boolean isCertificateBasedAuthenticationEnabled() {
        return certificateBasedAuthenticationEnabled;
    }

    /**
     * Get the client certificate policy.
     *
     * @return the client certificate policy.
     */
    public ClientCertificatePolicyDTO getClientCertificatePolicy() {
        return clientCertificatePolicy;
    }

    /**
     * Create an instance of the {@link AuthenticationPolicyDTO.Builder} class.
     *
     * @return a new {@code AuthenticationPolicyDTO.Builder}.
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
        private Boolean passwordBasedAuthenticationEnabled;
        private Boolean windowsBasedAuthenticationEnabled;
        private Boolean certificateBasedAuthenticationEnabled;
        private ClientCertificatePolicyDTO clientCertificatePolicy;


        public Builder withPasswordBasedAuthenticationEnabled(Boolean passwordAuthenticationEnabled) {
            this.passwordBasedAuthenticationEnabled = passwordAuthenticationEnabled;
            return this;
        }

        public Builder withWindowsBasedAuthenticationEnabled(Boolean windowsAuthenticationEnabled) {
            this.windowsBasedAuthenticationEnabled = windowsAuthenticationEnabled;
            return this;
        }


        public Builder withCertificateBasedAuthenticationEnabled(Boolean certificateAuthenticationEnabled) {
            this.certificateBasedAuthenticationEnabled = certificateAuthenticationEnabled;
            return this;
        }

        public Builder withClientCertificatePolicy(ClientCertificatePolicyDTO clientCertificatePolicy) {
            this.clientCertificatePolicy = clientCertificatePolicy;
            return this;
        }

        public AuthenticationPolicyDTO build() {
            return new AuthenticationPolicyDTO(passwordBasedAuthenticationEnabled, windowsBasedAuthenticationEnabled, certificateBasedAuthenticationEnabled, clientCertificatePolicy);
        }
    }

}
