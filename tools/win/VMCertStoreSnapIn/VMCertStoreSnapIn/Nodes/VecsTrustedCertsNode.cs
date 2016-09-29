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
using System;
using System.IO;
using System.Security.Cryptography.X509Certificates;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Vecs;
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.ListViews;
using VMCertStoreSnapIn.UI;
using VMCertStoreSnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsTrustedCertsNode : VecsStoreEntriesNode
    {
        const int ACTION_ADD_CERTIFICATE = 1;

        public VecsTrustedCertsNode(VMCertStoreServerDTO dto, string storeName)
            : base(dto, storeName)
        {
            ImageIndex = SelectedImageIndex = (int)VMCertStoreImageIndex.TrustedCert;
            this.EnabledStandardVerbs = StandardVerbs.Refresh;
            DisplayName = "Trusted Certs";
            this.Tag = storeName;
            InitConsole();
        }

        void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
        }

        void InitConsole()
        {
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(
                                        "Add Certificate",
                                        "Add Certificate",
                                        -1,
                                        ACTION_ADD_CERTIFICATE));

            var lvd = new MmcListViewDescription();
            lvd.DisplayName = "certificate details";
            lvd.ViewType = typeof(VecsTrustedCertsListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;

            AddViewDescription(this, lvd);
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            if (ListView != null)
            {
                ListView.Refresh();
            }
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((int)action.Tag)
            {
                case ACTION_ADD_CERTIFICATE:
                    AddCertificate();
                    break;
            }
        }

        void AddCertificate()
        {
             MMCActionHelper.CheckedExec(delegate()
            {
                var frm = new frmAddCertificate();
                if (MMCDlgHelper.ShowForm(frm))
                {
                    var dto = frm.CertificateDTO;
                    string storeName = (string)Tag;
                    string storePass = "";
                    using (var session = new VecsStoreSession(ServerDTO.VecsClient, storeName, storePass))
                    {
                        session.AddCertificateEntry(dto.Alias, "", "", dto.Certificate);
                        if (ListView != null)
                        {
                            ListView.Refresh();
                        }
                    }
                }
            });
        }
    }
}
