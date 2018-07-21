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

import org.apache.commons.lang.StringEscapeUtils;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

/**
 * Sample page where logged on user might be redirected (this would be actual application content)
 *
 */
@Controller
public class ContentController {
    /**
     * Show some content
     */
    @RequestMapping(value = "/content/{tenant:.*}", method = RequestMethod.GET)
    public String content (HttpServletRequest request, @PathVariable(value = "tenant") String tenant, Model model) {
        try {
            String userIdentity = StringEscapeUtils.escapeJavaScript(ComponentUtils.getSessionCookieValue(request));
            model.addAttribute("user", userIdentity);
            model.addAttribute("logout", request.getRequestURI().replace("/content/", "/logout/"));
            return "content";
        } catch (Exception e) {
            model.addAttribute("Data", e.getMessage() + "\n\n" + ComponentUtils.getStackTrace(e));
            return "error";
        }
    }

    /**
     * Show some content
     */
    @RequestMapping(value = "/logout_content/{tenant:.*}", method = RequestMethod.GET)
    public String logout_content (HttpServletRequest request, @PathVariable(value = "tenant") String tenant, Model model) {
        try {
            String userIdentity = StringEscapeUtils.escapeJavaScript(ComponentUtils.getSessionCookieValue(request));

            model.addAttribute("user", userIdentity);
            model.addAttribute("logon", request.getRequestURI().replace("/logout_content/", "/logon/"));
            return "logout_content";
        } catch (Exception e) {
            model.addAttribute("Data", e.getMessage() + "\n\n" + ComponentUtils.getStackTrace(e));
            return "error";
        }
    }

}
