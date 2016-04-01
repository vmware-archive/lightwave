/*
 *
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
 *
 */

package com.vmware.identity.idm;

import java.io.Serializable;
import java.util.Collections;
import java.util.Set;

/**
 * @author Yehia Zayour
 */
public class ResourceServer implements Serializable {
    private static final long serialVersionUID = 1L;
    private static final String SINGLE_BACK_SLASH_REGEX = "\\\\"; // regex representing a single \
    private static final String NAME_PREFIX = "rs_"; // this is how oidc knows it's a resource server when it appears in the scope parameter

    private final String name;
    private final Set<String> groupFilter;

    private ResourceServer(Builder builder) {
        this.name = builder.name;
        this.groupFilter = (builder.groupFilter != null) ? builder.groupFilter : Collections.<String>emptySet();
    }

    public String getName() {
        return this.name;
    }

    public Set<String> getGroupFilter() {
        return this.groupFilter;
    }

    public static class Builder {
        private String name;
        private Set<String> groupFilter;

        public Builder(String name) {
            boolean valid =
                    name != null &&
                    name.startsWith(NAME_PREFIX) &&
                    name.length() > NAME_PREFIX.length();
            if (!valid) {
                throw new IllegalArgumentException("name must start with " + NAME_PREFIX);
            }

            this.name = name;
        }

        public Builder groupFilter(Set<String> groupFilter) {
            if (groupFilter != null) {
                for (String entry : groupFilter) {
                    String[] parts = entry.split(SINGLE_BACK_SLASH_REGEX);
                    boolean valid =
                            parts.length == 2 &&
                            parts[0].length() > 0 &&
                            parts[1].length() > 0;
                    if (!valid) {
                        throw new IllegalArgumentException("groupFilter entry must be of the form domain\\name");
                    }
                }
            }

            this.groupFilter = groupFilter;
            return this;
        }

        public ResourceServer build() {
            return new ResourceServer(this);
        }
    }
}
