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
 * Base exception thrown at IDM layer to indicate <i>domain</i> errors.
 * <p>
 * By <i>domain</i> errors we mean errors that are function of the input and
 * current system state. Depending on the input parameters and exception type
 * client should be able to determine what's wrong.
 */
public class IDMException extends Exception {

   private static final long serialVersionUID = 4933827572526844178L;

   public IDMException(String message) {
      super(message);
   }

   public IDMException(Throwable ex) {
      super(ex);
   }

   public IDMException(String message, Throwable ex) {
       super(message, ex);
   }
}
