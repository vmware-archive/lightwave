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
    private boolean isMultiTenant;

    /**
     * Constructs solution details by certificate
     *
     * @param certificate
     *            certificate; requires {@code non-null} value
     */
    public SolutionDetail(X509Certificate certificate) {
        this(certificate, null, false);
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

        this(certificate, description, false);
    }

    /**
     * Constructs solution details by certificate and description
     *
     * @param certificate
     *            certificate; requires {@code non-null} value
     * @param description
     *            the description to set; requires {@code non-null} value
     * @param multiTenant
     *           whether this soluiton user represents a "multi-tenant" solution user
     *           Multi-tenant solution users can only be registered within system tenant.
     *           It is possible to obtain token representing such user within the context of
     *           any tenant.
     *           Sets the common name of the solution user
     */
    public SolutionDetail(X509Certificate certificate, String description, boolean multiTenant) {

        super(description);

        // ValidateUtil.validateNotNull(certificate, "certificate");

        // Lotus is having this as optional for now.
        // ValidateUtil.validateNotNull(description, "description");

        this.certificate = certificate;
        this.isMultiTenant = multiTenant;
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
     * Specifies whether this solution user is multi-tenant.
     * Multi-tenant solution users can only be registered within system tenant.
     * It is possible to obtain token representing such user within the context of
     * any tenant.
     */
    public boolean isMultiTenant()
    {
       return this.isMultiTenant;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Object[] getDetailFields() {
       return new Object[] { getDescription(), getCertificate(), isMultiTenant() };
    }
}
