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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * LogonProcessor is an interface which client needs to implement to handle
 * callbacks for successful and unsuccessful authentication. We expect
 * LogonProcessor implementation to be instantiated as a bean as it needs to be
 * autowired into controllers.
 *
 * This is how you instantiate your Logon Processor implementation as a bean
 * (note that bean id should be logonProcessor for controller autowiring to
 * work).
 *
 * <bean id="logonProcessor" class="com.vmware.mynamespace.MyLogonProcessor"/>
 *
 * Deprecated: Please use LogonProcessorEx
 *
 */
@Deprecated
public interface LogonProcessor {

    /**
     * Callback function at authentication success.
     *
     * @param message  non-null
     *            Websso message
     * @param request  non-null
     * @param response  non-null
     */
    void authenticationSuccess(Message message, HttpServletRequest request, HttpServletResponse response);

    /**
     * Callback function at authentication error. An authentication error is
     * could be detected on client side or server side authentication process.
     *
     * @param message  non-null
     *            Websso message
     * @param request  non-null
     * @param response  non-null
     */
    void authenticationError(Message message, HttpServletRequest request, HttpServletResponse response);

    /**
     * Callback function to report internal error.. Contrasting to
     * authentication error, internal error is caused by client side software
     * and hardware issues.
     *
     * @param internalError  non-null
     * @param request  non-null
     * @param response  non-null
     */
    void internalError(Exception internalError, HttpServletRequest request, HttpServletResponse response);

}
