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
import javax.servlet.http.HttpSession;

import com.vmware.identity.websso.client.AuthnData;
import com.vmware.identity.websso.client.LogonProcessor;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.vim.sso.client.SamlToken;

/**
 * Sample Logon Processor Implementation
 *
 */
public class LogonProcessorImpl implements LogonProcessor {

	/*
	 * (non-Javadoc)
	 *
	 * @see
	 * com.vmware.identity.websso.client.LogonProcessor#authenticationError(
	 * com.vmware.identity.websso.client.Message,
	 * javax.servlet.http.HttpServletResponse)
	 */
	@Override
	public void authenticationError(Message arg0, HttpServletRequest request,
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

	/*
	 * (non-Javadoc)
	 *
	 * @see
	 * com.vmware.identity.websso.client.LogonProcessor#authenticationSuccess
	 * (com.vmware.identity.websso.client.Message,
	 * javax.servlet.http.HttpServletResponse)
	 */
	@Override
	public void authenticationSuccess(Message arg0, HttpServletRequest request,
			HttpServletResponse response) {
		String redirectUrl;
		try {
			AuthnData authnData = (AuthnData) arg0.getMessageData();
			ComponentUtils.setSessionCookie(response, authnData
					.getSubjectData().getSubject(), arg0.getSessionIndex());
			redirectUrl = arg0.getTarget().replace("/SsoClient/SSO/",
					"/content/");

			response.sendRedirect(redirectUrl);

		} catch (Exception e) {
			throw new IllegalStateException(e);
		}
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see
	 * com.vmware.identity.websso.client.LogonProcessor#internalError(java.lang
	 * .Exception, javax.servlet.http.HttpServletRequest)
	 */
	@Override
	public void internalError(Exception internalError,
			HttpServletRequest request, HttpServletResponse response) {
		// TODO Auto-generated method stub
	}

}
