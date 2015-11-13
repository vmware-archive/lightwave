/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client;

/**
 * Credential to acquire an authentication token from the token service by
 * username and password
 */
public class UsernamePasswordCredential extends Credential {
    private String subject;
    private String password;

    /**
     *
     * @param subject
     *            The subject for which the token will be issued. Cannot be
     *            <code>null</code>
     * @param password
     *            The password of the authenticating subject. Cannot be
     *            <code>null</code>
     */
    public UsernamePasswordCredential(String subject, String password) {
        ValidateUtil.validateNotNull(subject, "Username");
        ValidateUtil.validateNotNull(password, "Password");
        this.subject = subject;
        this.password = password;
    }

    public String getSubject() {
        return subject;
    }

    public String getPassword() {
        return password;
    }

    @Override
    public String toString() {
        return String.format("UsernamePasswordCredential [subject=%s]", this.subject);
    }
}
