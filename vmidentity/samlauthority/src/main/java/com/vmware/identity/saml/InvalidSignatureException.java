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
package com.vmware.identity.saml;

/**
 * This exception will be thrown when the signature of a parsed saml token
 * cannot be validated against trusted root certificates.
 */
public class InvalidSignatureException extends RuntimeException {

   private static final long serialVersionUID = -7054070101257870639L;

   public InvalidSignatureException(String arg0, Throwable arg1) {
      super(arg0, arg1);
   }

   public InvalidSignatureException(String arg0) {
      super(arg0);
   }

   public InvalidSignatureException(Throwable arg0) {
      super(arg0);
   }

}