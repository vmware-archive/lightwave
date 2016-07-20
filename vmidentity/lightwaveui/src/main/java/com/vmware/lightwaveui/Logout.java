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
import java.util.Collection;
import java.util.Map;

import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * Servlet implementation class Home
 */
@WebServlet("/Logout")
public class Logout extends HttpServlet {
	private static final long serialVersionUID = 1L;
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public Logout() {
        super();
        // TODO Auto-generated constructor stub
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		String[] values_id_token = request.getParameterValues("id_token");
		String value_id_token = "";
		if(values_id_token != null && values_id_token.length > 0){
			value_id_token = values_id_token[0];
		}
		
		String[] values_state = request.getParameterValues("state");
		String value_state = "";
		if(values_state != null && values_state.length > 0){
			value_state = values_state[0];
		}
		
		String uri = request.getRequestURL().toString();
		String server = uri.split("://")[1].split("/")[0].split(":")[0];
		//String server = "blr-1st-1-dhcp613.eng.vmware.com";
		String postLogoutRedirectUri = "https://" + server + "/lightwaveui";
		String openIdConnectUri = "https://" + server + "/openidconnect/logout";
		String args = "?id_token_hint=" + value_id_token +
					  "&post_logout_redirect_uri=" + postLogoutRedirectUri +
					  "&state=" + value_state;
		String logoutUri = openIdConnectUri + args;
		response.sendRedirect(logoutUri);
	}

	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		doGet(request, response);
	}
}
