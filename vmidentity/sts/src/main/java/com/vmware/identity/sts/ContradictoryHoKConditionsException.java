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
package com.vmware.identity.sts;

/**
 * This exception is thrown because the given conditions for HoK token are
 * logically opposite and the request cannot be satisfied.
 */
public final class ContradictoryHoKConditionsException extends InvalidSecurityHeaderException {

   private static final long serialVersionUID = -6825288476812207403L;

   public ContradictoryHoKConditionsException(String message) {
      super(message);
   }

   public ContradictoryHoKConditionsException(String message, Throwable cause) {
      super(message, cause);
   }
}
