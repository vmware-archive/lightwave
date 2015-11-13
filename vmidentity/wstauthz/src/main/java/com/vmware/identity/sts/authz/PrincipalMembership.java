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
package com.vmware.identity.sts.authz;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

/**
 * This interface offers system groups membership checks. Implementations should
 * be provided by integrators of this module.
 */
public interface PrincipalMembership {

   /**
    * Checks whether principal is member of system group. Principals include end
    * users and groups in any back-end identity source (incl. solution users).
    *
    * @param principalId
    *           principal identifier, required
    * @param groupName
    *           system group name, required
    * @return true if the principal is member of the given system group, false
    *         if that group does not exist or the principal is not its member
    * @throws InvalidPrincipalException
    *            when there is no such principal
    */
   boolean isMemberOfSystemGroup(PrincipalId principalId, String groupName)
      throws InvalidPrincipalException;
}
