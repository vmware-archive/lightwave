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

/**
 * SSL Options for TLS, used by platform only.
 */
enum SSLOption
{
   LDAP_OPT_X_TLS_CTX(0x6001),
   LDAP_OPT_X_TLS_PROTOCOL_MIN(0x6007),
   LDAP_OPT_X_TLS_NEWCTX(0x600f);

   private int _code;

   private SSLOption(int code)
   {
      this._code = code;
   }

   public int getCode()
   {
      return this._code;
   }
}
