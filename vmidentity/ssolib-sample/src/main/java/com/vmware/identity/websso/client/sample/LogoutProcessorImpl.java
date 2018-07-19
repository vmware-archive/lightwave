/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client.sample;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.vmware.identity.websso.client.LogoutProcessor;
import com.vmware.identity.websso.client.Message;

/**
 * Sample Logout Processor Implementation
 *
 */
public class LogoutProcessorImpl implements LogoutProcessor {

    /* (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogoutProcessor#logoutError(com.vmware.identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void logoutError(Message arg0, HttpServletRequest request,
          HttpServletResponse response) {
        String redirectUrl;
        try {
            redirectUrl = String.format("https://%s:%s/ssolib-sample/error",
                    ComponentUtils.getHostName(), request.getServerPort());
            response.sendRedirect(redirectUrl);
        } catch (Exception e) {
            throw new IllegalStateException(e);
        }
    }

    /* (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogoutProcessor#logoutRequest(com.vmware.identity.websso.client.Message)
     */
    @Override
    public void logoutRequest(Message arg0, HttpServletRequest request,
			HttpServletResponse response, String tenant) {
        // incoming logout request from the IDP, need to clean up session info on the server
        // we have no session information stored on the server in this sample implementation

    }

    /* (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogoutProcessor#logoutSuccess(com.vmware.identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void logoutSuccess(Message arg0, HttpServletRequest request,
          HttpServletResponse response) {
        String redirectUrl;
        try {
            ComponentUtils.setSessionCookie(response,
                    null,
                    null);
            // redirect back to logon page, can do something else here
            redirectUrl = arg0.getTarget().replace("/SsoClient/SLO/", "/logout_content/");
            response.sendRedirect(redirectUrl);
        } catch (Exception e) {
            throw new IllegalStateException(e);
        }
    }

	/* (non-Javadoc)
	 * @see com.vmware.identity.websso.client.LogoutProcessor#internalError(java.lang.Exception, javax.servlet.http.HttpServletRequest)
	 */
	@Override
    public void internalError(Exception internalError,
			HttpServletRequest request, HttpServletResponse response) {
		// TODO Auto-generated method stub

	}
}
