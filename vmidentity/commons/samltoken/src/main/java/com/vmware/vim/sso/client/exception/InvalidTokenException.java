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
package com.vmware.vim.sso.client.exception;

import com.vmware.vim.sso.client.BundleMessageSource.Key;

/**
 * Thrown if an element of the currently parsed SAML token is not valid. The
 * specific descendants will express the element and/or reason due to which a
 * token is not valid.
 */
public abstract class InvalidTokenException extends SsoException {

   private static final long serialVersionUID = -7205272177400202564L;

   public InvalidTokenException(String message) {
      super(message);
   }

   public InvalidTokenException(String message, Throwable cause) {
      super(message, cause);
   }

   public InvalidTokenException(String message, Key messageKey,
      Throwable cause, Object... messageDetails) {
      super(message, messageKey, cause, messageDetails);
   }
}
