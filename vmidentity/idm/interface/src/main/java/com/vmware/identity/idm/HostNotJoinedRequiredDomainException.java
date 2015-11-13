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

public class HostNotJoinedRequiredDomainException extends IDMException
{
   private static final long serialVersionUID = -8482574744611835748L;
   private final String requiredDomainName;
   private final String joinedDomainName;

   public HostNotJoinedRequiredDomainException(String requiredDomainName,
         String joinedDomainName)
   {
      this("host is not joined to the required domain", requiredDomainName,
            joinedDomainName);
   }

   public HostNotJoinedRequiredDomainException(String message,
         String requiredDomainName, String joinedDomainName)
   {
      super(message);
      this.requiredDomainName = requiredDomainName;
      this.joinedDomainName = joinedDomainName;
   }

   public String getRequiredDomainName()
   {
      return requiredDomainName;
   }

   public String getJoinedDomainName()
   {
      return joinedDomainName;
   }
}
