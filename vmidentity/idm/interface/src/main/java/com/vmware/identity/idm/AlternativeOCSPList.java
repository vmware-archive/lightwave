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
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.Validate;

public class AlternativeOCSPList implements Serializable {

    /**
     *
     */
    private static final long serialVersionUID = -4705716594192745813L;
    private String _siteID;  //This attribute current is the same as PSC site ID.
    private List<AlternativeOCSP> _ocspList;

    public AlternativeOCSPList(String siteID, List<AlternativeOCSP> ocspList) {
        Validate.notEmpty(siteID, "siteID");

        this._siteID = siteID;
        this.set_ocspList(ocspList);
    }

    public List<AlternativeOCSP> get_ocspList() {
        return _ocspList;
    }

    public void set_ocspList(List<AlternativeOCSP> _ocspList) {
        this._ocspList = _ocspList;
    }

    public String get_siteID() {
        return _siteID;
    }

    /**
     * Add or update (of signing certificagte) an alternative OCSP responder. If the given altOCSP's URL match to one
     *  of the exist alternative OCSP responder, it does a replace with incoming one.
     * @param siteID
     * @param altOCSP
     */
    public void addAlternativeOCSP( AlternativeOCSP altOCSP) {
        Validate.notNull(altOCSP, "altOCSP");

        if (_ocspList == null) {
            _ocspList = new ArrayList<AlternativeOCSP>();
        }

        //remove existing responder that matches the url.
        for (AlternativeOCSP ocsp:_ocspList) {
            if (ocsp.get_responderURL().equals(altOCSP.get_responderURL() )) {
                //do a update of the signing cert.
                ocsp.set_responderSigningCert(altOCSP.get_responderSigningCert());
                return;
            }
        }
        _ocspList.add(altOCSP);
    }

    @Override
    public boolean equals(Object other) {
        boolean result = false;
        if (other instanceof AlternativeOCSPList) {
            AlternativeOCSPList otherAlternativeOCSPList = (AlternativeOCSPList) other;
            if (this._siteID.equals(otherAlternativeOCSPList.get_siteID()) &&
                 ((this._ocspList == null && otherAlternativeOCSPList.get_ocspList() == null) || (this._ocspList != null && this._ocspList.equals(otherAlternativeOCSPList.get_ocspList())) ) ) {
                result = true;
            }
        }
        return result;
    }
    @Override
    public int hashCode() {
        int hash = _siteID.hashCode();

        if (null != _ocspList) {
            hash += _ocspList.hashCode();
        }
        return hash;
    }

}
