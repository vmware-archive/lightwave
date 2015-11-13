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
package com.vmware.identity.saml;

import org.apache.commons.lang.Validate;

/**
 * Represents an principal attribute definition, without the value.
 */
public class PrincipalAttributeDefinition {

   private final String name;
   private final String friendlyName;
   private final String nameFormat;

   /**
    * @param name
    *           required
    * @param nameFormat
    *           required
    * @param friendlyName
    *           mandatory
    */
   public PrincipalAttributeDefinition(String name, String nameFormat,
      String friendlyName) {
      Validate.notNull(name);
      Validate.notNull(nameFormat);

      this.name = name;
      this.nameFormat = nameFormat;
      this.friendlyName = friendlyName;
   }

   /**
    * @return name, not null
    */
   public String getName() {
      return name;
   }

   /**
    * @return name format, not null
    */
   public String getNameFormat() {
      return nameFormat;
   }

   /**
    * @return friendly name, mandatory
    */
   public String getFriendlyName() {
      return friendlyName;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result
         + ((friendlyName == null) ? 0 : friendlyName.hashCode());
      result = prime * result + name.hashCode();
      result = prime * result + nameFormat.hashCode();
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || this.getClass() != obj.getClass()) {
         return false;
      }

      PrincipalAttributeDefinition other = (PrincipalAttributeDefinition) obj;
      boolean equalFriendlyName = (friendlyName == null && other.friendlyName == null)
         || (friendlyName != null && other.friendlyName != null && friendlyName
            .equals(other.friendlyName));
      return equalFriendlyName && name.equals(other.name)
         && nameFormat.equals(other.nameFormat);
   }

   @Override
   public String toString() {
      final StringBuilder sb = new StringBuilder();
      sb.append("PrincipalAttributeDefinition [name=").append(name);
      sb.append(", format=").append(nameFormat);
      sb.append(", friendly name=").append(friendlyName).append("]");
      return sb.toString();
   }

}
