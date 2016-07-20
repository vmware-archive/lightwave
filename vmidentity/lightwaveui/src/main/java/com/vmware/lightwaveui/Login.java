/*
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
 */

package com.vmware.lightwaveui;

import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * Servlet implementation class Login
 */
@WebServlet("/Login")
public class Login extends HttpServlet {
	private static final long serialVersionUID = 1L;
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public Login() {
        super();
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		String[] tenant = request.getParameterValues("tenant");
		String tenantName = "";
		if(tenant != null && tenant.length > 0){
			tenantName = tenant[0];
		}
		
		if(tenantName == null || tenantName == "")
		{
			response.sendError(HttpServletResponse.SC_BAD_REQUEST, "No tenant specifed.");
			return;
		}
		
		String[] clientIds = request.getParameterValues("clientId");
		String clientId = "";
		if(clientIds != null && clientIds.length > 0){
			clientId = clientIds[0];
		}
		String uri = request.getRequestURL().toString();
		String server = uri.split("://")[1].split("/")[0].split(":")[0];
		//String server = "blr-1st-1-dhcp613.eng.vmware.com";
		String client_id = clientId;//"6b96fb61-bf26-40bc-bf23-b6c9b1819bbc";
		String redirect_uri = "https://" + server + "/lightwaveui/Home";
		String openIdConnectUri = "https://" + server + "/openidconnect/oidc/authorize/" + tenantName;
		String args = "?response_type=id_token%20token&response_mode=form_post&client_id=" +
					  client_id + 
					  "&redirect_uri=" + 
					  redirect_uri + 
					  "&state=_state_lmn_&nonce=_nonce_lmn_&scope=openid%20rs_admin_server";
		String authorizeUri = openIdConnectUri + args;
		response.sendRedirect(authorizeUri);
		
	}

	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		doGet(request, response);
	}

}
