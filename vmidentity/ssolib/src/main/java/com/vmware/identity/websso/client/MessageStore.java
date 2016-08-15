/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 * ************************************************************************/
package com.vmware.identity.websso.client;

/**
 * MessageStore is an interface which archives SSO and SLO messages. This can be
 * useful if client wants to refer back to previously sent messages or do some
 * advanced processing. We expect MessageStore implementation to be instantiated
 * as a bean as it needs to be autowired into controllers.
 * 
 */
public interface MessageStore {

    /**
     * add a Message to the store. Message must have an ID, or it will ignore
     * the action.
     * 
     */
    void add(Message message);

    void remove(String messageId) throws WebssoClientException;

    Message get(String messageId);
}
