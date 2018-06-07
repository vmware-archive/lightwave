/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.openidconnect.server;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;

import javax.servlet.http.HttpServletRequest;

public interface FederatedIdentityProcessor {

    // IDP-initiated flow processing (called from /federate endpoint handler)
  HttpResponse processRequest(
                    HttpServletRequest request,
                    FederationRelayState relayState,
                    IDPConfig idpConfig
                ) throws Exception ;

  // SP-initiated flow (called from oidc's authz endpoint)
  HttpResponse processAuthRequestForFederatedIDP(
          AuthenticationRequest request,
          String tenant,
          IDPConfig idpConfig
      ) throws ServerException;
}
