/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
using Microsoft.ManagementConsole;
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.ListViews;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsStoreEntriesNode : ScopeNode
    {
        public VecsStoreEntriesListView ListView { get; set; }
        public VMCertStoreServerDTO ServerDTO { get; protected set; }
        public string StoreName { get; protected set; }

        public VecsStoreEntriesNode(VMCertStoreServerDTO dto, string storeName)
        {
            StoreName = storeName;
            ServerDTO = dto;
            this.EnabledStandardVerbs = StandardVerbs.Refresh;
        }
    }
}
