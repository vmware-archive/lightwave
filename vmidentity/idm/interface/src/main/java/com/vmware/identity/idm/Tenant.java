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
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/7/11
 * Time: 5:51 PM
 * To change this template use File | Settings | File Templates.
 */
public class Tenant implements Serializable
{
    /**
     * Port where we will run our private RMI registry
     */
    public static final int RMI_PORT = 12721;

	private static final long serialVersionUID = 2160924228504203312L;

	private final String _name;
    public final String _longName;
    public String _guid;
    public String _issuerName;
    public String _tenantKey;

    /**
     *
     *
     * @param name  name is case-insensitive.
     */
    public Tenant(String name)
    {
        this( name, null );
    }

    /**
    *
    *
    * @param name  name is case-insensitive.
    */
    public Tenant(String name, String longName)
    {
        ValidateUtil.validateNotEmpty(name, "name");
        this._name = name;
        this._longName = longName;
    }

    /**
    *
    *
    * @param name  name is case-insensitive.
     * @throws Exception
    */
    public Tenant(String name, String longName, String tenantKey)
    {
        ValidateUtil.validateNotEmpty(name, "name");
        this._name = name;
        this._longName = longName;
        this._tenantKey = tenantKey;
    }

    public String getName() { return this._name; }


   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + ((_guid == null) ? 0 : _guid.hashCode());
      result = prime * result
         + ((_issuerName == null) ? 0 : _issuerName.hashCode());
      result = prime * result
         + ((_longName == null) ? 0 : _longName.hashCode());
      result = prime * result + ((_name == null) ? 0 : _name.hashCode());
      result = prime * result + ((_tenantKey == null) ? 0 : _tenantKey.hashCode());
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj)
         return true;
      if (obj == null)
         return false;
      if (getClass() != obj.getClass())
         return false;
      Tenant other = (Tenant) obj;
      if (_guid == null) {
         if (other._guid != null)
            return false;
      } else if (!_guid.equals(other._guid))
         return false;
      if (_issuerName == null) {
         if (other._issuerName != null)
            return false;
      } else if (!_issuerName.equals(other._issuerName))
         return false;
      if (_longName == null) {
         if (other._longName != null)
            return false;
      } else if (!_longName.equals(other._longName))
         return false;
      if (_name == null) {
         if (other._name != null)
            return false;
      } else if (!_name.equals(other._name))
      if (_tenantKey == null) {
          if (other._tenantKey != null)
            return false;
      } else if (!_tenantKey.equals(other._tenantKey))
         return false;
      return true;
   }

}


