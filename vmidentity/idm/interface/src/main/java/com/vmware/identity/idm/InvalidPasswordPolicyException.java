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
 * This exception is raised when the supplied password policy is invalid (e.g.
 * negative prohibitedPreviousPasswordsCount, minLength > maxLength in the
 * PasswordFormat, etc.)
 */
public class InvalidPasswordPolicyException extends IDMException {

   private static final long serialVersionUID = 7310831112663801838L;

   public InvalidPasswordPolicyException(String message, Throwable cause) {
      super(message, cause);
   }

   public InvalidPasswordPolicyException(String message) {
      super(message);
   }

   public InvalidPasswordPolicyException(Throwable cause) {
      super(cause);
   }

}
