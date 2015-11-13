/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;


public class LdapSetOptionException extends LdapException
{
   private static final long serialVersionUID = 4508186671490025836L;
   private final LdapOption opt;

   public LdapSetOptionException (int errorCode, LdapOption opt)
   {
       super(errorCode, "Ldap set option failed");
       this.opt = opt;
   }

   public LdapOption getLdapOption()
   {
      return opt;
   }
}
