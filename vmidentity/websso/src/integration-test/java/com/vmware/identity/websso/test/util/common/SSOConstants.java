/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.test.util.common;

public interface SSOConstants
{
   public static final String UTF8_CHARSET = "UTF-8";
   public static final int BUF_SIZE = 1024;
   public static final int INFLATE_SIZE = 5000;

   public static final String HDR_LOCATION_KEY = "location";

   // For IDP Configuration
   public static final String SCHEME =  "https://";
   public static final String STS_SUFFIX = "/sts/STSService/";
   public static final String METADATA_SUFFIX = "/websso/SAML2/Metadata/"; // + <tenant>;
}
