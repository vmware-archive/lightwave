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
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Vecs;
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.ListViews;
using VMCertStoreSnapIn.Utilities;
using VMCertStoreSnapIn.UI;
using System;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsSecretKeysNode : VecsStoreEntriesNode
    {
        const int ACTION_ADD_SECRET_KEY = 1;
        public VecsSecretKeysNode(VMCertStoreServerDTO dto, string storeName)
            : base(dto, storeName)
        {
            ImageIndex = SelectedImageIndex = (int)VMCertStoreImageIndex.SecretKeys;
            DisplayName = "Secret Keys";
            this.Tag = storeName;
            this.EnabledStandardVerbs = StandardVerbs.Refresh;

            InitConsole();
        }

        void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ImageIndex = node.SelectedImageIndex = (int)VMCertStoreImageIndex.SecretKeys;
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
        }

        void InitConsole()
        {
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(
                                        "Add Secret Key",
                                        "Add Secret Key",
                                        -1,
                                        ACTION_ADD_SECRET_KEY));
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = "Secret key details";
            lvd.ViewType = typeof(VecsSecretKeysListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;

            AddViewDescription(this, lvd);
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            MMCActionHelper.CheckedExec(delegate()
            {
                if (ListView != null)
                {
                    ListView.Refresh();
                }
            });
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((int)action.Tag)
            {
                case ACTION_ADD_SECRET_KEY:
                    AddSecretKey();
                    break;
            }
        }

        void AddSecretKey()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var frm = new FormAddSecretKey();
                if (MMCDlgHelper.ShowForm(frm))
                {
                    var dto = frm.SecretKeyDTO;
                    string storeName = (string)Tag;
                    string storePass = "";
                    using (var session = new VecsStoreSession(ServerDTO.VecsClient, storeName, storePass))
                    {
                        // add secret key to session here.
                        session.AddSecretKeyEntry(dto.Alias, dto.SecretKey, dto.Password, null);
                    }
                    if (ListView != null)
                        ListView.Refresh();
                }
            });
        }
    }
}
