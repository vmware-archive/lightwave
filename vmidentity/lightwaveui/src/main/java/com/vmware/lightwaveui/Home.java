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

package com.vmware.lightwaveui;
import java.io.IOException;
import java.lang.StringBuilder;
import java.util.Map;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


@WebServlet("/Home")
public class Home extends HttpServlet {
	private static final long serialVersionUID = 1L;
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public Home() {
        super();
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		String querystring = "?" + request.getQueryString();
		String uri = request.getRequestURL().toString();
		response.getWriter().append("Latest Test Served at: ").append(uri + querystring);
	}

	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		try{
		String querystring = "?" + request.getQueryString();
		String uri = request.getRequestURL().toString();
		
		String[] values_state = request.getParameterValues("state");
		String value_state = "";
		if(values_state != null && values_state.length > 0){
			value_state = values_state[0];
		}
		
		String[] values_id_token = request.getParameterValues("id_token");
		String value_id_token = "";
		if(values_id_token != null && values_id_token.length > 0){
			value_id_token = values_id_token[0];
		}
		
		String[] values_access_token = request.getParameterValues("access_token");
		String value_access_token = "";
		if(values_access_token != null && values_access_token.length > 0){
			value_access_token = values_access_token[0];
		}
		
		String[] values_token_type = request.getParameterValues("token_type");
		String value_token_type = "";
		if(values_token_type != null && values_token_type.length > 0){
			value_token_type = values_token_type[0];
		}
		
		String[] values_expires_in = request.getParameterValues("expires_in");
		String value_expires_in = "";
		if(values_expires_in != null && values_expires_in.length > 0){
			value_expires_in = values_expires_in[0];
		}
		
		 Map<String, String[]> map = request.getParameterMap();
		
		StringBuilder builder = new StringBuilder();
		
		for (Map.Entry<String, String[]> entry : map.entrySet())
		{
			StringBuilder values = new StringBuilder();
			for(String value : entry.getValue())
			{
				values.append(value + "|");
			}
			builder.append("Key: " + entry.getKey() + "Values: " + values + " \n");
		}
		builder.append("URI: " + uri + querystring);
		response.getWriter().append("Test Served at: ").append(builder.toString());
		String protocol = uri.split("://")[0];
		String hostname = uri.split("://")[1].split("/")[0];
		String baseuri = protocol + "://" + hostname + "/lightwaveui/app/index.html#/home?";
		String stateUri = "state=" + value_state;
		String idTokenUri = "id_token=" + value_id_token;
		String accessTokenUri = "access_token=" + value_access_token;
		String tokenTypeUri = "token_type=" + value_token_type;
		String expiresInUri = "expires_in=" + value_expires_in;
		String fullUri = baseuri + stateUri + "&" + idTokenUri + "&" + accessTokenUri + "&" + tokenTypeUri + "&" + expiresInUri;
		response.sendRedirect(fullUri);
		} catch(Exception exc)
			{
				String message = " Message: " + exc.getMessage();
				String querystring = "?" + request.getQueryString();
				String uri = request.getRequestURL().toString();
				response.getWriter().append("Home Served at: ").append(uri + querystring + message);
			}
	}
}
