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
