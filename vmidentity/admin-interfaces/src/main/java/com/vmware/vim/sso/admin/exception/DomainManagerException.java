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

public class DomainManagerException extends ADIDPRegistrationServiceException {

   private static final long serialVersionUID = -5820870200173702476L;
   private final String domainName;
   private final int errorCode;

   public DomainManagerException(String name, int errorCode, Throwable cause) {
      super(getDefaultMessage(name, errorCode), cause);
      assert (null != name && !name.trim().isEmpty());
      this.domainName = name;
      this.errorCode = errorCode;
   }

   public DomainManagerException(String name, int errorCode) {
      super(getDefaultMessage(name, errorCode));
      assert (null != name && !name.trim().isEmpty());
      this.domainName = name;
      this.errorCode = errorCode;
   }

   private static String getDefaultMessage(String domainName, int errorCode) {
      return String.format(
            "DomainManager native API error with domainName [%s], errorCode [%d]", domainName, errorCode);
   }

   public String getDomainName() {
      return domainName;
   }

   public int getErrorcode() {
      return errorCode;
   }
}
