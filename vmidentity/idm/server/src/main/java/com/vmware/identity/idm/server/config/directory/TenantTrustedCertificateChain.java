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

package com.vmware.identity.idm.server.config.directory;

import java.io.Serializable;
import java.security.cert.Certificate;
import java.util.ArrayList;

import com.vmware.identity.idm.ValidateUtil;

/**
 * Certificate information representation.
 */
final class TenantTrustedCertificateChain implements Serializable {

	private static final long serialVersionUID = -4278812150257131712L;

	private final String _tenantTrustedChainName; // 'cn' for tenantCredentialObject in ldap
    private final ArrayList<Certificate> _certificateChain;

    public TenantTrustedCertificateChain(String tenantTrustedChainName, ArrayList<Certificate> certificateChain)
    {
        ValidateUtil.validateNotEmpty(tenantTrustedChainName, "Tenant trusted certification chain CN name");
        ValidateUtil.validateNotEmpty(certificateChain, "CertificateChain");
        this._tenantTrustedChainName = tenantTrustedChainName;
        this._certificateChain = certificateChain;
    }

    public String getTenantTrustedCertChainName()
    {
        return this._tenantTrustedChainName;
    }

    public ArrayList<Certificate> getCertificateChain()
    {
        return this._certificateChain;
    }
}
