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
 * Indicates that native AD Identity Source registration cannot be performed for
 * one of the following reasons:
 * <ul>
 * <li>SSO server machine is not joined to AD domain</li>
 * <li>other native AD Identity Source has already been registered</li>
 * <li>other LDAP AD Identity Sources has already been registered</li>
 * <li>the LDAP AD Identity Sources cannot be replaced by a native AD Identity
 * Source without loss of functionality</li>
 * </ul>
 */
public class NativeADRegistrationException extends ServiceException {

   private static final long serialVersionUID = 8894008357764625568L;

   public NativeADRegistrationException(String message) {
      super(message);
   }

   public NativeADRegistrationException(String message, Throwable cause) {
      super(message, cause);
   }
}
