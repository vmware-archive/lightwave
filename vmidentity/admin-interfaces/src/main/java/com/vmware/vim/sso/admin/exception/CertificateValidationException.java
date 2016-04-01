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

import java.security.cert.X509Certificate;

/**
 * This exception is thrown when the server's certificate(s) cannot be validated
 * against the local trust store. The exception also contains the failed
 * certificate chain and the thumbprint of the first certificate in the chain.
 */
public class CertificateValidationException extends SystemException {

   private static final long serialVersionUID = -1877402054517000009L;

   private final X509Certificate[] _chain;
   private final String _thumbprint;

   /**
    * @param message
    *           The error message.
    * @param cause
    *           The root validation failure exception.
    * @param certificateChain
    *           The failed certificate chain.
    * @param thumbprint
    *           The thumbprint of the first certificate in the chain.
    */
   @SuppressWarnings("deprecation")
   public CertificateValidationException(String message, Throwable cause,
      X509Certificate[] certificateChain, String thumbprint) {
      super(message, cause);
      _chain = certificateChain;
      _thumbprint = thumbprint;
   }

   /**
    * @return the failed certificate chain.
    */
   public X509Certificate[] getCertificateChain() {
      return _chain;
   }

   /**
    * @return the thumbprint of the first certificate in the chain.
    */
   public String getThumbprint() {
      return _thumbprint;
   }
}
