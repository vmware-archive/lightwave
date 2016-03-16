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
import java.util.Arrays;

import org.apache.commons.lang.Validate;

/**
 * @author schai
 *
 */
public class RSAAMInstanceInfo implements Serializable {


    /**
     * Rsa secureID instance-specific configuration. It is expect to have one instance
     * located on each sso site.
     */
    private static final long serialVersionUID = 1039350323429682674L;
    //Instance attributes to be used to generate rsa_api.properties file
    private final String _siteID;                 //Required. Default?
    private String _agentName;              //required.

    private byte[] _sdconfRec;                //required. sdconf.rec file
    private byte[] _sdoptsRec;                //optional. sdopts.rec file

    public RSAAMInstanceInfo(String siteID, String agentName, byte[] sdconfRec, byte[] sdoptsRec) {
        Validate.notEmpty(siteID, "siteID");
        Validate.notEmpty(agentName, "agentName");
        Validate.notNull(sdconfRec, "sdconfRec");

        this._siteID = siteID;
        this._agentName = agentName;
        this._sdconfRec = sdconfRec;
        this._sdoptsRec = sdoptsRec;
    }
    @Override
    public boolean equals(Object another) {
        if ( this == another ) return true;

        if ( !(another instanceof RSAAMInstanceInfo) ) return false;

        RSAAMInstanceInfo that = (RSAAMInstanceInfo)another;
        boolean retVal = _siteID.equals(that._siteID) &&
                _agentName.equals(that._agentName) &&
                Arrays.equals(_sdconfRec, that._sdconfRec) &&
                Arrays.equals(_sdoptsRec, that._sdoptsRec);
        return retVal;
    }
    public String get_siteID() {
        return _siteID;
    }

    public String get_agentName() {
        return _agentName;
    }

    public void set_agentName(String agentName) throws InvalidArgumentException  {
        if (agentName == null || agentName.isEmpty()) {
            throw new InvalidArgumentException("Null or empty RSA agent name.");
        }
        this._agentName = agentName;
    }

    public byte[] get_sdconfRec() {
        return _sdconfRec;
    }

    public void set_sdconfRec(byte[] _sdconfRec) throws InvalidArgumentException {
        if (_sdconfRec == null || _sdconfRec.length < 1) {
            throw new InvalidArgumentException("Null or empty sd_conf.rec file.");
        }
        this._sdconfRec = _sdconfRec;
    }

    public byte[] get_sdoptsRec() {
        return this._sdoptsRec;
    }


    public void set_sdoptsRec(byte[] _sdoptsRec) {
        this._sdoptsRec = _sdoptsRec;
    }
}
