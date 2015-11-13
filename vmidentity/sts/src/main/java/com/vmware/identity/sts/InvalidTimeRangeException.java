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
 * This exception is thrown when time range in the request does not match, for
 * example it was ended before the time when the request is made or it starts
 * after the request will eventually finish.
 */
public class InvalidTimeRangeException extends RequestFailedException {

   private static final long serialVersionUID = -3968725971458155376L;

   public InvalidTimeRangeException(String message, Throwable cause) {
      super(message, cause);
   }

   public InvalidTimeRangeException(String message) {
      super(message);
   }

   public InvalidTimeRangeException(Throwable cause) {
      super(cause);
   }

}
