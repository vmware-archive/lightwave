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

package com.vmware.vim.sso.admin;


import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.ADDomainAccessDeniedException;
import com.vmware.vim.sso.admin.exception.ADDomainAlreadyJoinedException;
import com.vmware.vim.sso.admin.exception.ADDomainNotJoinedException;
import com.vmware.vim.sso.admin.exception.ADDomainUnknownDomainException;
import com.vmware.vim.sso.admin.exception.ADIDSAlreadyExistException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

public interface SystemManagement {

   /**
    * Queries the Active Directory Join Status of the system on which the
    * STS Admin Server is executing.
    * @return Active Directory Join Status.
    *         <ul>
    *         <li>When the system is join to an AD domain, the
    *         {@link ActiveDirectoryJoinInfo#joinStatus} will be a string defined by
    *         {@link ActiveDirectoryJoinInfo.JoinStatus#ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN},
    *         with other fields specifying additional information about the joined domain.</li>
    *         <li>When the system is not joined to an AD domain, the
    *         {@link ActiveDirectoryJoinInfo#joinStatus} will be a string defined by
    *         {@link ActiveDirectoryJoinInfo.JoinStatus#ACTIVE_DIRECTORY_JOIN_STATUS_WORKGROUP}</li>
    *         </ul>
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   ActiveDirectoryJoinInfo
   getActiveDirectoryJoinStatus() throws NoPermissionException;

   /**
    * Perform the operation of joining the SSO server to the AD domain
    * @param username
    *           cannot be null or empty
    * @param password
    *           cannot be null
    * @param domain
    *           cannot be null or empty
    * @param orgUnit
    *           can be null or empty
    * @throws ADDomainAccessDeniedException
    *            user access denied when trying to access the AD domain
    * @throws ADDomainUnknownDomainException
    *            cannot contact the server when trying to contact the AD domain
    * @throws ADDomainAlreadyJoinedException
    *            SSO server is already joined to AD domain so cannot join to another one
    * @throws NotAuthenticatedException
    *            there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            the required privilege for calling this method is not held
    *            by the caller
    */

   @Privilege(Role.Administrator)
   void joinActiveDirectory(String username, String password, String domain, String orgUnit)
          throws ADDomainAccessDeniedException, ADDomainUnknownDomainException,
          ADDomainAlreadyJoinedException, NotAuthenticatedException, NoPermissionException;

   /**
    * Perform the operation of SSO server leaving the AD domain
    * @param username
    *           cannot be null or empty
    * @param password
    *           cannot be null
    * @throws ADIDSAlreadyExistException
    *            cannot un-join AD domain due to currently registered IWA IDS.
    * @throws ADDomainAccessDeniedException
    *            user access denied when trying to access the AD domain
    * @throws ADDomainUnknownDomainException
    *            cannot contact the server when trying to contact the AD domain
    * @throws ADDomainNotJoinedException
    *            SSO server is not joined to AD domain
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
     */
   @Privilege(Role.Administrator)
   void leaveActiveDirectory(String username, String password)
           throws ADIDSAlreadyExistException, ADDomainAccessDeniedException, ADDomainUnknownDomainException,
           ADDomainNotJoinedException, NotAuthenticatedException, NoPermissionException;
}
