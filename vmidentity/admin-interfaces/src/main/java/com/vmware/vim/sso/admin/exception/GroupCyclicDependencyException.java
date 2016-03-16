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
package com.vmware.vim.sso.admin.exception;

import com.vmware.vim.sso.admin.PrincipalManagement;

/**
 * This exception is thrown by {@link PrincipalManagement#addSubGroup} indicates
 * an attempt to create a cyclic group dependency
 */
public class GroupCyclicDependencyException extends ServiceException {

   private static final long serialVersionUID = -2000985479801427514L;
   private final String groupBeingAdded;
   private final String existingGroup;

   public GroupCyclicDependencyException(String groupBeingAdded,
      String existingGroup) {
      super(getDefaultMessage(groupBeingAdded, existingGroup));
      assert (null != groupBeingAdded);
      assert (null != existingGroup);
      this.groupBeingAdded = groupBeingAdded;
      this.existingGroup = existingGroup;
   }

   public static String getDefaultMessage(String groupBeingAdded,
      String existingGroup) {
      return "Cannot add group '" + groupBeingAdded
         + "' as a member of local group '" + existingGroup
         + "' as it is already a parent of that group.";
   }

   public GroupCyclicDependencyException(Throwable cause,
      String groupBeingAdded, String existingGroup) {
      super(getDefaultMessage(groupBeingAdded, existingGroup));
      assert (null != groupBeingAdded);
      assert (null != existingGroup);
      this.groupBeingAdded = groupBeingAdded;
      this.existingGroup = existingGroup;
   }

   public String getGroupBeingAdded() {
      return groupBeingAdded;
   }

   public String getExistingGroup() {
      return existingGroup;
   }
}
