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
package com.vmware.identity.saml.config;

/**
 * Thrown when general error happened when trying to extract configuration ,
 * e.g. no connectivity, exception when transporting info between the layers
 * etc.
 */
public class SystemConfigurationException extends RuntimeException {

   private static final long serialVersionUID = 994884179229145261L;

   public SystemConfigurationException() {
      super();
   }

   public SystemConfigurationException(String message, Throwable cause) {
      super(message, cause);
   }

   public SystemConfigurationException(String message) {
      super(message);
   }

   public SystemConfigurationException(Throwable cause) {
      super(cause);
   }

}
