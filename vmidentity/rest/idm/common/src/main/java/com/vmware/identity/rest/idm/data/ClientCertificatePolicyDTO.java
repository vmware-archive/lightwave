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

import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code ClientCertificatePolicyDTO} class contains the configuration details related to certificate management, as well as details related to Online Certificate Status Protocol (OCSP) and the
 * Certificate Revocation List (CRL).
 *
 * @see <a href=https://tools.ietf.org/html/rfc6960>X.509 Internet Public Key Infrastructure Online Certificate Status Protocol - OCSP</a>
 * @see <a href=https://tools.ietf.org/html/rfc3280>Internet X.509 Public Key Infrastructure Certificate and Certificate Revocation List (CRL) Profile</a>
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = ClientCertificatePolicyDTO.Builder.class)
public class ClientCertificatePolicyDTO extends DTO {

    private final List<String> certPolicyOIDs;
    private final List<CertificateDTO> trustedCACertificates;
    private final Boolean revocationCheckEnabled;
    private final Boolean userNameHintEnabled;

    // OCSP (Online Certificate Status Protocol) related configurable entities
    private final Boolean ocspEnabled;
    private final Boolean failOverToCrlEnabled;
    private final String ocspUrlOverride;
    private final AlternativeOCSPConnectionsDTO altOCSPConnections;
    // CRL (Certificate Revocation List) related configurable entities
    private final Boolean crlDistributionPointUsageEnabled;
    private final String crlDistributionPointOverride;

    /**
     * Construct a {@code ClientCertificatePolicyDTO} with its details.
     *
     * @param certPolicyOIDs
     *            certificate policy OID whitelist. Only this list of policies will accepted. Null/empty list means all policies will be accepted.
     * @param trustedCACertificates
     *            trusted certificate-authority certificates.
     * @param ocspEnabled
     *            enable or disable OCSP.
     * @param failOverToCrlEnabled
     *            enable or disable fail-over to CRL.
     * @param ocspUrlOverride
     *            override the OCSP URL.
     * @param altOCSPConnections
     *            A map of alternative OCSP connections for sites.
     * @param revocationCheckEnabled
     *            enable or disable revocation checks.
     * @param userNameHintEnabled
     *            for PIV/SmartCard authentication.
     * @param crlDistributionPointUsageEnabled
     *            enable or disable CRL distribute point usage.
     * @param crlDistributionPointOverride
     *            override the CRL distribution point.
     */
    protected ClientCertificatePolicyDTO(List<String> certPolicyOIDs,
            List<CertificateDTO> trustedCACertificates, Boolean ocspEnabled,
            Boolean failOverToCrlEnabled, String ocspUrlOverride,
            AlternativeOCSPConnectionsDTO altOCSPConnections,
            Boolean revocationCheckEnabled,
            Boolean crlDistributionPointUsageEnabled,
            String crlDistributionPointOverride, Boolean userNameHintEnabled) {
        this.certPolicyOIDs = certPolicyOIDs;
        this.trustedCACertificates = trustedCACertificates;
        this.ocspEnabled = ocspEnabled;
        this.failOverToCrlEnabled = failOverToCrlEnabled;
        this.ocspUrlOverride = ocspUrlOverride;
        this.altOCSPConnections = altOCSPConnections;
        this.revocationCheckEnabled = revocationCheckEnabled;
        this.crlDistributionPointUsageEnabled = crlDistributionPointUsageEnabled;
        this.crlDistributionPointOverride = crlDistributionPointOverride;
        this.userNameHintEnabled = userNameHintEnabled;
    }

    /**
     * Get the list of certificate policy OIDs.
     *
     * @return list of certificate policies.
     */
    public List<String> getCertPolicyOIDs() {
        return certPolicyOIDs;
    }

    /**
     * Get the list of certificate-authority certificates.
     *
     * @return list of certificate-authority certificates.
     */
    public List<CertificateDTO> getTrustedCACertificates() {
        return trustedCACertificates;
    }

    /**
     * Check if OCSP is enabled on the client.
     *
     * @return {@code true} if OCSP is enabled, {@code false} otherwise.
     */
    public Boolean isOcspEnabled() {
        return ocspEnabled;
    }

    /**
     * Check if the CRL mechanism should be used when OCSP fails to communicate with its provider.
     *
     * @return {@code true} if OCSP failover is enabled, {@code false} otherwise.
     */
    public Boolean isFailOverToCrlEnabled() {
        return failOverToCrlEnabled;
    }

    /**
     * Get the override URL to use for OCSP.
     *
     * @return the override URL.
     */
    public String getOcspUrlOverride() {
        return ocspUrlOverride;
    }

    public AlternativeOCSPConnectionsDTO getAltOCSPConnections() {
        return altOCSPConnections;
    }

    /**
     * Check if certificate revocation checks must be performed.
     *
     * @return {@code true} if certificate revocation is enabled, {@code false} otherwise.
     */
    public Boolean isRevocationCheckEnabled() {
        return revocationCheckEnabled;
    }

    /**
     * Check if username hint is enabled in smartcard login UI.
     *
     * @return {@code true} if certificate revocation is enabled, {@code false} otherwise.
     */
    public Boolean isUserNameHintEnabled() {
        return userNameHintEnabled;
    }

    /**
     * Check if CRL distribution point usage is enabled.
     *
     * @return {@code true} if CRL distribution point usage is enabled, {@code false} otherwise.
     */
    public Boolean isCrlDistributionPointUsageEnabled() {
        return crlDistributionPointUsageEnabled;
    }

    /**
     * Get the override URL for the CRL distribution point.
     *
     * @return the override URL for the CRL distribution point.
     */
    public String getCrlDistributionPointOverride() {
        return crlDistributionPointOverride;
    }

    /**
     * Creates an instance of the {@link ClientCertificatePolicyDTO.Builder} class.
     *
     * @return a new {@code ClientCertificatePolicyDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for usage when constructing the object from its JSON string and thus only accepts content for the canonical fields of the
     * JSON representation. Other constructors may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown = true)
    @JsonPOJOBuilder
    public static class Builder {

        private List<String> objectIdentifiers = null;
        private List<CertificateDTO> trustedCertificates = null;

        private Boolean ocspEnabled = false;
        private Boolean failOverToCrlEnabled = false;
        private String ocspUrlOverride = null;
        private AlternativeOCSPConnectionsDTO altOCSPConnections;

        private Boolean revocationCheckEnabled = true;
        private Boolean userNameHintEnabled = false;

        private Boolean crlDistributionPointUsageEnabled = true;
        private String crlDistributionPointOverride = null;

        public Builder withCertPolicyOIDs(List<String> objectIdentifiers) {
            this.objectIdentifiers = objectIdentifiers;
            return this;
        }

        public Builder withTrustedCACertificates(List<CertificateDTO> trustedCertificates) {
            this.trustedCertificates = trustedCertificates;
            return this;
        }

        public Builder withOcspEnabled(Boolean ocspEnabled) {
            this.ocspEnabled = ocspEnabled;
            return this;
        }

        public Builder withFailOverToCrlEnabled(Boolean failOverToCrlEnabled) {
            this.failOverToCrlEnabled = failOverToCrlEnabled;
            return this;
        }

        public Builder withOcspUrlOverride(String ocspUrlOverride) {
            this.ocspUrlOverride = ocspUrlOverride;
            return this;
        }

        public Builder withAltOCSPConnections(AlternativeOCSPConnectionsDTO altOCSPConnections) {
            this.altOCSPConnections = altOCSPConnections;
            return this;
        }

        public Builder withRevocationCheckEnabled(Boolean revocationCheckEnabled) {
            this.revocationCheckEnabled = revocationCheckEnabled;
            return this;
        }

        public Builder withUserNameHintEnabled(Boolean userNameHintEnabled) {
            this.userNameHintEnabled = userNameHintEnabled;
            return this;
        }

        public Builder withCrlDistributionPointUsageEnabled(Boolean crlDistributionPointUsageEnabled) {
            this.crlDistributionPointUsageEnabled = crlDistributionPointUsageEnabled;
            return this;
        }

        public Builder withCrlDistributionPointOverride(String crlDistributionPointOverride) {
            this.crlDistributionPointOverride = crlDistributionPointOverride;
            return this;
        }

        public ClientCertificatePolicyDTO build() {
            return new ClientCertificatePolicyDTO(objectIdentifiers,
                    trustedCertificates, ocspEnabled, failOverToCrlEnabled,
                    ocspUrlOverride, altOCSPConnections,
                    revocationCheckEnabled, crlDistributionPointUsageEnabled,
                    crlDistributionPointOverride, userNameHintEnabled);
        }
    }

}
