package com.vmware.identity.rest.idm.data;

import java.util.List;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;

@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = PrincipalIdentifiersDTO.Builder.class)
public class PrincipalIdentifiersDTO {

    private final List<String> ids;

    /**
     * Construct a {@code PrincipalIdentifiersDTO} with a list of principal Ids.
     *
     * @param ids a list of principal ids.
     *
     */
    public PrincipalIdentifiersDTO(List<String> ids) {
        this.ids = ids;
    }

    public List<String> getIds() {
        return this.ids;
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
        private List<String> ids;

        public Builder withIds(List<String> ids) {
            this.ids = ids;
            return this;
        }

        public PrincipalIdentifiersDTO build() {
            return new PrincipalIdentifiersDTO( this.ids);
        }
    }
}
