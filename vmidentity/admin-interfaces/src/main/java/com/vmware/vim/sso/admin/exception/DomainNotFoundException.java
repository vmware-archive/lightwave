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
 * Thrown to indicate that there's no Domain associated with a given name.
 */
public class DomainNotFoundException extends ServiceException {
   private static final long serialVersionUID = 3834601685482158291L;
   /**
    * Name of the domain which could not be found;
    */
   private final String name;

   public DomainNotFoundException(Throwable cause, String name) {
      super(getDefaultMessage(name), cause);
      assert (null != name && !name.trim().isEmpty());
      this.name = name;
   }

   public DomainNotFoundException(String name) {
      super(getDefaultMessage(name));
      assert (null != name && !name.trim().isEmpty());
      this.name = name;
   }

   private static String getDefaultMessage(String name) {
      return String.format("Domain '%s' does not exist.", name);
   }

   public String getName() {
      return name;
   }
}
