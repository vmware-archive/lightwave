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

package com.vmware.identity.idm.server;

import java.net.URI;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.HashSet;

import com.vmware.identity.interop.ldap.ISslX509VerificationCallback;
import com.vmware.identity.interop.ldap.SslX509EqualityMatchVerificationCallback;

public class LdapCertificateValidationSettings {
    private Collection<X509Certificate> idsTrustedCertificates;
    private Collection<X509Certificate> tenantTrustedCertificates;
    private boolean isLegacy;
    private boolean forceValidation;

    public LdapCertificateValidationSettings(
            Collection<X509Certificate> trustedCertificates) {
        this.idsTrustedCertificates = trustedCertificates;
        this.forceValidation = false;
        // when only trusted certificates are set even if they are null or empty is not considered legacy
        this.isLegacy = false;
    }

    public LdapCertificateValidationSettings(
            Collection<X509Certificate> idsTrustedCertificates,
            Collection<X509Certificate> tenantTrustedCertificates) {
        this(idsTrustedCertificates, tenantTrustedCertificates, false);
    }

    public LdapCertificateValidationSettings(
            Collection<X509Certificate> idsTrustedCertificates,
            Collection<X509Certificate> tenantTrustedCertificates,
            boolean forceValidation) {
        this.idsTrustedCertificates = idsTrustedCertificates;
        this.tenantTrustedCertificates = tenantTrustedCertificates;
        this.isLegacy = idsTrustedCertificates == null
                || idsTrustedCertificates.size() == 0;
        this.forceValidation = forceValidation;
    }

    public Collection<X509Certificate> getTrustedCertificates() {
        return isLegacy ? tenantTrustedCertificates : idsTrustedCertificates;
    }

    public boolean isForceValidation() {
        return forceValidation;
    }

    public boolean isLegacy() {
		return isLegacy;
	}

    public ISslX509VerificationCallback getCertVerificationCallback(URI uri) {
        if (this.getTrustedCertificates() == null)
            return new SslX509EqualityMatchVerificationCallback(null);
        else
            return new SslX509EqualityMatchVerificationCallback(
                    new HashSet<X509Certificate>(this.getTrustedCertificates()));

    }
}
