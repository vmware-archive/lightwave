/*
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
 */
package com.vmware.identity.sts.idm;

/**
 * Indicates for a system error that prevents normal execution of a given
 * operation. This error has nothing in relation to application type errors like
 * wrong input parameters, bad application state, domain errors. It is more
 * about system-level failures, like broken connectivity to external system,
 * etc. Usually the failed operation should be able to succeed later.
 */
public final class SystemException extends RuntimeException {

   private static final long serialVersionUID = 8741112548456106740L;

   /**
    * 
    */
   public SystemException() {
   }

   /**
    * @param message
    */
   public SystemException(String message) {
      super(message);
   }

   /**
    * @param cause
    */
   public SystemException(Throwable cause) {
      super(cause);
   }

   /**
    * @param message
    * @param cause
    */
   public SystemException(String message, Throwable cause) {
      super(message, cause);
   }

}
