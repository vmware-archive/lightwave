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

package com.vmware.vim.sso.admin;

import java.io.Serializable;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Immutable data type describing a host machine by its name
 * and whether it is a domain controller or not.
 */
public class VmHost implements Serializable {

   private static final long serialVersionUID = -1973202329176685989L;

   private String _hostName;
   private boolean _isDomainController;

   /**
    * Protected no-arg contructor for the Serializable interface.
    */
   protected VmHost() {
      _hostName = "";
      _isDomainController = false;
   }

   /**
    * Creates a new VmHost with its domain controller flag set to
    * <tt>false</tt>
    *
    * @param hostName
    *    cannot be empty or null
    */
   public VmHost(String hostName) {
      this(hostName, false);
   }

   /**
    * Creates a new VmHost
    *
    * @param hostName
    *    cannot be empty or null
    * @param isDomainController
    */
   public VmHost(String hostName, boolean isDomainController) {
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
