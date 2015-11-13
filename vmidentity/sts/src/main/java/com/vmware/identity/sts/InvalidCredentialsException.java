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

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Indicates that authentication fails because credentials are invalid.
 *
 * Maintains a boolean flag that allows details, e.g. message and cause, to be
 * revealed to the clients.
 */
public final class InvalidCredentialsException extends AuthenticationFailedException {

   private static final long serialVersionUID = -4076992309036996473L;
   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(InvalidCredentialsException.class);
   private static final String DEFAULT_MESSAGE = "Invalid credentials";

   private final boolean canRevealCause;

   public InvalidCredentialsException(String message, Throwable cause) {
      this(message, cause, false);
   }

   public InvalidCredentialsException(String message, Throwable cause,
      boolean canRevealCause) {
      super(message, cause);
      this.canRevealCause = canRevealCause;
   }

   public InvalidCredentialsException(String message) {
      this(message, false);
   }

   public InvalidCredentialsException(String message, boolean canRevealCause) {
      super(message);
      this.canRevealCause = canRevealCause;
   }

   public InvalidCredentialsException buildPublic() {
      log.debug("About to censor authentication failure : ", super.getMessage());
      return canRevealCause ? this : new InvalidCredentialsException(
         DEFAULT_MESSAGE);
   }
}
