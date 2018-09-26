package com.vmware.identity.rest.idm.data;

import java.util.List;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code AttributeDefinitionsDTO} class contains the {@link AttributeDTO}s.
 *
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = AttributeDefinitionsDTO.Builder.class)
public class AttributeDefinitionsDTO extends DTO {

    private final List<AttributeDTO> attributes;

    /**
     * Construct a {@code AttributeDefinitionsDTO} from a list of attributes.
     *
     * @param certificates the list of attributes.
     */
    public AttributeDefinitionsDTO(List<AttributeDTO> attributes) {
        this.attributes = attributes;
    }

    /**
     * Get the list of attributes.
     *
     * @return a list of attributes.
     */
    public List<AttributeDTO> getAttributes() {
        return this.attributes;
    }

    /**
     * Create an instance of the {@link AttributeDefinitionsDTO.Builder} class.
     *
     * @return a new {@code AttributeDefinitionsDTO.Builder}.
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

        private List<AttributeDTO> attributes;

        public Builder withAttributes(List<AttributeDTO> attributes) {
            this.attributes = attributes;
            return this;
        }

        public AttributeDefinitionsDTO build() {
            return new AttributeDefinitionsDTO(attributes);
        }
    }
}
