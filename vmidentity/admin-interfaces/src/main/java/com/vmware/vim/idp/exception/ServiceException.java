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
package com.vmware.vim.idp.exception;

/**
 * Base exception indicating errors that are function of the input and the
 * current system state. In most of the cases the client can determine what is
 * wrong and knows how to recover.
 */
public abstract class ServiceException extends Exception {

   private static final long serialVersionUID = 7394465536783144111L;

   public ServiceException(String message, Throwable cause) {
      super(message, cause);
   }

   public ServiceException(String message) {
      super(message);
   }

}
