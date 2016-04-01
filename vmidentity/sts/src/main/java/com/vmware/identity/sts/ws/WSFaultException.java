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

package com.vmware.identity.sts.ws;

import org.apache.commons.lang.Validate;

import com.vmware.identity.sts.ws.SOAPFaultHandler.FaultKey;

/**
 * This class indicates a condition that should be propagated to the client
 * using a web service fault
 */
public class WSFaultException extends RuntimeException {

   private static final long serialVersionUID = 6681860645758777020L;
   private final FaultKey key;

   public WSFaultException(FaultKey key, String message) {
      super(message);
      Validate.notNull(key);
      Validate.notEmpty(message);
      this.key = key;
   }

   public WSFaultException(FaultKey key, Throwable cause) {
      super(cause);
      Validate.notNull(key);
      Validate.notNull(cause);
      this.key = key;
   }

   public FaultKey getFaultKey() {
      return key;
   }
}