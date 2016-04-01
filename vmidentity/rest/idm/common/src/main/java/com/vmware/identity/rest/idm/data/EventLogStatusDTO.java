package com.vmware.identity.rest.idm.data;
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
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code EventLogStatusDTO} class represents the status of the
 * event log.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = EventLogStatusDTO.Builder.class)
public class EventLogStatusDTO extends DTO {

    private final Boolean enabled;
    private final Long size;

    /**
     * Construct an {@code EventLogStatusDTO} with a boolean indicating whether
     * the event log is enabled and the size of the event log.
     *
     * @param enabled a boolean indicating whether the event log is enabled.
     * @param size the maximum size of the event log.
     */
    public EventLogStatusDTO(Boolean enabled, Long size) {
        this.enabled = enabled;
        this.size = size;
    }

    /**
     * Check if the Event Log is enabled.
     *
     * @return {@code true} if the event log is enabled, {@code false} otherwise.
     */
    public Boolean isEnabled() {
        return this.enabled;
    }

    /**
     * Get the size of the event log.
     *
     * @return the size of the event log.
     */
    public Long getSize() {
        return this.size;
    }

    /**
     * Create an instance of the {@link EventLogStatusDTO.Builder} class.
     *
     * @return a new {@code EventLogStatusDTO.Builder}.
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
        private Boolean enabled;
        private Long size;

        public Builder withEnabled(Boolean enabled) {
            this.enabled = enabled;
            return this;
        }

        public Builder withSize(Long size) {
            this.size = size;
            return this;
        }

        public EventLogStatusDTO build() {
            return new EventLogStatusDTO(enabled, size);
        }

    }

}
