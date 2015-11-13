/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * ********************************************************************
 */
package com.vmware.identity.cdc;

/**
 * Represents type of a VECS entry.
 */
public enum CdcState {
   /**
    * STATE IS UNDEFINED
    */
   CDC_STATE_UNDEFINED(0),
   /**
    * THERE IS NO LIST OF DCs
    */
   CDC_STATE_NO_DC_LIST(1),

   /**
    * SITE AFFINITIZED
    */
   CDC_STATE_SITE_AFFINITIZED(2),
   
   /**
    * OFF SITE
    */
   CDC_STATE_OFF_SITE(3),
   
   /**
    * NO DCs ARE ALIVE
    */
   CDC_STATE_NO_DCS_ALIVE(4),
   
   /**
    * CDC IS DISABLED
    */
   CDC_STATE_DISABLED(5);
   

   private final int _cdcState;

   private CdcState(int cdcState) {
	      _cdcState = cdcState;
   }
   /**
    * Gets numeric representation of CdcState.
    *
    * @return Numeric representation of CdcState.
    */
   public int getValue() {
      return _cdcState;
   }

   /**
    * Gets CdcState object from the numeric representation.
    *
    * @param type
    *           Numeric representation of the CdcState.
    * @return CdcState object.
    */
   public static CdcState getState(int state) {
      switch (state) {
	case 0:
         return CdcState.CDC_STATE_UNDEFINED;
    case 1:
         return CdcState.CDC_STATE_NO_DC_LIST;
	case 2:
         return CdcState.CDC_STATE_SITE_AFFINITIZED;
	case 3:
		 return CdcState.CDC_STATE_OFF_SITE;
	case 4:
		 return CdcState.CDC_STATE_NO_DCS_ALIVE;
	case 5:
		 return CdcState.CDC_STATE_DISABLED;
	default:
         throw new IllegalArgumentException(String.format(
               "Unidentified State [%d]", state));
      }
   }
}
