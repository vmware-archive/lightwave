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
package com.vmware.identity.websso.client;

import java.io.Serializable;

import javax.servlet.http.HttpServletResponse;

/**
 * Data class for caching current HTTPServletResponse error.
 * 
 */

public class ValidationResult implements Serializable {

    private static final long serialVersionUID = 1L;
    private int responseCode;
    private String status;
    private String substatus;

    /**
     * Create result {success}
     */
    public ValidationResult() {
        // preset to return 200 (success)
        setResponseCode(HttpServletResponse.SC_OK);
    }

    /**
     * Create result as specified
     * 
     * @param rc
     * @param s
     * @param ss
     */
    public ValidationResult(int rc, String s, String ss) {
        setResponseCode(rc);
        setStatus(s);
        setSubstatus(ss);
    }

    /**
     * Create result as specified (result code is 200)
     * 
     * @param s
     * @param ss
     */
    public ValidationResult(String s, String ss) {
        setResponseCode(HttpServletResponse.SC_OK);
        setStatus(s);
        setSubstatus(ss);
    }

    /**
     * Create result as specified (result code is 200, substatus is null)
     * 
     * @param s
     */
    public ValidationResult(String s) {
        setResponseCode(HttpServletResponse.SC_OK);
        setStatus(s);
        setSubstatus(null);
    }

    public int getResponseCode() {
        return responseCode;
    }

    public void setResponseCode(int responseCode) {
        this.responseCode = responseCode;
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getSubstatus() {
        return substatus;
    }

    public void setSubstatus(String substatus) {
        this.substatus = substatus;
    }

    public boolean isValid() {
        return responseCode == HttpServletResponse.SC_OK && status == null && substatus == null;
    }

    public boolean isRedirect() {
        return responseCode == HttpServletResponse.SC_FOUND;
    }
}
