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
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown to indicate that no SMTP configuration is set on the server
 *
 * @deprecated Since SSO 2.0 this exception has been deprecated.
 */
@Deprecated
public class SmtpConfigNotSetException extends ServiceException {

   private static final long serialVersionUID = 1512854391943177349L;

   /**
    * @deprecated Since SSO 2.0 SmtpConfigNotSetException has been deprecated.
    */
   @Deprecated
   public SmtpConfigNotSetException(String message) {
      super(message);
   }

   /**
    * @deprecated Since SSO 2.0 SmtpConfigNotSetException has been deprecated.
    */
   @Deprecated
   public SmtpConfigNotSetException(String message, Throwable cause) {
      super(message, cause);
   }
}
