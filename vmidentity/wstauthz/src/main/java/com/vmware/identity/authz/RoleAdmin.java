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

package com.vmware.identity.authz;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

/**
 * Role administrator.
 */
public interface RoleAdmin<RoleType> {

   /**
    * Grant role to principal user.
    *
    * @param userId
    *           user identifier; {@code not-null} value is required
    * @param role
    *           role to be granted; {@code not-null} value is required
    * @return whether the role has been granted
    * @throws InvalidPrincipalException
    *            when there is no such user
    */
   boolean grantRole(PrincipalId userId, RoleType role)
      throws InvalidPrincipalException;

   /**
    * Revoke role from principal user.
    *
    * @param userId
    *           user identifier; {@code not-null} value is required
    * @param role
    *           role to be revoked; {@code not-null} value is required
    * @return whether the role has been revoked
    * @throws InvalidPrincipalException
    *            when there is no such user
    */
   boolean revokeRole(PrincipalId userId, RoleType role)
      throws InvalidPrincipalException;

}
