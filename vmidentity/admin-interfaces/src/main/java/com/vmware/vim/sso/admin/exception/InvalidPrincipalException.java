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
package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods meaning incorrect <i>user id</i>
 * or <i>group id</i> argument
 */
public class InvalidPrincipalException extends ServiceException {

   private static final long serialVersionUID = -4633171543482214996L;
   private final String principal;

   public InvalidPrincipalException(Throwable cause, String principal) {
      super(getDefaultMessage(principal), cause);
      assert(null != principal);
      this.principal = principal;
   }

   public InvalidPrincipalException(String principal) {
      super(getDefaultMessage(principal));
      assert(null != principal);
      this.principal = principal;
   }

   public static String getDefaultMessage(String principal) {
      return "The specified principal (" + principal + ") is invalid.";
   }

   public String getPrincipal() {
      return principal;
   }
}
