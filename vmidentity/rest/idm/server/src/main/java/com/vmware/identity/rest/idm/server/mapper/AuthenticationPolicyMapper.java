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
package com.vmware.identity.rest.idm.server.mapper;

import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;

/**
 * Mapper for AUthenticationPolicy on tenant
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class AuthenticationPolicyMapper {

	public static AuthenticationPolicyDTO getAuthenticationPolicyDTO(AuthnPolicy authnPolicy) {

        AuthenticationPolicyDTO authnPolicyDTO = null;
        if(authnPolicy != null) {
        AuthenticationPolicyDTO.Builder authnPolicyBuilder = AuthenticationPolicyDTO.builder();

        authnPolicyBuilder.withPasswordBasedAuthenticationEnabled(authnPolicy.IsPasswordAuthEnabled());
        authnPolicyBuilder.withWindowsBasedAuthenticationEnabled(authnPolicy.IsWindowsAuthEnabled());
        authnPolicyBuilder.withCertificateBasedAuthenticationEnabled(authnPolicy.IsTLSClientCertAuthnEnabled());
        if (authnPolicy.getClientCertPolicy() != null) {
            authnPolicyBuilder.withClientCertificatePolicy(ClientCertificatePolicyMapper.getClientCertificatePolicyDTO(authnPolicy.getClientCertPolicy()));
        }
            authnPolicyDTO = authnPolicyBuilder.build();
        }
        return authnPolicyDTO;
    }

    public static AuthnPolicy getAuthenticationPolicy(AuthenticationPolicyDTO authnPolicy) {

        Boolean pwdAuthnEnabled = authnPolicy.isPasswordBasedAuthenticationEnabled() != null ? authnPolicy.isPasswordBasedAuthenticationEnabled() : false;
        Boolean windowsAUthnEnabled = authnPolicy.isWindowsBasedAuthenticationEnabled() != null ? authnPolicy.isWindowsBasedAuthenticationEnabled() : false;
        Boolean certAuthnEnbaled = authnPolicy.isCertificateBasedAuthenticationEnabled() != null ? authnPolicy.isCertificateBasedAuthenticationEnabled() : false;
        ClientCertPolicy certificatePolicy = authnPolicy.getClientCertificatePolicy() != null ? ClientCertificatePolicyMapper.getClientCertificatePolicy(authnPolicy.getClientCertificatePolicy()) : null;
        return new AuthnPolicy(pwdAuthnEnabled, windowsAUthnEnabled, certAuthnEnbaled, certificatePolicy);
    }

}
