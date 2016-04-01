/**************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. 
 **************************************************************************/
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
