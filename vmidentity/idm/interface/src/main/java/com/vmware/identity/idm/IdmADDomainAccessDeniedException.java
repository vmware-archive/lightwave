/*
 *
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
 *
 */

package com.vmware.identity.idm;

/**
 * Thrown to indicate user authentication error when trying to
 * access the server
 *
 */
public class IdmADDomainAccessDeniedException extends IDMException {

   private static final long serialVersionUID = -2166752001158011136L;
   //domain trying to reach. value is null when trying to un-join the domain
   private final String domain;
   private final String username;

   public IdmADDomainAccessDeniedException(String domain, String username)
   {
      super(String.format("access denied for accessing domain [%s], username [%s]", domain, username));
      this.domain = domain;
      this.username = username;
   }

   /**
    * @return Domain for the AD join / un-join operation. value is {@code null}
    *         for the un-join operation.
    */
   public String getDomain()
   {
      return domain;
   }

   public String getUsername()
   {
      return username;
   }
}
