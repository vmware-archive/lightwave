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
 * User: snambakam
 * Date: 12/25/11
 * Time: 1:15 AM
 */
public class Attribute implements Serializable
{
   private static final long serialVersionUID = 8146908415068307317L;

   private String name;
   private String friendlyName;
   private String nameFormat;

   /**
    * Constructs Attribute object.
    *
    * @param name
    *           attribute name; {@code not-null} required
    */
   public Attribute(String name)
   {
      this(name, null, null);
   }

   /**
    * Constructs Attribute object.
    *
    * @param name
    *           attribute name; {@code not-null} required
    * @param nameFormat         {@code null} optional
    * @param friendlyName       {@code null} optional
    */
   public Attribute(String name, String nameFormat, String friendlyName)
   {
      ValidateUtil.validateNotEmpty(name, "Null attribute name");
      this.name = name;
      this.nameFormat = nameFormat;
      this.friendlyName = friendlyName;
   }

   public String getName()
   {
      return name;
   }

   public String getNameFormat()
   {
      return nameFormat;
   }

   public void setNameFormat(String nameFormat)
   {
      this.nameFormat = nameFormat;
   }

   public String getFriendlyName()
   {
      return friendlyName;
   }

   public void setFriendlyName(String friendlyName)
   {
      this.friendlyName = friendlyName;
   }

   @Override
   public String toString() {
      return "Attribute [name=" + name + ", friendlyName=" + friendlyName
            + ", nameFormat=" + nameFormat + "]";
   }

}
