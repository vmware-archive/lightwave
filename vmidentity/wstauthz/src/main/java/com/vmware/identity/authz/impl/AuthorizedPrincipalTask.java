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
package com.vmware.identity.authz.impl;

import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Represent tasks that are executed behind the authorization control.
 *
 * Basically it transforms unexpected checked exceptions to ones that indicate
 * bug has occurred.
 */
abstract class AuthorizedPrincipalTask<T> {

   /**
    * Execution of the task
    *
    * @return task result
    * @throws InvalidPrincipalException
    *            propagated from the underlying calls
    */
   final T execute() throws InvalidPrincipalException {

      try {
         return executeInternal();
      } catch (NoPermissionException e) {
         throw newBug(e);
      } catch (NotAuthenticatedException e) {
         throw newBug(e);
      }
   }

   /**
    * @return
    * @throws InvalidPrincipalException
    * @throws NoPermissionException
    * @throws NotAuthenticatedException
    */
   protected abstract T executeInternal() throws InvalidPrincipalException,
      NoPermissionException, NotAuthenticatedException;

   private IllegalStateException newBug(Exception e) {
      return new IllegalStateException("Bug", e);
   }
}
