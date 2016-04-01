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
 * This exception is thrown by service methods meaning incorrect provider configuration
 */
public class InvalidProviderException extends ServiceException {

   private static final long serialVersionUID = -5649024920103570953L;
   private final String fieldName;
   private final String fieldValue;

//   public InvalidProviderException(Throwable cause, String fieldName, String fieldValue) {
//      super(getDefaultMessage(fieldName, fieldValue), cause);
//      this.fieldName = fieldName;
//      this.fieldValue = fieldValue;
//   }

   public InvalidProviderException(String fieldName, String fieldValue) {
      super(getDefaultMessage(fieldName, fieldValue));
      this.fieldName = fieldName;
      this.fieldValue = fieldValue;
   }

   private static String getDefaultMessage(String fieldName, String fieldValue) {
      return String.format("Invalid provider configuration: %s = %s", fieldName, fieldValue);
   }

   public String getFieldName() {
      return this.fieldName;
   }

   public String getFieldValue() {
      return this.fieldValue;
   }
}
