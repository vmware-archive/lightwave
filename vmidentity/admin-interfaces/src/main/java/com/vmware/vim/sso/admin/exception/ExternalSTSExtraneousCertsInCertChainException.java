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
 * This exception is thrown by service methods to indicate Certification
 * configuration error.
 */
public class ExternalSTSExtraneousCertsInCertChainException extends
      ServiceException {

   private static final long serialVersionUID = 6662725965157582229L;
   private final String name;

   /**
    * @param name
    */
   public ExternalSTSExtraneousCertsInCertChainException(String name) {
      super(getDefaultMessage(name));
      assert name != null && !name.trim().isEmpty();
      this.name = name;
   }

   public String getName() {
      return name;
   }

   private static String getDefaultMessage(String name) {
      return String.format("Certpath for [%s] should contain only one chain.",
            name);
   }
}
