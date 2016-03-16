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
 * Thrown to indicate some unexpected or internal failure has occurred in the
 * SSO Server.
 * <p>
 * This exception usually indicates a bug in the server, corrupt installation
 * file(s), corrupt database or incompatibility of the underlying platform (JVM,
 * OS, etc.).
 */
@SuppressWarnings("deprecation")
public class InternalError extends SystemException {

   private static final long serialVersionUID = -5366022288056937572L;

   public InternalError(String message) {
      super(message);
   }

   public InternalError(String message, Throwable cause) {
      super(message, cause);
   }
}
