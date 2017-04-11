package com.vmware.identity.rest.idm.data;

import java.net.URL;
import java.security.cert.X509Certificate;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * AlternativeOCSPConnectionDTO
 ** The {@code AlternativeOCSPConnectionDTO} class represents the connection info for an alternative
 * OCSP responder.
 *
 * @author schai
 *
 */
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AlternativeOCSPConnectionDTO.Builder.class)
public class AlternativeOCSPConnectionDTO extends DTO {
    private final String ocspResponderUrl;
    private final X509Certificate signingCert;

    protected AlternativeOCSPConnectionDTO(String ocspUrl,
            X509Certificate signingCert) {
        this.ocspResponderUrl = ocspUrl;
        this.signingCert = signingCert;
    }

    /**
     * get signing certificate of OCSP responder
     *
     * @return cert
     */
    public X509Certificate getSigningCert() {
        return signingCert;
    }

    /**
     * @return url
     */
    public String getOcspResponderUrl() {
        return ocspResponderUrl;
    }

    /**
     * Create an instance of the {@link AlternativeOCSPListDTO.Builder} class.
     *
     * @return a new {@code AlternativeOCSPListDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly
     * for usage when constructing the object from its JSON string and thus only
     * accepts content for the canonical fields of the JSON representation.
     * Other constructors may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown = true)
    @JsonPOJOBuilder
    public static class Builder {
        private String ocspResponderUrl;
        private X509Certificate signingCert;

        public Builder withURL(String url) {
            this.ocspResponderUrl = url;
            return this;
        }

        public Builder withSigningCert(X509Certificate signingCert) {
            this.signingCert = signingCert;
            return this;
        }

        public AlternativeOCSPConnectionDTO build() {
            return new AlternativeOCSPConnectionDTO(ocspResponderUrl,
                    signingCert);
        }
    }
}
