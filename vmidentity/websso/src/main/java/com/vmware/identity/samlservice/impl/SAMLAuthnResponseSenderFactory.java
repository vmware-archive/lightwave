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

package com.vmware.identity.samlservice.impl;

import java.util.Locale;

import javax.servlet.http.HttpServletResponse;

import org.springframework.context.MessageSource;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.SAMLResponseSender;
import com.vmware.identity.samlservice.SAMLResponseSenderFactory;
import com.vmware.identity.session.SessionManager;

/**
 * @author schai
 *
 */
public class SAMLAuthnResponseSenderFactory implements SAMLResponseSenderFactory {

        @Override
        public SAMLResponseSender buildResponseSender(String tenant, HttpServletResponse response, Locale locale, String relayState,
                AuthnRequestState reqState, AuthnMethod authMethod, String sessionId, PrincipalId pId,
                MessageSource msgSource, SessionManager manager) {

            return new SAMLAuthnResponseSender(tenant, response,locale, relayState,
                        reqState,authMethod,sessionId, pId,msgSource,manager);
        }


}
