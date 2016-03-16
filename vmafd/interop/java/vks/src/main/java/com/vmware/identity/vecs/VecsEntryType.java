/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.vecs;

/**
 * Represents type of a VECS entry.
 */
public enum VecsEntryType {
   /**
    * PrivateKey entry, which can store a PrivateKey and its corresponding
    * certificate chain.
    */
   CERT_ENTRY_TYPE_PRIVATE_KEY(1),

   /**
    * SecretKey entry, which can store a SecterKey only.
    */
   CERT_ENTRY_TYPE_SECRET_KEY(2),

   /**
    * TrustesCert entry, which can store a trusted certificate only.
    */
   CERT_ENTRY_TYPE_TRUSTED_CERT(3),

   /**
    * CRL entry, which can store a CRL only.
    */
   CERT_ENTRY_TYPE_CRL(4);

   private final int _entryType;

   private VecsEntryType(int entryType) {
      _entryType = entryType;
   }

   /**
    * Gets numeric representation of VecsEntryType.
    * 
    * @return Numeric representation of VecsEntryType.
    */
   public int getValue() {
      return _entryType;
   }

   /**
    * Gets VecsEntryType object from the numeric representation.
    * 
    * @param type
    *           Numeric representation of the VecsEntryType.
    * @return VecsEntryType object.
    */
   public static VecsEntryType getEntryType(int type) {
      switch (type) {
      case 1:
         return VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY;
      case 2:
         return VecsEntryType.CERT_ENTRY_TYPE_SECRET_KEY;
      case 3:
         return VecsEntryType.CERT_ENTRY_TYPE_TRUSTED_CERT;
      case 4:
         return VecsEntryType.CERT_ENTRY_TYPE_CRL;
      default:
         throw new IllegalArgumentException(String.format(
               "Unidentified entity type [%d]", type));
      }
   }
}
