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

namespace VMDirSnapIn.ScopeNodes.ListViews
{
    class DirectoryNodeListView : MmcListView
    {
        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = "Name";
            this.Columns[0].SetWidth(120);

            this.Columns.Add(new MmcListViewColumn("Class", 100));
            this.Columns.Add(new MmcListViewColumn("Distinguished Name", 250));
            this.Mode = MmcListViewMode.Report;

            OnRefresh(status);
        }
        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            Refresh();
        }
        void Refresh()
        {
            FillNodes();
        }
        protected override void OnSelectionChanged(SyncStatus status)
        {
            if (this.SelectedNodes.Count == 0)
            {
                this.SelectionData.Clear();
            }
            else
            {
                this.SelectionData.Update(GetDetails(), this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                this.SelectionData.EnabledStandardVerbs = StandardVerbs.Refresh | StandardVerbs.Delete;
            }
        }

        string GetDetails()
        {
            return "";
        }
        protected override void OnSelectionAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
        }

        protected override void OnDelete(SyncStatus status)
        {
            base.OnDelete(status);

            MiscUtilsService.CheckedExec(delegate()
            {
                var serverDTO = (this.ScopeNode as DirectoryNode).ServerDTO;
                int count = this.SelectedNodes.Count;
                if (MiscUtilsService.Confirm(string.Format(CommonConstants.CONFIRM_SELECTED_DELETE, "objects", count)))
                {
                    foreach (ResultNode nodeToDelete in this.SelectedNodes)
                    {
                        string dn = nodeToDelete.Tag as string;
                        serverDTO.Connection.DeleteObject(dn);
                    }
                }
            });
        }

        void FillNodes()
        {
            this.ResultNodes.Clear();
            ILdapMessage ldMsg = null;
            try
            {
                var node = this.ScopeNode as DirectoryNode;
                var conn = node.ServerDTO.Connection;
                List<ILdapEntry> entries = conn.SearchAndGetEntries(node.Name, LdapScope.SCOPE_BASE, "(objectClass=*)", null, 0, ref ldMsg);
                foreach (LdapEntry entry in entries)
                {
                    var resultNode = new ResultNode { DisplayName = VMDirServerDTO.DN2CN(entry.getDN()) };
                    resultNode.ImageIndex = 2;
                    resultNode.SubItemDisplayNames.Add("");
                    resultNode.SubItemDisplayNames.Add(entry.getDN());
                    resultNode.Tag = entry.getDN();
                    this.ResultNodes.Add(resultNode);
                }
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
            }
            finally
            {
                if (ldMsg != null)
                    (ldMsg as LdapMessage).FreeMessage();
            }
        }
    }
}
