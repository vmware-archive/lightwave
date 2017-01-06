/*
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
 */
package com.vmware.identity.websso.client;

/**
 * Stucture holding single logout service data.
 * 
 */
public class SingleLogoutService {

    private String location;

    private String responseLocation;

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

	/**
	 * SingleLogoutService object.
	 * This object correspond to samlm:EndPointType
	 *
	 * @param location		Required. URL value for SLO request.
	 * @param responseLocation Required. URL value for SLO response.
	 * @param binding		Required. uri of the binding type per SAML2.0 metadata protocal.
	 */
	public SingleLogoutService (String location, String responseLocation, String binding) {
		this.location = location;
		this.responseLocation = responseLocation;
		this.binding = binding;
	}

    public void setLocation(String location) {
        this.location = location;
    }

    public String getLocation() {
        return location;
    }


	public void setResponseLocation (String responseLocation) {
		this.responseLocation = responseLocation;
	}

	public String getResponseLocation() {
		return responseLocation;
	}

    public void setBinding(String binding) {
        this.binding = binding;
    }

    public String getBinding() {
        return binding;
    }

}
