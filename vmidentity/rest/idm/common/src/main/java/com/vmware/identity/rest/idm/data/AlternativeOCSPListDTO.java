package com.vmware.identity.rest.idm.data;

import java.net.URL;
import java.util.List;

import javax.security.cert.X509Certificate;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;
import com.vmware.identity.rest.idm.data.AlternativeOCSPConnectionDTO.Builder;

/**
 * A DTO class of AlternativeOCSPList.
 *
 * @author schai
 *
 */
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AlternativeOCSPListDTO.Builder.class)
public class AlternativeOCSPListDTO extends DTO {
    private final String siteId;
    private final List<AlternativeOCSPConnectionDTO> altOCSPConnectionList;

    public AlternativeOCSPListDTO(String siteId,
            List<AlternativeOCSPConnectionDTO> altOCSPConnectionList) {
        this.siteId = siteId;
        this.altOCSPConnectionList = altOCSPConnectionList;
    }

    /**
     * @return site id of the ocsp connection list.
     */
    public String getSiteId() {
        return siteId;
    }

    /**
     * @return list of ocsp connections for the site.
     */
    public List<AlternativeOCSPConnectionDTO> getAltOCSPConnectionList() {
        return altOCSPConnectionList;
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
        private String siteId;
        private List<AlternativeOCSPConnectionDTO> altOCSPConnectionList;

        public Builder withSiteId(String siteId) {
            this.siteId = siteId;
            return this;
        }

        public Builder withOCSPConnectionList(
                List<AlternativeOCSPConnectionDTO> altOCSPConnectionList) {
            this.altOCSPConnectionList = altOCSPConnectionList;
            return this;
        }

        public AlternativeOCSPListDTO build() {
            return new AlternativeOCSPListDTO(siteId, altOCSPConnectionList);
        }
    }
}
