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
package com.vmware.identity.saml;

/**
 * Indicates that the requested token lifetime is not supported by SAML Authority because
 * it is for the past of future.
 */
public final class UnsupportedTokenLifetimeException extends RuntimeException {

   private static final long serialVersionUID = 2350593462623486384L;

   public UnsupportedTokenLifetimeException(String message) {
      super(message);
   }

   public UnsupportedTokenLifetimeException(Throwable cause) {
      super(cause);
   }

   public UnsupportedTokenLifetimeException(String message, Throwable cause) {
      super(message, cause);
   }

}
