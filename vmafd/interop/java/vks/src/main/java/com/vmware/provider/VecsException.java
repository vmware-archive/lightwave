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

package com.vmware.provider;

import com.vmware.identity.vecs.VecsGenericException;

/**
 * Generic exception for any problem that happened in the Vecs native interop.
 */
public class VecsException extends RuntimeException
{
   private static final long serialVersionUID = -5384054043778004063L;

   private final int _errorCode;
   private final String _errorMsg;

   VecsException(String message, int errorCode) {
      _errorCode = errorCode != 0 ? errorCode : -1;
      _errorMsg = message;
   }
   
   VecsException(VecsGenericException vge) {
      int errorCode = vge.getErrorCode();
      _errorCode = errorCode != 0 ? errorCode : -1;
      _errorMsg = vge.getMessage();
      initCause(vge);
   }

   public int getErrorCode() {
      return _errorCode;
   }

    public String getMessage() {
        return String.format(
                  "Native platform error [code: %d][%s]",
                  _errorCode,
                  _errorMsg);
    }
}
