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
using Microsoft.ManagementConsole;
using VMDir.Common.DTO;
using VMDirSnapIn.Services;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VMIdentity.CommonUtils;
using VMDirSnapIn.UI;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.ScopeNodes.ListViews
{
    public class DirectoryNodeDetailsView : MmcListView
    {
        private List<KeyValuePair<string, string>> _kvData;
        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = "Attribute";
            this.Columns[0].SetWidth(300);
            this.Columns.Add(new MmcListViewColumn("Value", 300));
            this.Mode = MmcListViewMode.Report;
            //this.SelectionData.EnabledStandardVerbs = StandardVerbs.Properties | StandardVerbs.Refresh;
            (this.ScopeNode as DirectoryNode).ListView = this;
            _kvData = new List<KeyValuePair<string, string>>();
            Refresh();
        }

        public void Refresh()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                var node = this.ScopeNode as DirectoryNode;
                var _properties = new Dictionary<string, VMDirBagItem>();
                VMDir.Common.VMDirUtilities.Utilities.GetItemProperties(node._name, node.ServerDTO, _properties);
                MiscUtilsService.ConvertToKVData(_properties, _kvData);
                FillNodes();
            });
        }

        void FillNodes()
        {
            this.ResultNodes.Clear();

            var node=this.ScopeNode as DirectoryNode;
            var resultNode = new ResultNode { DisplayName = "dn" };
            resultNode.SubItemDisplayNames.Add(node._name);
            resultNode.ImageIndex = (int)VMDirImageIndex.Object;
            this.ResultNodes.Add(resultNode);

            foreach (var entry in _kvData)
            {
                resultNode = new ResultNode { DisplayName = entry.Key };
                resultNode.SubItemDisplayNames.Add(entry.Value);
                resultNode.ImageIndex = (int)VMDirImageIndex.Object;
                this.ResultNodes.Add(resultNode);
            }
        }
    }
}
