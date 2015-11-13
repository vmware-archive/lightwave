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

import java.util.HashSet;
import java.util.Set;

/**
 * Defines and return list of implicit group names for SSO system.
 * @see SSOImplicitGroupNamesEnum
 * @see SSOImplicitGroupNames#getAllNames
 */
public final class SSOImplicitGroupNames
{

   private enum SSOImplicitGroupNamesEnum
   {
      Everyone;
      //add additional implicit group names here
   };

   private static final Set<String> groupNames;
   static
   {
      groupNames = new HashSet<String>();
      for (SSOImplicitGroupNamesEnum item : SSOImplicitGroupNamesEnum.values())
      {
         groupNames.add(item.name());
      }
   }

   private SSOImplicitGroupNames()
   {}


   public static Set<String> getAllNames()
   {
      return groupNames;
   }

   public static String getEveryoneGroupName()
   {
      return SSOImplicitGroupNamesEnum.Everyone.name();
   }
}
