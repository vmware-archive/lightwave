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

import java.io.File;
import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.w3c.dom.Node;

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
		try{
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
			tenantName = tenantName.toLowerCase();
			String uri = request.getRequestURL().toString();
			String protocol = uri.split("://")[0];
			String hostname = uri.split("://")[1].split("/")[0];
			String client_id = getClientId(tenantName);
			String redirect_uri = protocol + "://" + hostname + "/lightwaveui/Home";
			String openIdConnectUri = protocol + "://" + hostname + "/openidconnect/oidc/authorize/" + tenantName;
			String args = "?response_type=id_token%20token&response_mode=form_post&client_id=" +
						  client_id +
						  "&redirect_uri=" +
						  redirect_uri +
						  "&state=_state_lmn_&nonce=_nonce_lmn_&scope=openid%20rs_admin_server";
			String authorizeUri = openIdConnectUri + args;
			response.sendRedirect(authorizeUri);
		} catch(Exception exc)
		{
			String message = " Message: " + exc.getMessage();
			String querystring = "?" + request.getQueryString();
			String uri = request.getRequestURL().toString();
			response.getWriter().append("Error: ").append(uri + querystring + message);
		}
	}
	
	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		doGet(request, response);
	}
	
	private String getClientId(String domain) throws ParserConfigurationException, IOException, SAXException, Exception {
		String clientId = "";
		Boolean found = false;
		File inputFile = new File("/opt/vmware/share/config/lightwave-ui-oidc.xml");
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
        Document doc = dBuilder.parse(inputFile);
        doc.getDocumentElement().normalize();
        NodeList nList = doc.getElementsByTagName("tenant");
        
        if(nList != null){
	        for (int i = 0; i < nList.getLength(); i++) {
	        	Node node = nList.item(i);
	        	NodeList children = node.getChildNodes();
	        	String name = children.item(0).getTextContent();
	        	
	        	if(name.toLowerCase().equals(domain)){
	        		clientId = children.item(1).getTextContent();
	        		found = true;
	        		break;
	        	}
	        }
        }
        
        if(!found){
        	throw new Exception("No OIDC client registered for tenant " + domain);
        }
        return clientId;
	}
}
