/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * ********************************************************************
 */
package com.vmware.identity.heartbeat;

/**
 * Generic exception for any problem that happened in the Vecs native interop.
 */
public class VmAfdGenericException extends RuntimeException
{
   private static final long serialVersionUID = -5384054043778004063L;

   private final int _errorCode;
   private final String _errorMsg;

   VmAfdGenericException(String message, int errorCode)
   {
      _errorCode = errorCode != 0 ? errorCode : -1;
      _errorMsg = message;
   }

   public int getErrorCode()
   {
      return _errorCode;
   }

    public String getMessage()
    {
        return String.format(
                  "Native platform error [code: %d][%s]",
                  _errorCode,
                  _errorMsg);
    }
}
