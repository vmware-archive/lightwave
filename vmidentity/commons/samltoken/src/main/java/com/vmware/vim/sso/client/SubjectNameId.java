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
package com.vmware.vim.sso.client;

/**
 * Instances of this object will contain the subject value and the format in
 * which it is. It represents the NameId section in the Subject of each token.
 *
 */
public final class SubjectNameId {

    // SAML 2.0:
    //if no Format value is provided, then the value
    // urn:oasis:names:tc:SAML:1.0:nameid-format:unspecified
    private static final String _defaultFormat = "urn:oasis:names:tc:SAML:1.0:nameid-format:unspecified";

   private final String value;
   private final String format;

   /**
    * @param value
    *           is the token subject name id. Cannot be null.
    * @param format
    *           is the corresponding format of subject's name id. Cannot be
    *           null.
    */
   public SubjectNameId(String value, String format) {
      assert value != null;

      if (( format == null ) || (format.isEmpty()) )
      {
          format = _defaultFormat;
      }

      this.value = value;
      this.format = format;
   }

   /**
    * Return the token subject. Cannot be null.
    */
   public String getValue() {
      return value;
   }

   /**
    * Returns the format of the token subject. Cannot be null.
    */
   public String getFormat() {
      return format;
   }

   @Override
   public String toString() {
      return String.format("SubjectNameId [value=%s, format=%s]", value, format);
   }

}
