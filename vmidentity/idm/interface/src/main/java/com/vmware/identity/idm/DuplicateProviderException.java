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
 * This exception is thrown when a provider name or domain already exists.
 */
public class DuplicateProviderException extends IDMException {

   private static final long serialVersionUID = 2390676191698981174L;
   private final String providerName;
   private final String providerAlias;

   public DuplicateProviderException(String message) {
      this(message, (String) null, (String) null);
   }

   public DuplicateProviderException(String providerName, String providerAlias) {
      super(getDefaultMessage(providerName, providerAlias));
      this.providerName = providerName;
      this.providerAlias = providerAlias;
   }

   public DuplicateProviderException(String message, String providerName,
         String providerAlias) {
      super(message);
      this.providerName = providerName;
      this.providerAlias = providerAlias;
   }

   public DuplicateProviderException(Throwable ex) {
      this(null, null, ex);
   }

   public DuplicateProviderException(String providerName, String providerAlias,
         Throwable t) {
      super(t);
      this.providerName = providerName;
      this.providerAlias = providerAlias;
   }

   public DuplicateProviderException(String message, Throwable ex) {
      this(message, null, null, ex);
   }

   public DuplicateProviderException(String message, String providerName,
         String providerAlias, Throwable ex) {
      super(message, ex);
      this.providerName = providerName;
      this.providerAlias = providerAlias;
   }

   public String getProviderName() {
      return providerName;
   }

   public String getProviderAlias() {
      return providerAlias;
   }

   private static String getDefaultMessage(String name, String alias) {
      return String.format("Duplicated provider -- name: [%s], alias: [%s]",
            name, alias);
   }
}
