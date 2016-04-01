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

import java.io.File;
import java.io.Serializable;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang.Validate;
/**
 * RSAAgentConfig specifies the RSA agent configuration. Note: The sso server act as an rsa agent.
 * @author schai
 */
public class RSAAgentConfig implements Serializable {

    /**
     *
     */
    private static final long serialVersionUID = -1497915281030892482L;

    //Attributes to be used to generate rsa_api.properties file
    private HashMap<String, RSAAMInstanceInfo> _instMap = new HashMap<String, RSAAMInstanceInfo>();
    private String _loginGuide = "Passcode for soft token users:<br>Enter only the generated token code from app<br><br>Passcode for hard token users:<br> Enter pin + generated token code";
    private RSALogLevelType _logLevel = RSALogLevelType.INFO;      //optional, "INFO" if not specified
    private int _logFileSize = 1;       //optional. 1MB default log file size.
    private int _maxLogFileCount = 10;  //optional. max number of log files
    private int _connectionTimeOut = 60;     //optional. default value will be read from config.xml
    private int _readTimeOut = 60;           //optional. default value will be read from config.xml
        //The default encryption key wrapping algorithm list.
    private Set<String> _rsaEncAlgList = new HashSet<String>
                    (Arrays.asList(ENC_ALG_AES128,ENC_ALG_AES192,ENC_ALG_AES256)); //optional. The set default to the list currently provided by RSA.

    //Attribute to be used to construct RSA userID and map to UPN in Lotus.
    private HashMap<String, String> _idsUserIDAttributeMap;  //can be null

    //Constants.
    public static String ENC_ALG_AES128 = "AES/16";
    public static String ENC_ALG_AES192 = "AES/24";
    public static String ENC_ALG_AES256 = "AES/32";

    public enum RSALogLevelType {
        OFF,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    }

    public RSAAgentConfig() {
    }
    /**
     * Minimum ctor with required attributes only
     * @param siteID
     * @param agentName
     * @param sdoptRec
     */
    public RSAAgentConfig(RSAAMInstanceInfo instInfo) {
        Validate.notNull(instInfo, "RSAAMInstanceInfo");

        this.get_instMap().put(instInfo.get_siteID(), instInfo);
    }

    /**
     * Full constructor with optional attribute values.
     * @param _siteID
     * @param _agentName
     * @param _sdconfRec
     * @param _sdoptsRec
     * @param _logLevel
     * @param _logFileSize
     * @param _maxLogFileCount
     * @param _connectionTimeOut
     * @param _readTimeOut
     * @param _idsUserIDAttributeMap
     */
    public RSAAgentConfig(String loginGuide, RSAAMInstanceInfo instInfo, RSALogLevelType _logLevel, int _logFileSize, int _maxLogFileCount, int _connectionTimeOut,
            int _readTimeOut, HashMap<String, String> _idsUserIDAttributeMap, Set<String> _rsaEncAlgList) {
        Validate.notNull(instInfo, "RSAAMInstanceInfo");

        this.get_instMap().put(instInfo.get_siteID(), instInfo);

        if (_loginGuide != null && !_loginGuide.isEmpty()) {
            //this prevent removing the guide
            this._loginGuide = loginGuide;
        }
        if (_logLevel != null) {
            this._logLevel = _logLevel;
        }

        this._logFileSize = _logFileSize;
        this._maxLogFileCount = _maxLogFileCount;
        this._connectionTimeOut = _connectionTimeOut;
        this._readTimeOut = _readTimeOut;
        if (_idsUserIDAttributeMap != null) {
            this._idsUserIDAttributeMap = _idsUserIDAttributeMap;
        }
        if (_rsaEncAlgList != null) {
            this._rsaEncAlgList = _rsaEncAlgList;
        }
    }
    @Override
    public boolean equals(Object another) {
        if ( this == another ) return true;

        if ( !(another instanceof RSAAgentConfig) ) return false;

        RSAAgentConfig that = (RSAAgentConfig)another;

        boolean userIDAttrMapSame = _idsUserIDAttributeMap == that._idsUserIDAttributeMap ||
                (null != _idsUserIDAttributeMap &&_idsUserIDAttributeMap.equals(that._idsUserIDAttributeMap));

        boolean retVal = _loginGuide.equals(that._loginGuide) &&
                _logLevel.equals(that._logLevel) &&
                _logFileSize == that._logFileSize &&
                _maxLogFileCount  == that._maxLogFileCount &&
                _connectionTimeOut == that._connectionTimeOut &&
                 _readTimeOut == that._readTimeOut &&
                 _instMap.equals(that._instMap) &&
                 _rsaEncAlgList.equals(that._rsaEncAlgList) &&
                 userIDAttrMapSame;
        return retVal;
    }

    public HashMap<String, RSAAMInstanceInfo> get_instMap(){
        return _instMap;
    }

    public void set_instMap(HashMap<String, RSAAMInstanceInfo> map){
        if (map != null) {
            this._instMap = map;
        }
        else {
            this.clearInstMap();
        }
    }
    public void add_instInfo(RSAAMInstanceInfo siteInfo){
        if (siteInfo != null) {
            this._instMap.put(siteInfo.get_siteID(), siteInfo);
        }
    }

    public void clearInstMap(){
        this._instMap.clear();
    }

    public RSALogLevelType get_logLevel() {
        return _logLevel;
    }
    /**
     *
     * @param _logLevel   could be null
     */
    public void set_logLevel(RSALogLevelType _logLevel) {
        this._logLevel = _logLevel;
    }
    public int get_logFileSize() {
        return _logFileSize;
    }
    /**
     * @param _logFileSize
     */
    public void set_logFileSize(int _logFileSize) {
        this._logFileSize = _logFileSize;
    }
    public int get_maxLogFileCount() {
        return _maxLogFileCount;
    }
    /**
     * @param _maxLogFileCount
     */
    public void set_maxLogFileCount(int _maxLogFileCount) {
            this._maxLogFileCount = _maxLogFileCount;
    }
    public int get_connectionTimeOut() {
        return _connectionTimeOut;
    }
    /**
     * @param _connectionTimeOut
     */
    public void set_connectionTimeOut(int _connectionTimeOut) {
        this._connectionTimeOut = _connectionTimeOut;
    }
    public int get_readTimeOut() {
        return _readTimeOut;
    }
    /**
     * @param _readTimeOut
     */
    public void set_readTimeOut(int _readTimeOut) {
        this._readTimeOut = _readTimeOut;
    }
    public HashMap<String, String> get_idsUserIDAttributeMap() {
        return _idsUserIDAttributeMap;
    }
    /**
     * @param _idsUserIDAttributeMap
     */
    public void set_idsUserIDAttributeMaps(HashMap<String, String> _idsUserIDAttributeMap) {
        if (_idsUserIDAttributeMap != null) {
            this._idsUserIDAttributeMap = _idsUserIDAttributeMap;
        }
    }
    /**
     * @param idsName
     * @param ldapAttr
     */
    public void add_idsUserIDAttributeMap(String idsName, String ldapAttr) {
        Validate.notEmpty(idsName, "idsName");
        Validate.notEmpty(ldapAttr, "ldapAttr");

        if (this._idsUserIDAttributeMap == null) {
            this._idsUserIDAttributeMap = new HashMap<String,String>();
        }
        this._idsUserIDAttributeMap.put(idsName, ldapAttr);
    }
    public Set<String> get_rsaEncAlgList() {
        return _rsaEncAlgList;
    }
    /**
     * @param _rsaEncAlgList
     */
    public void set_rsaEncAlgList(Set<String> _rsaEncAlgList)
    {
        if (_rsaEncAlgList != null) {
            this._rsaEncAlgList = _rsaEncAlgList;
        }
    }
    /**
     * @return string.  non-null non-empty
     */
    public String get_loginGuide() {
        return _loginGuide;
    }
    public void set_loginGuide(String _loginGuide) {
        if (_loginGuide != null && !_loginGuide.isEmpty()) {
            this._loginGuide = _loginGuide;
        }
    }

}
