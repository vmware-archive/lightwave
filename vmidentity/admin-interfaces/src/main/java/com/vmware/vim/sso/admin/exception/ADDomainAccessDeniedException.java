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

public class ADDomainAccessDeniedException extends ServiceException {

   private static final long serialVersionUID = 2479251432586421445L;
   private final String domain;
   private final String username;

   public ADDomainAccessDeniedException(String domain, String username) {
      super(getDefaultMessage(domain, username));
      this.domain = domain;
      this.username = username;
   }

   private static String getDefaultMessage(String domain, String username) {
      return String.format("user [%s] cannot access domain [%s]", username, domain);
   }

   public String getDomain() {
      return domain;
   }

   public String getUsername() {
      return username;
   }
}
