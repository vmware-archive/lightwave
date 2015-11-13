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
package com.vmware.identity.sts.util;

import com.vmware.identity.idm.PrincipalId;

/**
 * TODO [849937] deprecated
 */
public final class PrincipalIdConvertor {

   /**
    * Convert principal Id from client SSO representation.
    *
    * @param subject
    *           required
    * @return converted principalId
    */
   public static PrincipalId toIdmPrincipalId(
      com.vmware.vim.sso.PrincipalId subject) {
      assert subject != null;
      return new PrincipalId(subject.getName(), subject.getDomain());
   }

   /**
    * Convert principal Id from IDM representation.
    *
    * @param subject
    *           required
    * @return converted principalId
    */
   public static com.vmware.vim.sso.PrincipalId fromIdmPrincipalId(
      PrincipalId subject) {
      assert subject != null;
      return new com.vmware.vim.sso.PrincipalId(subject.getName(),
         subject.getDomain());
   }
}
