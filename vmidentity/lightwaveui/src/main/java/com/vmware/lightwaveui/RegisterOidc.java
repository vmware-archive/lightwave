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
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

@WebServlet("/RegisterOidc")
public class RegisterOidc extends HttpServlet {
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public RegisterOidc() {
        super();
    }

    // Method to handle GET method request.
    public void doGet(HttpServletRequest request,
                      HttpServletResponse response)
              throws ServletException, IOException
    {
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
		String[] clientIds = request.getParameterValues("clientId");
		String clientId = "";
		if(clientIds != null && clientIds.length > 0){
			clientId = clientIds[0];
		}
		
		if(clientId == null || clientId == "")
		{
			response.sendError(HttpServletResponse.SC_BAD_REQUEST, "No clientId specifed.");
			return;
		}
		
		Boolean success = saveToDisk(tenantName, clientId);
		
		if(!success){
			response.sendError(HttpServletResponse.SC_BAD_REQUEST, "Unable to save the clientId for tenant specifed.");
			return;
		}
		response.setStatus(HttpServletResponse.SC_OK);
    }
    
    // Method to handle POST method request.
    public void doPost(HttpServletRequest request,
                       HttpServletResponse response)
        throws ServletException, IOException {
       doGet(request, response);
    }
    
    private Boolean saveToDisk(String domain, String clientId){
		String filePath = "/opt/vmware/share/config/lightwave-ui-oidc.xml";
		File f = new File(filePath);
		Boolean success = true;
		if(!f.exists()) { 
			success = createNewXml(filePath);
		}
		
		if(success){
			success = addNewXmlNode(filePath, domain, clientId);
		}
		return success;
	}
    
    private Boolean createNewXml(String xmlFile){
		
		try {
				DocumentBuilderFactory documentBuilderFactory = DocumentBuilderFactory.newInstance();
		        DocumentBuilder documentBuilder = documentBuilderFactory.newDocumentBuilder();
		        Document document = documentBuilder.newDocument();
		        Element root = document.createElement("tenants");
		        document.appendChild(root);
	        	writeXmlToDisk(xmlFile, document);
	        	return true;
        	
		} catch(Exception e) {
			 return false;
		}
	}
	
	private Boolean addNewXmlNode(String xmlFile, String domain, String clientId){
		
		try {
		        DocumentBuilderFactory documentBuilderFactory = DocumentBuilderFactory.newInstance();
		        DocumentBuilder documentBuilder = documentBuilderFactory.newDocumentBuilder();
		        Document document = documentBuilder.parse(xmlFile);
		        Element root = document.getDocumentElement();
		        
		        Element tenantNode = document.createElement("tenant");
		        Element name = document.createElement("name");
		        name.appendChild(document.createTextNode(domain));
		        Element clientIdNode = document.createElement("clientId");
		        clientIdNode.appendChild(document.createTextNode(clientId));
		        tenantNode.appendChild(name);
		        tenantNode.appendChild(clientIdNode);
		
		        root.appendChild(tenantNode);
	        	writeXmlToDisk(xmlFile, document);
	        	return true;
        	
		} catch(Exception e){
			return false;
		}
	}

	private void writeXmlToDisk(String xmlFile, Document document)
			throws TransformerFactoryConfigurationError, TransformerConfigurationException, TransformerException {
		DOMSource source = new DOMSource(document);
        TransformerFactory transformerFactory = TransformerFactory.newInstance();
        Transformer transformer = transformerFactory.newTransformer();
        StreamResult result = new StreamResult(xmlFile);
        transformer.transform(source, result);
	}

}
