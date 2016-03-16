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
 * This exception is thrown by service methods indicating that host is not
 * joined the required domain
 */
public class HostNotJoinedRequiredDomainException extends ADIDPRegistrationServiceException {

   private static final long serialVersionUID = 7774656340218069416L;
   private final String requiredDomainName;
   private final String joinedDomainName;

   public HostNotJoinedRequiredDomainException(String requiredDomainName,
         String joinedDomainName) {
      super(getDefaultMessage(requiredDomainName, joinedDomainName));
      assert (null != requiredDomainName && !requiredDomainName.trim().isEmpty());
      this.requiredDomainName = requiredDomainName;
      this.joinedDomainName = joinedDomainName;
   }

   public HostNotJoinedRequiredDomainException(String requiredDomainName,
         String joinedDomainName, Throwable t) {
      super(getDefaultMessage(requiredDomainName, joinedDomainName), t);
      assert (null != requiredDomainName && !requiredDomainName.trim().isEmpty());
      this.requiredDomainName = requiredDomainName;
      this.joinedDomainName = joinedDomainName;
   }

   private static String getDefaultMessage(String requiredDomanName,
         String joinedDomainName) {
      if (joinedDomainName == null || joinedDomainName.isEmpty())
      {
          return "To support native AD, the host is required to join properly.";
      }

      return String.format(
            "The host is required to join to domain [%s] but joined to [%s]",
            requiredDomanName, joinedDomainName);
   }

   public String getRequiredDomainName() {
      return requiredDomainName;
   }

   public String getJoinedDomainName() {
      return joinedDomainName;
   }
}
