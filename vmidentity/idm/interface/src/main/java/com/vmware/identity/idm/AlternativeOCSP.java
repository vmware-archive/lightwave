/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.io.Serializable;
import java.net.URL;
import java.security.cert.X509Certificate;

import org.apache.commons.lang.Validate;

public class AlternativeOCSP implements Serializable {

    /**
     *
     */
    private static final long serialVersionUID = 2291153773678362909L;
    private URL _responderURL;
    private X509Certificate _responderSigningCert;

    public AlternativeOCSP(URL responderURL, X509Certificate signingCert) {
        Validate.notNull(responderURL, "responderURL");

        this._responderURL = responderURL;
        this._responderSigningCert = signingCert;
    }

    public X509Certificate get_responderSigningCert() {
        return _responderSigningCert;
    }

    public void set_responderSigningCert(X509Certificate cert) {
        this._responderSigningCert = cert;
    }

    public URL get_responderURL() {
        return _responderURL;
    }

    @Override
    public boolean equals(Object other) {
        boolean result = true;
        if (other instanceof AlternativeOCSP) {
            AlternativeOCSP otherAlternativeOCSP = (AlternativeOCSP) other;
            if (!this._responderURL.equals(otherAlternativeOCSP.get_responderURL()) ||
                 (this._responderSigningCert != null && !this._responderSigningCert.equals(otherAlternativeOCSP.get_responderSigningCert()) ) ) {
                result = false;
            }
        } else {
            result = false;
        }
        return result;
    }

    @Override
    public int hashCode() {
        int hash = _responderURL.hashCode();

        if (null != _responderSigningCert) {
            hash += _responderSigningCert.hashCode();
        }
        return hash;
    }
}
