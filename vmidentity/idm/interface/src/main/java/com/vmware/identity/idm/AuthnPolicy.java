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

/**
 * AuthnPolicy specifies the allowed client authentication methods
 * @author qiangw
 */
/**
 * @author root
 *
 */
public class AuthnPolicy implements Serializable {

    private static final long serialVersionUID = 3313553465520313163L;

    private final boolean _passwordAuthEnabled;
    private final boolean _windowsAuthEnabled;
    private final boolean _certAuthEnabled;
    private final boolean _rsaSecureIDAuthnEnabled;

    private final ClientCertPolicy _clientCertPolicy;

    private RSAAgentConfig _rsaAgentConfig;
    /**
     * Constructor of AuthnPolicy
     * @param password
     * @param windows
     * @param cert
     * @param certPolicy, can be set to null
     */
    public AuthnPolicy(boolean password, boolean windows, boolean cert,
            ClientCertPolicy certPolicy) {
        this._passwordAuthEnabled = password;
        this._windowsAuthEnabled = windows;
        this._certAuthEnabled = cert;
        this._rsaSecureIDAuthnEnabled = false;
        this._clientCertPolicy = certPolicy;
        this._rsaAgentConfig = null;
    }

    /**
     * Constructor of AuthnPolicy
     * @param password
     * @param windows
     * @param cert
     * @param rsaSecureID
     * @param certPolicy, can be set to null
     */
    public AuthnPolicy(boolean password, boolean windows, boolean cert,boolean rsaSecureID,
            ClientCertPolicy certPolicy, RSAAgentConfig rsaConfig) {
        this._passwordAuthEnabled = password;
        this._windowsAuthEnabled = windows;
        this._certAuthEnabled = cert;
        this._rsaSecureIDAuthnEnabled = rsaSecureID;
        this._clientCertPolicy = certPolicy;
        this._rsaAgentConfig = rsaConfig;
    }

    /**
     * get if password authentication is enabled or not
     */
    public boolean IsPasswordAuthEnabled() {
        return this._passwordAuthEnabled;
    }

    /**
     * get if Windows authentication is enabled or not
     */
    public boolean IsWindowsAuthEnabled() {
        return this._windowsAuthEnabled;
    }

    /**
     * get if client certificate authentication is enabled or not
     */
    public boolean IsTLSClientCertAuthnEnabled() {
        return this._certAuthEnabled;
    }

    /**
     * get the client certificate checking policy
     * @return ClientCertPolicy, which can be null
     *  when the client certificate revocation check policy hasn't been configured yet.
     */
    public ClientCertPolicy getClientCertPolicy() {
        return this._clientCertPolicy;
    }

    /**
     * @return if rsa secureID authenticatin enabled.
     */
    public boolean IsRsaSecureIDAuthnEnabled() {
        return _rsaSecureIDAuthnEnabled;
    }

    public RSAAgentConfig get_rsaAgentConfig() {
        return _rsaAgentConfig;
    }

    public void set_rsaAgentConfigs(RSAAgentConfig _rsaAgentConfig) {
        this._rsaAgentConfig = _rsaAgentConfig;
    }

}