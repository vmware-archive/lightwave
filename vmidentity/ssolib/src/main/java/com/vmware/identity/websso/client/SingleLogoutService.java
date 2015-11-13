/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 * ************************************************************************/
package com.vmware.identity.websso.client;

/**
 * Stucture holding single logout service data.
 * 
 */
public class SingleLogoutService {

    private String location;

    private String binding;

    /**
     * SingleLogoutService object. This object correspond to samlm:EndPointType
     * 
     * @param location
     *            Required. URL value.
     * @param binding
     *            Required. uri of the binding type per SAML2.0 metadata
     *            protocal.
     */
    public SingleLogoutService(String location, String binding) {
        this.location = location;
        this.binding = binding;
    }

    public void setLocation(String location) {
        this.location = location;
    }

    public String getLocation() {
        return location;
    }

    public void setBinding(String binding) {
        this.binding = binding;
    }

    public String getBinding() {
        return binding;
    }

}
