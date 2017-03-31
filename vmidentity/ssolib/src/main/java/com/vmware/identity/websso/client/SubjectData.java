/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client;

/**
 * SubjectData is an immutable structure of MessageData type which holds
 * information about authenticated subject. It is used for logout request in the
 * MessageStore.
 * 
 */
public class SubjectData implements MessageData {

    private static final long serialVersionUID = 1L;

    private String format;

    private String subject;

    // required by subclass of serializable
    protected SubjectData() {
    }

    public SubjectData(String format, String subject) {
        this.format = format;
        this.subject = subject;
    }

    public String getFormat() {
        return format;
    }

    public String getSubject() {
        return subject;
    }

}
