/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Xml.Serialization;
using VMDir.Common.VMDirUtilities;


namespace VMDir.Common.DTO
{
    public class VMDirServerDTO
    {
        public string Server { get; set; }

        [XmlIgnore]
        public string BaseDN { get; set; }

        [XmlIgnore]
        public string BindDN { get; set; }

        [XmlIgnore]
        public string Password { get; set; }

        public string GUID { get; set; }

        [XmlIgnore]
        public LdapConnectionService Connection { get; set; }

        public  static VMDirServerDTO CreateInstance ()
        {
            var dto = new VMDirServerDTO { GUID = Guid.NewGuid ().ToString () };
            return dto;
        }

        public static string DN2CN (string dn)
        {
            if (string.IsNullOrEmpty (dn))
                return "";

            if (dn.StartsWith ("dc"))
                return DN2DomainName (dn);
            else
                return dn.Split (',') [0].Split ('=') [1];
        }

        private static string DN2DomainName (string dn)
        {
            return dn.Replace (",", ".").Replace ("dc=", "");
        }
    }

}

