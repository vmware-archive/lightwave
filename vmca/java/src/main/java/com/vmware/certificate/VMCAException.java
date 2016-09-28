/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

package com.vmware.certificate;

/**
 * @author aengineer
 *
 */
public class VMCAException extends Exception {

   /**
    *
    */
   private static final long serialVersionUID = 4844087723421700671L;
   /**
    *
    */
   int ErrorCode = 0;
   public VMCAException() {
      // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    */
   public VMCAException(String message) {
      super(message);
      // TODO Auto-generated constructor stub
   }

   /**
    * @return the errorCode
    */
   public int getErrorCode() {
      return ErrorCode;
   }

   /**
    * @param errorCode the errorCode to set
    */
   public void setErrorCode(int errorCode) {
      ErrorCode = errorCode;
   }

   /**
    * @param cause
    */
   public VMCAException(Throwable cause) {
      super(cause);
      // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    * @param cause
    */
   public VMCAException(String message, Throwable cause) {
      super(message, cause);
      // TODO Auto-generated constructor stub
   }

   /**
    * @param message
    * @param cause
    * @param enableSuppression
    * @param writableStackTrace
    */
   public VMCAException(String message, Throwable cause,
         boolean enableSuppression, boolean writableStackTrace) {
      super(message, cause, enableSuppression, writableStackTrace);
      // TODO Auto-generated constructor stub
   }

   public String  getDescriptiveError() throws VMCAException {
      return VMCAAdapter.VMCAGetErrorString(ErrorCode);
   }

   public String getShortError() throws VMCAException {
	   return VMCAAdapter.VMCAGetShortError(ErrorCode)
;   }
   
   

}
