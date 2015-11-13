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
 * This exception indicates that the request cannot be accepted due to expired
 * lifetime or if the request is ahead of time.
 */
public class RequestExpiredException extends RuntimeException {

   private static final long serialVersionUID = -2466128434416204671L;

   public RequestExpiredException(String message, Throwable cause) {
      super(message, cause);
   }

   public RequestExpiredException(String message) {
      super(message);
   }

   public RequestExpiredException(Throwable cause) {
      super(cause);
   }

}
