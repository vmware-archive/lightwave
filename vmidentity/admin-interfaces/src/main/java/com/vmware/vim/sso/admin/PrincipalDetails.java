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

import java.util.Arrays;

/**
 * Abstract class for principal ( user and group) attributes. These objects are
 * mutually comparable with equals method. This class is immutable.
 */
public abstract class PrincipalDetails {

   /**
    * A descriptive name of the user (e.g. John Doe)
    */
   private final String _description;

   /**
    * Create principal details
    *
    * @param description
    *           the description to set; accepts <code>null</code> value
    */
   protected PrincipalDetails(String description) {
      _description = description;
   }

   /**
    * Return the description
    *
    * @return the description or <code>null</code> value
    */
   public String getDescription() {
      return _description;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public final boolean equals(Object obj) {
      if (obj == this) {
         return true;
      }

      if (obj == null || getClass() != obj.getClass()) {
         return false;
      }

      PrincipalDetails other = (PrincipalDetails) obj;
      return Arrays.equals(getIdentityState(), other.getIdentityState());
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public final int hashCode() {
      return Arrays.hashCode(getIdentityState());
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public String toString() {

      StringBuilder sb = new StringBuilder();
      for (Object obj : getIdentityState()) {
         sb.append("'" + obj + "',");
      }

      if (sb.length() > 0) {

         // delete the last comma
         sb.deleteCharAt(sb.length() - 1);
      }

      return sb.toString();
   }

   /**
    * Template method defining the object state on which basis is determined
    * when two objects are equal. It is used by equals and hashCode method
    * implementations.
    *
    * @return object identity state
    */
   protected abstract Object[] getIdentityState();

}
