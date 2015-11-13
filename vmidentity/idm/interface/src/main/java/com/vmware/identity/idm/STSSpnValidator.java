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

import java.util.StringTokenizer;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

public class STSSpnValidator
{

   private static final String VALID_SPN_SEPARATORS = "/";
   private static final String SERVICE_PRINCIPAL_NAME = "STS";

   public static boolean validate(String spn)
   {
      // validate spn format, and service principal name and domain name are
      // valid strings.
      Validate.notNull(spn, "Service Princiapal Name cannot be null");
      Validate.notEmpty(spn, "Service Principal Name cannot be empty");
      StringTokenizer st = new StringTokenizer(spn, VALID_SPN_SEPARATORS);
      if (st.countTokens() != 2)
      {
         logAndThrow("Invalid service principal name format: " + spn);
      } else
      {
         String servicePrincipalName = st.nextToken();
         String domainName = st.nextToken();
         if (null == servicePrincipalName)
         {
            logAndThrow(String
                  .format("Service Name must be specfied before [%s]"
                        + VALID_SPN_SEPARATORS));
         }
         if (!servicePrincipalName.equalsIgnoreCase(SERVICE_PRINCIPAL_NAME))
         {
            logAndThrow("Service name must be STS (case insensitive)");
         }
         if (null == domainName || domainName.trim().isEmpty())
         {
            logAndThrow(String.format("Domain Name must be specfied after [%s]"
                  + VALID_SPN_SEPARATORS));
         }
      }
      return true;
   }

   private static void logAndThrow(String msg) {
      DiagnosticsLoggerFactory.getLogger(STSSpnValidator.class).error(msg);
      throw new IllegalArgumentException(msg);
   }
}
