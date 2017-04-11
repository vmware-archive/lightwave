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

import java.io.Serializable;
import java.net.URL;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 *  ClientCertPolicy specifies the client certificate checking policy,
 *      for example: revocation check enabled?, use OCSP? CRL cache size, etc.
 * @author qiangw
 *
 */
public class ClientCertPolicy implements Serializable {

    private static final long serialVersionUID = 3793003706126097551L;

    private boolean _revCheck = true;
    private boolean _ocspEnabled = false;
    private boolean _useCRLAsFailOver = false;
    private boolean _sendOCSPNonce = false;
    private URL _ocspUrl = null;
    private HashMap<String, AlternativeOCSPList> _siteOCSPMap = null;
    private X509Certificate _ocspResponderSigningCert = null;
    private boolean _useCerCRL = true; // Use CRL distribution point in end cert.
    private URL _crlUrl = null; // Override CRL distribution point in end cert.
                              // used in case useCRLInClientCert is false
    private int _crlCacheSize = 512;
    private String[] _oids = null;
    private Certificate[] _trustedCAs = null;
    private boolean _enableHint = false;

    public ClientCertPolicy() {}

    /**
     * @param revCheckEnabled
     * @param ocspEnabled
     * @param useCRLAsFailOver
     * @param sendOCSPNonce
     * @param ocspURL  to be ignored. please use new ctr to specify alternative ocsp.
     * @param ocspResponderCert to be ignored. please use new ctr to specify alternative ocsp.
     * @param useCRLDP
     * @param crlAddress
     * @param crlCacheSize
     * @param oidFilters
     */
    public ClientCertPolicy(boolean revCheckEnabled, boolean ocspEnabled,
                    boolean useCRLAsFailOver, boolean sendOCSPNonce,
                    URL ocspURL, X509Certificate ocspResponderCert,
                    boolean useCRLDP, URL crlAddress, int crlCacheSize,
                    String[] oidFilters) {
        this._revCheck = revCheckEnabled;
        this._ocspEnabled = ocspEnabled;
        this._useCRLAsFailOver = useCRLAsFailOver;
        this._sendOCSPNonce = sendOCSPNonce;
        this._useCerCRL = useCRLDP;
        this._crlUrl = crlAddress;
        this._crlCacheSize = crlCacheSize;
        this._oids = oidFilters;
        this._enableHint = false;
    }

    /**
     * ClientCertPolicy ctr that allow configuring per-site OCSP responder address and signing certificates.
     *
     *
     * @param revCheckEnabled
     * @param ocspEnabled
     * @param useCRLAsFailOver
     * @param sendOCSPNonce
     * @param altOCSPLists
     * @param useCRLDP
     * @param crlAddress
     * @param crlCacheSize
     * @param oidFilters
     */
    public ClientCertPolicy(boolean revCheckEnabled, boolean ocspEnabled,
            boolean useCRLAsFailOver, boolean sendOCSPNonce,
            HashMap<String, AlternativeOCSPList> altOCSPmap,
            boolean useCRLDP, URL crlAddress, int crlCacheSize,
            String[] oidFilters, boolean enableHint) {
        this._revCheck = revCheckEnabled;
        this._ocspEnabled = ocspEnabled;
        this._useCRLAsFailOver = useCRLAsFailOver;
        this._sendOCSPNonce = sendOCSPNonce;
        this._siteOCSPMap = altOCSPmap;
        this._useCerCRL = useCRLDP;
        this._crlUrl = crlAddress;
        this._crlCacheSize = crlCacheSize;
        this._oids = oidFilters;
        this._enableHint = enableHint;
}

    /*
     * if certificate revocation check is enabled or not
     */
    public boolean revocationCheckEnabled() {
        return this._revCheck;
    }

    /*
     * enable or disable certificate revocation check
     */
    public void setRevocationCheckEnabled(boolean enabled) {
        this._revCheck = enabled;
    }

    /*
     * whether or not to use OCSP to do revocation check
     */
    public boolean useOCSP() {
        return this._ocspEnabled;
    }

    /*
     * enable or disable useOCSP
     */
    public void setUseOCSP(boolean ocspEnabled) {
        this._ocspEnabled = ocspEnabled;
    }

    /*
     * whether or not to use CRL as FailOver
     */
    public boolean useCRLAsFailOver() {
        return _useCRLAsFailOver;
    }

    /*
     * enable or disable useCRLAsFailOver
     */
    public void setUseCRLAsFailOver(boolean useCRLAsFailOver){
        this._useCRLAsFailOver = useCRLAsFailOver;
    }

    /*
     * whether or not to send OCSPNonce
     */
    public boolean sendOCSPNonce() {
        return _sendOCSPNonce;
    }

    /*
     * enable or disable sendOCSPNonce
     */
    public void setSendOCSPNonce(boolean sendOCSPNounce) {
        this._sendOCSPNonce = sendOCSPNounce;
    }

    /*
     * get OCSP url
     */
    public URL getOCSPUrl() {
        return this._ocspUrl;
    }

    /*
     * set OCSP url
     */
    public void setOCSPUrl(URL url) {
        this._ocspUrl = url;
    }

    /*
     * get OCSP signing certificate
     */
    public X509Certificate getOCSPResponderSigningCert() {
        return this._ocspResponderSigningCert;
    }

    /*
     * set OCSP signing certificate
     */
    public void setOCSPResponderSigningCert(X509Certificate cert) {
        this._ocspResponderSigningCert = cert;
    }

    /*
     * whether or not to use certificate CRL url
     */
    public boolean useCertCRL() {
        return this._useCerCRL;
    }

    /*
     * enable or disable useCertCRL
     */
    public void setUseCertCRL(boolean useCertCRL) {
        this._useCerCRL = useCertCRL;
    }

    /*
     * get CRL url
     */
    public URL getCRLUrl() {
        return this._crlUrl;
    }

    /*
     * set CRL url
     */
    public void setCRLUrl(URL url) {
        this._crlUrl = url;
    }

    /*
     * get CRL cache size
     */
    public int getCacheSize() {
        return this._crlCacheSize;
    }

    /*
     * set CRL cache size
     */
    public void setCacheSize(int cacheSize) {
        this._crlCacheSize = cacheSize;
    }

    /*
     * get allowed OIDs
     */
    public String[] getOIDs() {
        return this._oids;
    }

    /*
     * set allowed OIDs
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

    /**
     * @return the _siteOCSPList
     */
    public HashMap<String, AlternativeOCSPList> get_siteOCSPList() {
        return _siteOCSPMap;
    }

    /**
     * @param _siteOCSPList the _siteOCSPList to set
     */
    public void set_siteOCSPMap(HashMap<String, AlternativeOCSPList> _siteOCSPMap) {
        this._siteOCSPMap = _siteOCSPMap;
    }

    public boolean getEnableHint() {
      return _enableHint;
    }

    public void setEnableHint(boolean enableHint) {
      this._enableHint = enableHint;
    }
}