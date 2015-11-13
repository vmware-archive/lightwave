/*
 *
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
 *
 */

package com.vmware.identity.idm;

import java.io.Serializable;

/**
 * Immutable data type describing a host machine by its name
 * and whether it is a domain controller or not.
 */
public class VmHostData implements Serializable {

   private static final long serialVersionUID = 7970653794163757016L;

   private String _hostName;
   private boolean _isDomainController;

   /**
    * Protected no-arg contructor for the Serializable interface.
    */
   protected VmHostData() {
      _hostName = "";
      _isDomainController = false;
   }

   /**
    * Creates a new VmHostData with its domain controller flag set to
    * <tt>false</tt>
    *
    * @param hostName
    *    cannot be empty or null
    */
   public VmHostData(String hostName) {
      this(hostName, false);
   }

   /**
    * Creates a new VmHostData
    *
    * @param hostName
    *    cannot be empty or null
    * @param isDomainController
    */
   public VmHostData(String hostName, boolean isDomainController) {
      ValidateUtil.validateNotEmpty(hostName, "hostName");
      _hostName = hostName;
      _isDomainController = isDomainController;
   }

   /**
    * Retrieve the host name
    *
    * @return string containing the host name
    */
   public String getHostName() {
      return _hostName;
   }

   /**
    * Retrieve the flag indicating whether the host is a
    * domain controller or not
    *
    * @return true if it is a domain controller, false otherwise
    */
   public boolean isDomainController() {
      return _isDomainController;
   }

}
