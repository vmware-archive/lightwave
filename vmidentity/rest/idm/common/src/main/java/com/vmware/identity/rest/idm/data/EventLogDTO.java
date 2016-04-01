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

import java.util.Collections;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code EventLogDTO} class represents a single entry
 * of an event log.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = EventLogDTO.Builder.class)
public class EventLogDTO extends DTO {

    private final String type;
    private final String correlationId;
    private final String level;
    private final Long start;
    private final Long elapsedMillis;
    private final Map<String, Object> metadata;

    /**
     * Construct an {@code EventLogDTO} with its event type, correlation ID, outcome,
     * start time, time elapsed, and map of metadata.
     *
     * @param type the type of event that was logged.
     * @param correlationId the correlation ID of the caller.
     * @param level the logging level of the event.
     * @param start the start time of the event, measured in milliseconds since the epoch.
     * @param elapsedMillis the amount of time the event took, measured in milliseconds.
     * @param metadata a map of metadata associated with the event.
     */
    public EventLogDTO(String type, String correlationId, String level,
            Long start, Long elapsedMillis, Map<String, Object> metadata) {
        this.type = type;
        this.correlationId = correlationId;
        this.level = level;
        this.start = start;
        this.elapsedMillis = elapsedMillis;
        if (metadata != null) {
            this.metadata = Collections.unmodifiableMap(metadata);
        } else {
            this.metadata = null;
        }
    }

    /**
     * Get the type of event this object represents.
     *
     * @return the type of event this object represents.
     */
    public String getType() {
        return this.type;
    }

    /**
     * Get the correlation ID of this event.
     *
     * @return the correlation ID of the event.
     */
    public String getCorrelationId() {
        return this.correlationId;
    }

    /**
     * Get the logging level of this event.
     *
     * @return the level of the event.
     */
    public String getLevel() {
        return this.level;
    }

    /**
     * Get the start time of this event, measured in milliseconds since the epoch.
     *
     * @return the difference, measured in milliseconds, between the start time of
     *  the event and midnight, January 1, 1970 UTC.
     */
    public Long getStart() {
        return this.start;
    }

    /**
     * Get the amount of time the event took, measured in milliseconds.
     *
     * @return the amount of time the event took, measured in milliseconds.
     */
    public Long getElapsedMillis() {
        return this.elapsedMillis;
    }

    /**
     * Get the map containing the metadata for this event.
     *
     * @return a map of strings to objects, representing the metadata
     *  for this event.
     */
    public Map<String, Object> getMetadata() {
        return this.metadata;
    }

    /**
     * Create an instance of the {@link EventLogDTO.Builder} class.
     *
     * @return a new {@code EventLogDTO.Builder}.
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

        private String type;
        private String correlationId;
        private String level;
        private Long start;
        private Long elapsedMillis;
        private Map<String, Object> metadata;

        public Builder withType(String type) {
            this.type = type;
            return this;
        }

        public Builder withCorrelationId(String correlationId) {
            this.correlationId = correlationId;
            return this;
        }

        public Builder withLevel(String level) {
            this.level = level;
            return this;
        }

        public Builder withStart(Long start) {
            this.start = start;
            return this;
        }

        public Builder withElapsedMillis(Long elapsedMillis) {
            this.elapsedMillis = elapsedMillis;
            return this;
        }

        public Builder withMetadata(Map<String, Object> metadata) {
            this.metadata = metadata;
            return this;
        }

        public EventLogDTO build() {
            return new EventLogDTO(type, correlationId, level, start, elapsedMillis, metadata);
        }
    }

}
