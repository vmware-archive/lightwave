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
package com.vmware.identity.rest.core.data;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.vmware.identity.rest.core.util.ObjectMapperSingleton;

/**
 * {@code DTO} is the abstract base class for all of the data objects in the REST SSO context.
 * A {@code DTO} encapsulates all of the information necessary for a purpose as well as instructions
 * for converting the object to and from JSON. The {@link #toString()} method will construct
 * the canonical JSON (i.e. what will be sent over the wire), while {@link #toPrettyString()}
 * will construct a more human readable form of the object.
 * <p>
 * In certain cases, the inheriting {@code DTO} may declare additional fields or methods that are
 * not intended to be present in the serialized form. In these cases, the fields are provided for
 * the convenience of those utilizing the classes. These fields should be marked with
 * {@code @JsonIgnore} and should not be present in any {@code Builder} class for the {@code DTO}.
 */
public abstract class DTO {

    @Override
    public String toString() {
        try {
            return ObjectMapperSingleton.getInstance().writer().writeValueAsString(this);
        } catch (JsonProcessingException e) {
            // Shouldn't happen, but if it does, we can just use the default printing...
            return super.toString();
        }
    }

    /**
     * Returns a pretty-printed string representation of the object. In general, the
     * {@code #toPrettyString()} method returns a JSON string that has not been minified
     * and thus is more human-readable for the purposes of logging.
     *
     * @return a pretty-printed string representation of the object.
     */
    public String toPrettyString() {
        try {
            return ObjectMapperSingleton.getInstance().writerWithDefaultPrettyPrinter().writeValueAsString(this);
        } catch (JsonProcessingException e) {
            // Shouldn't happen, but if it does, we can just use the default printing...
            return super.toString();
        }
    }

}
