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

import java.io.Serializable;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;

import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.CertificateUtil;
import com.vmware.identity.idm.ValidateUtil;

/**
 * Represents a certificate that the IDM layer manages in its certificate store.
 * <p>
 * The certificates are uniquely identified by their SHA-1 fingerprints. The
 * fingerprints are expected to be formatted like "XX:XX:...:XX" where "XX" are
 * the hexadecimal representations of the fingerprint's bytes. The fingerprints
 * are case-insensitive.
 */
public class IdmCertificate implements Serializable {

    /**
     * Serial version id
     */
    private static final long serialVersionUID = -3111398315669193988L;

    private byte[] certBytes;

    private CertificateType certType;

    private X509Certificate cert;

    /**
     * Creating an IdmCertificate object.
     * @param certfificate  required
     * @param type          required
     */
    public IdmCertificate(X509Certificate certificate, CertificateType type) {
        ValidateUtil.validateNotNull(certificate, "certificate");
        ValidateUtil.validateNotNull(type, "type");
        this.cert = certificate;
        this.certType = type;
    }

    public X509Certificate getCertificate() {
        return this.cert;
    }

    public byte[] getCertBytes() {
        return this.certBytes;
    }

    public CertificateType getCertType() {
        return this.certType;
    }

    public void setCertType(CertificateType type) {
        ValidateUtil.validateNotNull(type, "type");
        this.certType = type;
    }

    /**
     * Gets SHA-1 hash over the given certificate and return a properly
     * formatted figureprint string. This is similar to the Thumbnail concept in
     * .NET, except Java doesn't support this as a property by default. This
     * field could be used to uniquely identify a certificate.
     */
    public String getFingerprint() throws CertificateEncodingException,
            NoSuchAlgorithmException, CertificateEncodingException {
        try {
            return CertificateUtil.generateFingerprint(this.cert);

        } catch (CertificateEncodingException e) {
            throw e;
        } catch (NoSuchAlgorithmException e) {
            throw e;
        }
    }
}
