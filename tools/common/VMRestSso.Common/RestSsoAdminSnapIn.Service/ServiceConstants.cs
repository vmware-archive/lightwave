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

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public static class ServiceConstants
    {
        // Http headers
        public static string ContentType = "application/x-www-form-urlencoded; charset=utf-8";
        public static string JsonContentType = "application/json";
        public static string PlainTextContentType = "text/plain";
        public static string XmlContentType = "application/xml";
        public static string UserAgent = "VMware-client/5.1.0";

        // Certificate Types
        public static string LdapTrustedCertificate = "LDAP_TRUSTED_CERT";
        public static string StsTrustedCertificate = "STS_TRUST_CERT";
    }
}
