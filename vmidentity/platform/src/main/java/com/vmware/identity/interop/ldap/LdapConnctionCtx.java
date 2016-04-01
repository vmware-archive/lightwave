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

import com.sun.jna.Pointer;

class LdapConnectionCtx
{
   private final Pointer conn;
   private final Object sslCertValidationCallback;
   private int sslMinimumProtocol;

   LdapConnectionCtx(Pointer conn, Object sslCertValidationCallback)
   {
      this(conn, sslCertValidationCallback, 0);
   }

   LdapConnectionCtx(Pointer conn, Object sslCertValidationCallback, int sslMinimumProtocol)
   {
       this.conn = conn;
       this.sslCertValidationCallback = sslCertValidationCallback;
       this.sslMinimumProtocol = sslMinimumProtocol;
   }

   Pointer getConnection()
   {
      return this.conn;
   }

   Object getsslCertValidationCallback()
   {
      return this.sslCertValidationCallback;
   }

   int getSSLMinimumProtocol()
   {
       return this.sslMinimumProtocol;
   }
}
