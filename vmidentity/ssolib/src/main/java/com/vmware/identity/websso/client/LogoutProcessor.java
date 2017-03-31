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
 * LogoutProcessor is an interface which client needs to implement to handle
 * callbacks for successful and unsuccessful loggout, and also handle logout
 * requests.. We expect LogoutProcessor implementation to be instantiated as a
 * bean as it needs to be autowired into controllers. This is how you
 * instantiate your Logout Processor implementation as a bean (note that bean id
 * should be logonProcessor for controller autowiring to work).
 * 
 * <bean id="logoutProcessor" class="com.vmware.mynamespace.MyLogoutProcessor"/>
 */
public interface LogoutProcessor {

    /**
     * Callback function at receiving a validated successful logout response.
     * 
     * @param message
     *            Websso message
     * @param request
     * @param response
     */
    void logoutSuccess(Message message, HttpServletRequest request, HttpServletResponse response);

    /**
     * Callback function at logout error..
     * 
     * @param message
     *            Websso message
     * @param request
     * @param response
     */
    void logoutError(Message message, HttpServletRequest request, HttpServletResponse response);

    /**
     * Callback function at receiving a logout request.
     * 
     * @param message
     *            Websso message
     * @param request
     * @param response
     * @param tenant
     */
    void logoutRequest(Message message, HttpServletRequest request, HttpServletResponse response, String tenant);

    /**
     * Callback function to report internal error..
     * 
     * @param internalError
     *            client side error.
     * @param request
     *            optional. used by controller
     * @param response
     *            optional. used by controller
     */
    void internalError(Exception internalError, HttpServletRequest request, HttpServletResponse response);
}
