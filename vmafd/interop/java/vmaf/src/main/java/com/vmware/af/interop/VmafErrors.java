/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.af.interop;

enum VmafErrors {

   ERROR_ACCESS_DENIED(5),
   ERROR_NOT_SUPPORTED(50),
   ERROR_INVALID_PARAMETER(87),
   ERROR_INVALID_COMPUTERNAME(1210),
   ERROR_NO_SUCH_LOGON_SESSION(1312),
   ERROR_WRONG_PASSWORD(1323),
   ERROR_NO_SUCH_DOMAIN(1355),
   NERR_UNKNOWNSERVER(2103),
   NERR_SETUPALREADYJOINED(2691),
   NERR_SETUPNOTJOINED(2692),
   ERROR_BAD_PACKET(9502),
   LW_ERROR_PASSWORD_MISMATCH(40022),
   LW_ERROR_LDAP_NO_SUCH_OBJECT(40318),
   LW_ERROR_KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN(41737);

   private int errorCode;
   private VmafErrors(int errorCode)
   {
      this.errorCode = errorCode;
   }

   public int getErrorCode()
   {
      return errorCode;
   }
}
