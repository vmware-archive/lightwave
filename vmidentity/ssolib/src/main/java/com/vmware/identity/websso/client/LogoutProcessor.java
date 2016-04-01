/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. 
 * ************************************************************************/
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
