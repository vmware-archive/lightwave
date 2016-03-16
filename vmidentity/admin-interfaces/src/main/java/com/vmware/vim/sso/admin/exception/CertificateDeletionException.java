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
 * Thrown to indicate an error during certificate delete operation.
 */
public class CertificateDeletionException extends ServiceException {
   private static final long serialVersionUID = -2189201183557054923L;

   private final X509Certificate certificate;

   public CertificateDeletionException(X509Certificate certificate) {
      super(getDefaultMessage(certificate));
      this.certificate = certificate;
   }

   public X509Certificate getCertificate() {
      return certificate;
   }

   private static String getDefaultMessage(X509Certificate cert) {
      assert cert != null;
      return String.format("Certificate '%s' cannot be deleted.", cert
         .getSubjectDN());
   }
}
