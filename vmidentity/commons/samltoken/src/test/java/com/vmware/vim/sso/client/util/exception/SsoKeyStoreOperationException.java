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
package com.vmware.vim.sso.client.util.exception;

import com.vmware.vim.sso.client.exception.SsoException;

/**
 * Signals that a keystore operation has failed (i.e. certificate or private key
 * extraction has failed, or the keystore cannot be opened etc.)
 */
public class SsoKeyStoreOperationException extends SsoException {

   private static final long serialVersionUID = 4328081480901343493L;


   public SsoKeyStoreOperationException(String message, Throwable cause) {
      super(message, cause);
   }
}
