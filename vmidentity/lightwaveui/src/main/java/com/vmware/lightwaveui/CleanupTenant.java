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
import javax.xml.parsers.*;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.xpath.*;
import org.w3c.dom.*;
import org.xml.sax.SAXException;
	
/**
 * Servlet implementation class Login
 */
@WebServlet("/CleanupTenant")
public class CleanupTenant extends HttpServlet {
	private static final long serialVersionUID = 1L;
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public CleanupTenant() {
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
			//deleteXmlNode(tenantName);
			deleteXmlDomNode(tenantName);
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

	private Boolean deleteXmlNode(String domain){
		
		try {
			DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			Document document = dbf.newDocumentBuilder().parse(new File("/opt/vmware/share/config/lightwave-ui-oidc.xml"));

			XPathFactory xpf = XPathFactory.newInstance();
			XPath xpath = xpf.newXPath();
			XPathExpression expression = xpath.compile("/tenants/tenant/[name=" + domain + "]");

			Node tenantNode = (Node) expression.evaluate(document, XPathConstants.NODE);
			tenantNode.getParentNode().getParentNode().removeChild(tenantNode.getParentNode());

			TransformerFactory tf = TransformerFactory.newInstance();
			Transformer t = tf.newTransformer();
			t.transform(new DOMSource(document), new StreamResult(System.out));
			return true;
        	
		} catch(Exception e){
			return false;
		}
	}
	
	private void deleteXmlDomNode(String domain) throws ParserConfigurationException, IOException, SAXException, Exception {
		
		Boolean found = false;
		String filePath = "/opt/vmware/share/config/lightwave-ui-oidc.xml";
		File inputFile = new File(filePath);
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
        Document doc = dBuilder.parse(inputFile);
        doc.getDocumentElement().normalize();
        Node tenants = doc.getElementsByTagName("tenants").item(0);;
        NodeList nList = doc.getElementsByTagName("tenant");
        
        if(nList != null){
	        for (int i = 0; i < nList.getLength(); i++) {
	        	Node node = nList.item(i);
	        	NodeList children = node.getChildNodes();
	        	String name = children.item(0).getTextContent();
	        	if(name.equals(domain)){
	        		tenants.removeChild(node);
	        		found = true;
	        		break;
	        	}
	        }
        }
        
        if(found){
        	TransformerFactory transformerFactory = TransformerFactory.newInstance();
    		Transformer transformer = transformerFactory.newTransformer();
    		DOMSource source = new DOMSource(doc);
    		StreamResult result = new StreamResult(new File(filePath));
    		transformer.transform(source, result);
        }
	}
}
