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
 * Thrown to indicate that an IDP with the given name or alias already exists.
 */
public final class DuplicateIdpNameException extends ServiceException {

   private static final long serialVersionUID = -6604096374387740369L;

   private final String _idpName;

   public DuplicateIdpNameException(Throwable cause, String idpName) {
      super(getDefaultMessage(idpName), cause);

      assert idpName != null && !idpName.trim().isEmpty();
      _idpName = idpName;
   }

   public String getIdpName() {
      return _idpName;
   }

   private static String getDefaultMessage(String idpName) {
      return String.format("IDP with given name '%s' already exists.", idpName);
   }

}
