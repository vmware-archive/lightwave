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
 * Thrown to indicate the domain cannot be reached
 *
 */
public class IdmADDomainUnknownDomainException extends IDMException
{
   private static final long serialVersionUID = 3633109627725071927L;
   //domain trying to reach. value is null when trying to un-join the domain
   private final String domain;

   public IdmADDomainUnknownDomainException(String domain)
   {
      super(String.format("Unknown domain [%s]", domain));
      this.domain = domain;
   }

   /**
    * @return Domain for the AD join / un-join operation. value is {@code null}
    *         for the un-join operation.
    */
   public String getDomain()
   {
      return this.domain;
   }
}
