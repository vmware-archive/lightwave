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
 * Base exception indicating bad condition of the system which prevents normal
 * completion of the given request. System errors might be caused by hardware
 * failures, software defects (bugs), network problems or when the service is
 * down.
 */
public abstract class SsoRuntimeException extends RuntimeException {

   private final Key _messageKey;
   private final Object[] _messageDetails;

   public SsoRuntimeException(String message) {
      super(message);
      _messageKey = null;
      _messageDetails = null;
   }

   /**
    * Creates an exception
    *
    * @param message, debug message in English, suited for logs optional
    * @param localizedMessage, localized message, suited for end-users, optional
    * @param cause, optional
    */
   public SsoRuntimeException(String message, Key messageKey, Throwable cause, Object... messageDetails) {
      super(message, cause);
      _messageKey = messageKey;
      _messageDetails = messageDetails;
   }

   public SsoRuntimeException(String message, Throwable cause) {
      this(message, null, cause);
   }

   // we cannot override getLocalizedMessage to throw unsuported
   // operation, because it is used by some JDK facilities

   public Key getMessageKey() {
      return _messageKey;
   }

   public Object[] getMessageDetails() {
      return _messageDetails;
   }

   private static final long serialVersionUID = -5890144949620014726L;

}
