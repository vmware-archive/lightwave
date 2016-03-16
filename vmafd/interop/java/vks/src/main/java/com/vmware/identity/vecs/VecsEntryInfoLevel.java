/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.vecs;

/**
 * Level of entry information to retrieve.
 */
public enum VecsEntryInfoLevel {
   /**
    * ENTRY_INFO_LEVEL_1 signifies that entry's 'entryType', 'date', 'alias'
    * fields are going to be retrieved.
    */
   ENTRY_INFO_LEVEL_1(1),

   /**
    * ENTRY_INFO_LEVEL_2 signifies that entry's 'entryType', 'date', 'alias',
    * 'certificateChain', 'crl' fields are going to be retrieved.
    */
   ENTRY_INFO_LEVEL_2(2);

   private final int _infoLevel;

   private VecsEntryInfoLevel(int infoLevel) {
      _infoLevel = infoLevel;
   }

   public int getValue() {
      return _infoLevel;
   }

   /**
    * Returns VecsEntryInfoLevel object for the numeric representation of the
    * level.
    * 
    * @param level
    *           Numeric representation.
    * @return VecsEntryInfoLevel object.
    */
   public static VecsEntryInfoLevel getInfoLevel(int level) {
      switch (level) {
      case 1:
         return VecsEntryInfoLevel.ENTRY_INFO_LEVEL_1;
      case 2:
         return VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2;
      default:
         throw new IllegalArgumentException(String.format(
               "Unidentified entity type [%d]", level));
      }
   }
}
