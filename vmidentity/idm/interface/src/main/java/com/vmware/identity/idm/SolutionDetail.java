/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.security.cert.X509Certificate;

public class SolutionDetail extends PrincipalDetail {

    /**
     * Serial version uid
     */
    private static final long serialVersionUID = -2668472390366804922L;
    /**
     * Certificate
     */
    private X509Certificate certificate;

    /**
     * Constructs solution details by certificate
     *
     * @param certificate
     *            certificate; requires {@code non-null} value
     */
    public SolutionDetail(X509Certificate certificate) {
        super(null);
        this.certificate = certificate;
    }

    /**
     * Constructs solution details by certificate and description
     *
     * @param certificate
     *            certificate; requires {@code non-null} value
     * @param description
     *            the description to set; requires {@code non-null} value
     */
    public SolutionDetail(X509Certificate certificate, String description) {

        super(description);

        // ValidateUtil.validateNotNull(certificate, "certificate");

        // Lotus is having this as optional for now.
        // ValidateUtil.validateNotNull(description, "description");

        this.certificate = certificate;
    }

    /**
     * Retrieve solution's certificate
     *
     * @return a valid certificate
     */
    public X509Certificate getCertificate() {
        return this.certificate;
    }

    /**
     * Retrieve solution's certificate
     * we shouldn't expose this set, should only allow initialization
     * from constructor
     * @return a valid certificate
     */
    public void setCertificate(X509Certificate cert) {
        this.certificate = cert;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Object[] getDetailFields() {
       return new Object[] { getDescription(), getCertificate() };
    }
}
