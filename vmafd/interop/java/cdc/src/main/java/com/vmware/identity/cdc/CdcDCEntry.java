/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved
 * ********************************************************************
 */
package com.vmware.identity.cdc;

/**
 * Represents a public information of CDC DC entry.
 */
public class CdcDCEntry {
   /**
    * Entry type.
    */
   public final CdcAddressType dcAddressType;
   /**
    * DC Name of the entry.
    */
   public final String dcName;
   /**
    * DC Address of the entry.
    */
   public final String dcAddress;
   /**
    * Site Name of the entry.
    */
   public final String dcSiteName;

   CdcDCEntry(CdcAddressType addressTypeArg, String dcNameArg,
                      String dcAddressArg, String dcSiteNameArg) {
      dcAddressType = addressTypeArg;
      dcName = dcNameArg;
      dcAddress = dcAddressArg;
      dcSiteName = dcSiteNameArg;
   }
}
