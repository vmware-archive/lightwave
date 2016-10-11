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
using VMCASnapIn.DTO;
using Microsoft.ManagementConsole;
using VMCASnapIn.ListViews;
using VMCASnapIn.Utilities;
using System;
using VMwareMMCIDP.UI;
using System.Windows.Forms;
using VMCA;
using System.ComponentModel;
using VMCASnapIn.UI.GridEditors;
using System.Drawing.Design;
using VMwareMMCIDP.UI.Common.Utilities;
using VMwareMMCIDP.UI.Common;

namespace VMCASnapIn.Nodes
{
    public class VMCACSRNode : ChildScopeNode
    {
        const int ACTION_CREATE_SIGNING_REQUEST = 1;

        public VMCACSRNode(VMCAServerDTO dto):base(dto)
        {
            DisplayName = "Signing Requests";

            this.EnabledStandardVerbs = StandardVerbs.Refresh;
            ImageIndex = SelectedImageIndex = (int)VMCAImageIndex.Certificate;
            InitNode();
        }

        void InitNode()
        {
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = "csr details";
            lvd.ViewType = typeof(CSRDetailListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;
            ViewDescriptions.Add(lvd);
            ViewDescriptions.DefaultIndex = 0;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Create Signing Request",
                                       "Create Signing Request", -1, ACTION_CREATE_SIGNING_REQUEST));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_CREATE_SIGNING_REQUEST:
                    CreateSigningRequest(ServerDTO);
                    OnRefresh(status);
                    break;
            }

        }

        public static void CreateSigningRequest(VMCAServerDTO serverDTO)
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                TypeDescriptor.AddAttributes(typeof(PrivateKeyDTO),new EditorAttribute
                    (typeof(PrivateKeyEditor), typeof(UITypeEditor)));
                TypeDescriptor.AddAttributes(typeof(PrivateKeyDTO), new CategoryAttribute("Security"));
                var dto = new CertRequestDTO();
                var frm = new GenericInputForm("Fill Signing Request", "Create", dto);
                frm.Icon = VMCASnapInEnvironment.Instance.GetIconResource(VMCAIconIndex.cert);
                frm.ApplyDelegate = MiscUtilsService.ApproveCertRequestHandler;
                if (MMCDlgHelper.ShowForm(frm))
                {
                    using (var request = new VMCARequest(serverDTO.VMCAClient))
                    {
                        dto.FillRequest(request);
                        string csr = request.GetCSR(dto.PrivateKey.ToString());
                        serverDTO.SigningRequests.Add(new SigningRequestDTO { CSR = csr, CreatedDateTime = DateTime.Now });
                        VMCASnapInEnvironment.Instance.SaveLocalData();
                        var frm2 = new InfoForm(csr);
                        frm2.Icon = VMCASnapInEnvironment.Instance.GetIconResource(VMCAIconIndex.cert);
                        MMCDlgHelper.ShowForm(frm2);
                    }
                }
            });
        }

        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
            OnRefresh(status);
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            if (ListView != null)
            {
                var list = ListView as CSRDetailListView;
                list.RefreshList();
            }
        }
    }
}
