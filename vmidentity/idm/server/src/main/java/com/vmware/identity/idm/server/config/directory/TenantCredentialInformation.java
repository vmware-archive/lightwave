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
import java.security.PrivateKey;
import java.util.ArrayList;

import com.vmware.identity.idm.ValidateUtil;

/**
 * Certificate information representation.
 */
final class TenantCredentialInformation implements Serializable {

    private static final long serialVersionUID = -6156474495173983411L;

    private String _tenantCredName; // 'cn' for tenantCredentialObject in ldap
    private PrivateKey _privateKey;
    private ArrayList<Certificate> _certificateChain;

    public TenantCredentialInformation(String tenantCredName, PrivateKey privateKey, ArrayList<Certificate> certificateChain)
    {
        ValidateUtil.validateNotEmpty(tenantCredName, "TenantCredential CN name");
        ValidateUtil.validateNotNull( privateKey, "PrivateKey" );
        ValidateUtil.validateNotEmpty(certificateChain, "CertificateChain");
        this._tenantCredName = tenantCredName;
        this._privateKey = privateKey;
        this._certificateChain = certificateChain;
    }

    public String getTenantCredName()
    {
        return this._tenantCredName;
    }

    public PrivateKey getPrivateKey()
    {
        return this._privateKey;
    }

    public ArrayList<Certificate> getCertificateChain()
    {
        return this._certificateChain;
    }
}
