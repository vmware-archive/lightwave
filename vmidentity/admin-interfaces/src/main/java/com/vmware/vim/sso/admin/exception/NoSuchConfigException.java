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
 * This exception is thrown by service methods to indicate that the specified
 * config does not exist.
 */
public class NoSuchConfigException extends ServiceException
{
    private static final long serialVersionUID = -1958364046661516980L;

    private final String missingConfigName;

   /**
    * Create a new exception for a missing configuration
    *
    * @param missingConfigName
    *           Name of the missing configuration
    */
   public NoSuchConfigException(String missingConfigName) {
       super(getDefaultMessage(missingConfigName));
       assert missingConfigName != null && !missingConfigName.trim().isEmpty();
       this.missingConfigName = missingConfigName;
   }

   public String getMissingConfigName() {
       return missingConfigName;
   }

   private static String getDefaultMessage(String missingConfigName) {
       return String.format("Config %s doesn't exist.", missingConfigName);
   }
}
