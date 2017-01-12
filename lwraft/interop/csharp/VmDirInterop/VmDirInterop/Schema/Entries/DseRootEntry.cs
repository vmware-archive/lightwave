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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VmDirInterop.Schema.Constants;
using VmDirInterop.Schema.Exceptions;

namespace VmDirInterop.Schema.Entries
{
    public class DseRootEntry
    {
        public String domain { get; private set; }
        public String serverDn { get; private set; }

        public DseRootEntry(ILdapEntry entry)
        {
            LdapValue val = entry.getAttributeValues(AttributeConstants.NAMING_CONTEXT)[0];
            domain = val.StringValue;

            val = entry.getAttributeValues(AttributeConstants.SERVER_NAME)[0];
            serverDn = val.StringValue;
        }

        public DseRootEntry(String domain, String serverDn)
        {
            this.domain = domain;
            this.serverDn = serverDn;
        }
    }
}
