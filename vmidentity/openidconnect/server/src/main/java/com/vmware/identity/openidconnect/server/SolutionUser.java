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

package com.vmware.identity.openidconnect.server;

import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPublicKey;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.PrincipalId;

/**
 * @author Yehia Zayour
 */
public class SolutionUser extends User {
    private final X509Certificate certificate;

    public SolutionUser(
            PrincipalId principalId,
            String tenant,
            X509Certificate certificate) {
        super(principalId, tenant);

        Validate.notNull(certificate, "certificate");
        this.certificate = certificate;
    }

    public RSAPublicKey getPublicKey() {
        return (RSAPublicKey) this.certificate.getPublicKey();
    }
}