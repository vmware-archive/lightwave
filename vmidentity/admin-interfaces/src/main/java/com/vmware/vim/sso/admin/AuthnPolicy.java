/**
 *
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

package com.vmware.vim.sso.admin;

import java.io.Serializable;

/**
 * Represents the client authentication policy for a domain.
 *
 */
public class AuthnPolicy implements Serializable {

    private static final long serialVersionUID = 3313553465520313163L;

    private boolean _passwordAuthEnabled;
    private boolean _windowsAuthEnabled;
    private boolean _certAuthEnabled;
    private ClientCertPolicy _clientCertPolicy;

    /**
     * Constructor of AuthnPolicy.
     * @param password - whether the password authentication is allowed.
     * @param windows - whether the Windows authentication is allowed.
     * @param cert - whether the client certificate authentication is allowed.
     * @param certPolicy - reference to a ClientCertPolicy object.
     */
    public AuthnPolicy(boolean password, boolean windows, boolean cert,
            ClientCertPolicy certPolicy) {
        this._passwordAuthEnabled = password;
        this._windowsAuthEnabled = windows;
        this._certAuthEnabled = cert;
        this._clientCertPolicy = certPolicy;
    }

    /**
     * @return Whether password authentication is enabled.
     */
    public boolean IsPasswordAuthEnabled() {
        return this._passwordAuthEnabled;
    }

    /**
     * @return Whether Windows authentication is enabled.
     */
    public boolean IsWindowsAuthEnabled() {
        return this._windowsAuthEnabled;
    }

    /**
     * @return Whether client certificate authentication is enabled.
     */
    public boolean IsTLSClientCertAuthnEnabled() {
        return this._certAuthEnabled;
    }

    /**
     * @return the reference to the client certificate policy instance.
     */
    public ClientCertPolicy getClientCertPolicy() {
        return this._clientCertPolicy;
    }
}
