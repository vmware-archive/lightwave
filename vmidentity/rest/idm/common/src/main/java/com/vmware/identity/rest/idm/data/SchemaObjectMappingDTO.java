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

import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;
import com.vmware.identity.rest.idm.data.attributes.DomainAttributeId;
import com.vmware.identity.rest.idm.data.attributes.GroupAttributeId;
import com.vmware.identity.rest.idm.data.attributes.ObjectClass;
import com.vmware.identity.rest.idm.data.attributes.UserAttributeId;

/**
 * The {@code SchemaObjectMappingDTO} class describes a mapping of attributes for
 * a given object class, for use in describing the schema of an identity object.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonDeserialize(builder=SchemaObjectMappingDTO.Builder.class)
@JsonInclude(Include.NON_EMPTY)
public class SchemaObjectMappingDTO extends DTO {

    private final String objectClass;
    private final Map<String, String> attributeMappings;

    /**
     * Construct a {@code SchemaObjectMappingDTO} with an object class and a mapping of
     * attributes to attribute names.
     * <p>
     * Attribute mappings occur from attribute identifier to the attribute class. See
     * {@link DomainAttributeId}, {@link GroupAttributeId}, {@link UserAttributeId} for known
     * identifiers.
     *
     * @param objectClass the class that the attribute mapping is for. See {@link ObjectClass}.
     * @param schemaAttributeMappings a mapping from attribute identifier to attribute names.
     */
    public SchemaObjectMappingDTO(String objectClass, Map<String, String> schemaAttributeMappings) {
        this.objectClass = objectClass;
        this.attributeMappings = schemaAttributeMappings;
    }

    /**
     * Get the object class represented by this mapping. See {@link ObjectClass} for
     * an enum containing the known object classes. Note, however, that the class returned
     * by the server may not be contained within this enum.
     *
     * @return the object class represented by the mapping.
     */
    public String getObjectClass() {
        return objectClass;
    }

    /**
     * Get the map of attribute identifiers to attribute names.
     *
     * @return the map of attribute identifiers to attribute names.
     */
    public Map<String, String> getAttributeMappings() {
        return attributeMappings;
    }

    /**
     * Creates an instance of the {@link SchemaObjectMappingDTO.Builder} class.
     *
     * @return a new {@code SchemaObjectMappingDTO.Builder}.
     */
    public static Builder builder() {
        return new SchemaObjectMappingDTO.Builder();
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
        private String objectClass;
        private Map<String, String> attributeMappings;

        public Builder withObjectClass(String objectClass) {
            this.objectClass = objectClass;
            return this;
        }

        public Builder withAttributeMappings(Map<String, String> schemaAttributeMappings) {
            this.attributeMappings = schemaAttributeMappings;
            return this;
        }

        public SchemaObjectMappingDTO build() {
            return new SchemaObjectMappingDTO(objectClass, attributeMappings);
        }
    }

}
