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
package com.vmware.identity.sts.auth.impl;

import java.util.HashSet;
import java.util.Set;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.AuthenticatorFactory;
import com.vmware.identity.sts.idm.IdmTenantServices;

/**
 * Implementation of {@link AuthenticatorFactory} returning
 * {@link CompositeAuthenticator} with {@link UNTAuthenticator},
 * {@link BSTAuthenticator}, {@link BETAuthenticator} and
 * {@link SamlTokenAuthenticator} inside.
 */
public final class CompositeAuthenticatorFactory implements
   AuthenticatorFactory {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(CompositeAuthenticatorFactory.class);
   private final IdmTenantServices idm;

   public CompositeAuthenticatorFactory(IdmTenantServices idm) {
      assert idm != null;

      this.idm = idm;
   }

   @Override
   public Authenticator getAuthenticator(String tenantName,
      TokenValidator validator) {

      assert tenantName != null;
      assert validator != null;
      log.debug("Getting CompositeAuthenticator for tenant:{}", tenantName);

      Set<Authenticator> authenticators = new HashSet<Authenticator>();
      final com.vmware.identity.sts.idm.Authenticator authService = idm
         .authService(tenantName);
      authenticators.add(new UNTAuthenticator(authService));
      authenticators.add(new BSTAuthenticator(idm
         .principalDiscovery(tenantName)));
      authenticators.add(new BETAuthenticator(authService));
      authenticators.add(new SamlTokenAuthenticator(validator));
      authenticators.add(new UserCertAuthenticator(authService));

      return new CompositeAuthenticator(authenticators);
   }
}
