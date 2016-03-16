/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved
 * ********************************************************************
 */
package com.vmware.identity.cdc;

/**
 * Represents type of a VECS entry.
 */
public enum CdcAddressType {
   /**
    * ADDRESS TYPE IS NO_ADDRESS
    */
   CDC_ADDRESS_TYPE_NO_ADDRESS(0),
   /**
    * ADDRESS TYPE IS INET
    */
   CDC_ADDRESS_TYPE_INET(1),

   /**
    * ADDRESS TYPE IS NETBIOS
    */
   CDC_ADDRESS_TYPE_NETBIOS(2);

   private final int _addressType;

   private CdcAddressType(int addressType) {
      _addressType = addressType;
   }

   /**
    * Gets numeric representation of CdcAddressType.
    *
    * @return Numeric representation of CdcAddressType.
    */
   public int getValue() {
      return _addressType;
   }

   /**
    * Gets CdcAddressType object from the numeric representation.
    *
    * @param type
    *           Numeric representation of the CdcAddressType.
    * @return CdcAddressType object.
    */
   public static CdcAddressType getEntryType(int type) {
      switch (type) {
	case 0:
         return CdcAddressType.CDC_ADDRESS_TYPE_NO_ADDRESS;
      case 1:
         return CdcAddressType.CDC_ADDRESS_TYPE_INET;
	case 2:
         return CdcAddressType.CDC_ADDRESS_TYPE_NETBIOS;
	default:
         throw new IllegalArgumentException(String.format(
               "Unidentified entity type [%d]", type));
      }
   }
}
