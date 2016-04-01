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

package com.vmware.vim.sso.admin;

import java.io.Serializable;
import java.net.URL;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

/**
 * Represents the client certificate policy
 */
public class ClientCertPolicy implements Serializable {

    private static final long serialVersionUID = 3793003706126097551L;

    private boolean _enabled = true;
    private boolean _ocspEnabled = false;
    private boolean _useCRLAsFailOver = false;
    private boolean _sendOCSPNonce = false;
    private URL _ocspUrl = null;
    private X509Certificate _ocspResponderSigningCert = null;
    private boolean _useCerCRL = true; // Use CRL distribution point in end cert.
    private URL _crlUrl = null; // Override CRL distribution point in end cert.
                              // used in case useCRLInClientCert is false
    private int _crlCacheSize = 512;
    private String[] _oids = null;
    private Certificate[] _trustedCAs = null;
    /**
     * The default constructor. The default values for each fields are:
     *  revocationCheckEnabled: true
     *  useOCSP: false
     *  useCRLAsFailOver: false
     *  sendOCSPNonce: false
     *  ocspUrl: false
     *  ocspResponderSigningCert: null
     *  useCertCRL: true
     *  crlUrl: null
     *  cacheSize: 512
     *  oids: null
     */
    public ClientCertPolicy() {}

    /**
     * The ClientCertPolicy constructor
     *
     * @param revocationCheckEnabled
     * @param useOCSP
     * @param useCRLAsFailOver
     * @param sendOCSPNonce
     * @param ocspUrl
     * @param ocspResponderSigningCert
     * @param useCertCRL
     * @param crlUrl
     * @param cacheSize
     * @param oids
     */
    public ClientCertPolicy(boolean revocationCheckEnabled, boolean useOCSP,
            boolean useCRLAsFailOver, boolean sendOCSPNonce, URL ocspUrl,
            X509Certificate ocspResponderSigningCert, boolean useCertCRL,
            URL crlUrl, int cacheSize, String[] oids) {
        this._enabled = revocationCheckEnabled;
        this._ocspEnabled = useOCSP;
        this._useCRLAsFailOver = useCRLAsFailOver;
        this._sendOCSPNonce = sendOCSPNonce;
        this._ocspUrl = ocspUrl;
        this._ocspResponderSigningCert = ocspResponderSigningCert;
        this._useCerCRL = useCertCRL;
        this._crlCacheSize = cacheSize;
        this._oids = oids;
    }

    /**
     * @return whether certificate revocation check is enabled.
     */
    public boolean revocationCheckEnabled() {
        return this._enabled;
    }

    /**
     * set whether certificate revocation check is enabled.
     * @param enabled
     */
    public void setRevocationCheckEnabled(boolean enabled) {
        this._enabled = enabled;
    }

    /**
     * @return whether OCSP is enabled.
     */
    public boolean useOCSP() {
        return this._ocspEnabled;
    }

    /**
     * set whether OCSP is enabled.
     * @param ocspEnabled
     */
    public void setUseOCSP(boolean ocspEnabled) {
        this._ocspEnabled = ocspEnabled;
    }

    /**
     * @return whether to use CRL as FailOver if OCSP failed.
     */
    public boolean useCRLAsFailOver() {
        return _useCRLAsFailOver;
    }

    /**
     * set whether to use CRL as FailOver if OCSP failed.
     * @param useCRLAsFailOver
     */
    public void setUseCRLAsFailOver(boolean useCRLAsFailOver){
        this._useCRLAsFailOver = useCRLAsFailOver;
    }

    /**
     * @return whether to send OCSP nonce.
     */
    public boolean sendOCSPNonce() {
        return _sendOCSPNonce;
    }

    /**
     * set whether to send OCSP nonce.
     * @param sendOCSPNounce
     */
    public void setSendOCSPNonce(boolean sendOCSPNounce) {
        this._sendOCSPNonce = sendOCSPNounce;
    }

    /**
     * @return OCSP Url
     */
    public URL getOCSPUrl() {
        return this._ocspUrl;
    }

    /**
     * set OCSP Url
     * @param url
     */
    public void setOCSPUrl(URL url) {
        this._ocspUrl = url;
    }

    /**
     * @return OCSP signing certificate
     */
    public X509Certificate getOCSPResponderSigningCert() {
        return this._ocspResponderSigningCert;
    }

    /**
     * set OCSP signing certificate
     * @param cert
     */
    public void setOCSPResponderSigningCert(X509Certificate cert) {
        this._ocspResponderSigningCert = cert;
    }

    /**
     * @return whether or not use CRL Url from certificate.
     */
    public boolean useCertCRL() {
        return this._useCerCRL;
    }

    /**
     * set whether or not use CRL Url from certificate.
     * @param useCertCRL
     */
    public void setUseCertCRL(boolean useCertCRL) {
        this._useCerCRL = useCertCRL;
    }

    /**
     * @return CRL Url
     */
    public URL getCRLUrl() {
        return this._crlUrl;
    }

    /**
     * set CRL Url
     * @param url
     */
    public void setCRLUrl(URL url) {
        this._crlUrl = url;
    }

    /**
     * @return the cache size for CRLs
     */
    public int getCacheSize() {
        return this._crlCacheSize;
    }

    /**
     * set the cache size for CRLs
     * @param cacheSize
     */
    public void setCacheSize(int cacheSize) {
        this._crlCacheSize = cacheSize;
    }

    /**
     * @return OID strings used to filter certificates.
     */
    public String[] getOIDs() {
        return this._oids;
    }

    /**
     * set OID strings used to filter certificates.
     * @param oids
     */
    public void setOIDs(String[] oids) {
        this._oids = oids;
    }

    /**
     * get trusted CAs for this tenant
     */
    public Certificate[] getTrustedCAs() {
        return this._trustedCAs;
    }

    /**
     * set trusted CAs for this tenant
     */
    public void setTrustedCAs(Certificate[] certs) {
        this._trustedCAs = certs;
    }
}