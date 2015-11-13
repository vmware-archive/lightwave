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
package com.vmware.identity.sts.ws;

import com.vmware.identity.sts.NoSuchIdPException;

/**
 * This class is an utility for extraction of tenant name.
 */
public class TenantExtractor {

   private static final String forwardSlash = "/";
   private static final String DEFAULT_TENANT_NAME = "vsphere.local";

   /**
    * @param pathInfo
    *           always starts with '/'.
    * @return tenant name in given pathInfo. Tenant name is located between the
    *         first two '/' It there is no second '/' the name is just without
    *         the first '/'. If the path contains only '/' then null is
    *         returned, meaning there is no tenant.
    * @throws NoSuchIdPException
    *            when tenant cannot be found or extracted from request URL
    */
   public static String extractTenantName(String pathInfo) {
      if (pathInfo == null || pathInfo.equals(forwardSlash)) {
         return DEFAULT_TENANT_NAME;
         // throw new NoSuchIdPException("There is no tenant in the request");
      }

      // if path contains second forward slash
      int slashIndex = pathInfo.indexOf(forwardSlash, 1);
      return (slashIndex != -1) ? pathInfo.substring(1, slashIndex) : pathInfo
         .substring(1);
   }

}
