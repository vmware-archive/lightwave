/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

import java.security.cert.X509Certificate;

import com.vmware.vim.sso.admin.exception.SystemException;

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
