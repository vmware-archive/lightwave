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
package com.vmware.identity.sts.impl;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.authz.PrincipalMembership;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.util.PrincipalIdConvertor;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

/**
 * IDM based implementation of <code>PrincipalMembership</code>. Intended for
 * use when checking user's permissions.
 */
final class PrincipalMembershipImpl implements PrincipalMembership {

   private final static IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(PrincipalMembershipImpl.class);
   private final PrincipalDiscovery principalDiscovery;

   PrincipalMembershipImpl(PrincipalDiscovery principalDiscovery) {
      assert principalDiscovery != null;

      this.principalDiscovery = principalDiscovery;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public boolean isMemberOfSystemGroup(PrincipalId principalId,
      String groupName) throws InvalidPrincipalException, NoSuchIdPException {

      boolean result = false;
      try {
         result = principalDiscovery
            .isMemberOfSystemGroup(PrincipalIdConvertor
               .toIdmPrincipalId(principalId), groupName);
      } catch (com.vmware.identity.sts.idm.InvalidPrincipalException e) {
         log.debug("The user {} does not exist!", principalId);
         throw new InvalidPrincipalException(e, principalId.toString());
      }

      log.trace("The user {} is member of {}: {}", new Object[] { principalId,
         groupName, result });
      return result;
   }

}
