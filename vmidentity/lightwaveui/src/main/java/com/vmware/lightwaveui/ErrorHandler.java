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
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@WebServlet("/ErrorHandler")
public class ErrorHandler extends HttpServlet {
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public ErrorHandler() {
        super();
    }

 // Method to handle GET method request.
    public void doGet(HttpServletRequest request,
                      HttpServletResponse response)
              throws ServletException, IOException
    {
    	String querystring = "?" + request.getQueryString();
		String uri = request.getRequestURL().toString();
		response.getWriter().append("Test Served at: ").append(uri + querystring);
        /*Throwable throwable = (Throwable)
        request.getAttribute("javax.servlet.error.exception");
        Integer statusCode = (Integer)
        request.getAttribute("javax.servlet.error.status_code");
        String servletName = (String)
        request.getAttribute("javax.servlet.error.servlet_name");
        if (servletName == null){
           servletName = "Unknown";
        }
        String requestUri = (String)
        request.getAttribute("javax.servlet.error.request_uri");
        if (requestUri == null){
           requestUri = "Unknown";
        }

        // Set response content type
        response.setContentType("text/html");
   
        PrintWriter out = response.getWriter();
  	  	String title = "Error/Exception Information";
        String docType =
        "<!doctype html public \"-//w3c//dtd html 4.0 " +
        "transitional//en\">\n";
        out.println(docType +
          "<html>\n" +
          "<head><title>" + title + "</title></head>\n" +
          "<body bgcolor=\"#f0f0f0\">\n");

        if (throwable == null && statusCode == null){
           out.println("<h2>Error information is missing</h2>");
           out.println("Please return to the <a href=\"" + 
             response.encodeURL("http://localhost:8080/") + 
             "\">Home Page</a>.");
        }else if (statusCode != null){
           out.println("The status code : " + statusCode);
        }else{
           out.println("<h2>Error information</h2>");
           out.println("Servlet Name : " + servletName + 
                               "</br></br>");
           out.println("Exception Type : " + 
                               throwable.getClass( ).getName( ) + 
                               "</br></br>");
           out.println("The request URI: " + requestUri + 
                               "<br><br>");
           out.println("The exception message: " + 
                                   throwable.getMessage( ));
        }
        out.println("</body>");
        out.println("</html>");*/
    }
    
    // Method to handle POST method request.
    public void doPost(HttpServletRequest request,
                       HttpServletResponse response)
        throws ServletException, IOException {
       doGet(request, response);
    }

}
