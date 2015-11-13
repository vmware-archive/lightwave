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

package com.vmware.identity;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletRequestWrapper;

/**
 * Wrapper over httpServletRequest to set secure flag for request.
 * The request scheme is used by spring security workflow to generate redirect url.
 *
 * This ensures redirect url contains https url.
 */
public class SecurityRequestWrapper extends HttpServletRequestWrapper {
    public static final String VMWARE_CLIENT_CERT_AUTH = "VMWARE_CLIENT_CERT";

    public SecurityRequestWrapper(HttpServletRequest request) {
      super(request);
    }

    @Override
    public boolean isSecure() {
          return true;
       }

    @Override
    public String getScheme() {
          return "https";
       }

    @Override
    public StringBuffer getRequestURL() {
        String urlStr = super.getRequestURL().toString();
        return(new StringBuffer( urlStr.replaceFirst("http:", "https:")) );
    }

    /**
     * Returns "VMWARE_CLIENT_CERT" for local request. This allows sso to use
     * client certificate (if provided) from ServerletRequest header.
     */
    @Override
    public String getAuthType() {
        if (super.getScheme().equals("http")) {
            return this.VMWARE_CLIENT_CERT_AUTH;
        } else {
            return null;
        }
    }

}
