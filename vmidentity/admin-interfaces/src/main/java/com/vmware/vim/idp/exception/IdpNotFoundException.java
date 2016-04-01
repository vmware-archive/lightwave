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

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Thrown to indicate that there's no IDP associated with a given name.
 */
public final class IdpNotFoundException extends ServiceException {

   private static final long serialVersionUID = 7421750026093775687L;

   private final String _idpName;

   public IdpNotFoundException(Throwable cause, String idpName) {
      super(getDefaultMessage(idpName), cause);

      ValidateUtil.validateNotEmpty(idpName, "IDP name");
      _idpName = idpName;
   }

   public IdpNotFoundException(String idpName) {
      super(getDefaultMessage(idpName));

      ValidateUtil.validateNotEmpty(idpName, "IDP name");
      _idpName = idpName;
   }

   public String getIdpName() {
      return _idpName;
   }

   private static String getDefaultMessage(String idpName) {
      return String.format("Cannot find IDP with name '%s'.", idpName);
   }

}
