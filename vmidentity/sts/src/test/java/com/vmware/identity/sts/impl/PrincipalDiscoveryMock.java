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
package com.vmware.identity.sts.impl;

import java.security.cert.X509Certificate;

import junit.framework.Assert;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.SystemException;

/**
 * Insert your comment for PrincipalDiscoveryMock here
 */
final class PrincipalDiscoveryMock implements PrincipalDiscovery {

   private static final String DESCRIPTION = "description";

   private final PrincipalId solutionId;
   private final X509Certificate solutionCert;
   private boolean findUserByNameThrowsException;

   PrincipalDiscoveryMock(PrincipalId solutionId, X509Certificate solutionCert) {
      assert solutionId != null;
      assert solutionCert != null;
      this.solutionId = solutionId;
      this.solutionCert = solutionCert;
   }

   PrincipalDiscoveryMock(PrincipalId solutionId, X509Certificate solutionCert, boolean findUserByNameThrowsException) {
      this(solutionId, solutionCert);
      this.findUserByNameThrowsException = findUserByNameThrowsException;
   }

   @Override
   public SolutionUser findSolutionUser(String subjectDN)
      throws SystemException {
      return null;
   }

   @Override
   public SolutionUser findSolutionUserByName(String name)
      throws SystemException {
      Assert.assertEquals(solutionId.getName(), name);
      if (findUserByNameThrowsException) {
         throw new SystemException("Exception for testing purposes.");
      }
      SolutionDetail detail = new SolutionDetail(solutionCert, DESCRIPTION);
      return new SolutionUser(solutionId, detail, false);
   }

   @Override
   public boolean isMemberOfSystemGroup(PrincipalId principalId, String groupName)
      throws SystemException {
      return false;
   }
}
