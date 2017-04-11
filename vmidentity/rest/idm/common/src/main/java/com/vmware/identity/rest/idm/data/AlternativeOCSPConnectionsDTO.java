package com.vmware.identity.rest.idm.data;

import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * A DTO class of Map<String, AlternativeOCSPList>
 *
 * @author schai
 *
 */
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AlternativeOCSPConnectionsDTO.Builder.class)
public class AlternativeOCSPConnectionsDTO extends DTO {
    private final Map<String, AlternativeOCSPListDTO> alternativeOCSPConnections;

    public AlternativeOCSPConnectionsDTO(Map<String, AlternativeOCSPListDTO> alternativeOCSPConnections) {
        this.alternativeOCSPConnections = alternativeOCSPConnections;
    }

    public Map<String, AlternativeOCSPListDTO> getAlternativeOCSPConnections() {
        return alternativeOCSPConnections;
    }

    /**
     * Create an instance of the {@link AlternativeOCSPConnectionsDTO.Builder}
     * class.
     *
     * @return a new {@code AlternativeOCSPConnectionsDTO.Builder}.
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

        private Map<String, AlternativeOCSPListDTO> alternativeOCSPConnections;

        public Builder withOCSPConnections(Map<String, AlternativeOCSPListDTO> alternativeOCSPConnections) {
            this.alternativeOCSPConnections = alternativeOCSPConnections;
            return this;
        }

        public AlternativeOCSPConnectionsDTO build() {
            return new AlternativeOCSPConnectionsDTO(alternativeOCSPConnections);
        }
    }
}
