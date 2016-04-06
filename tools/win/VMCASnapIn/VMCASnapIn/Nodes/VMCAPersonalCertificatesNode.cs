﻿/*
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
using VMCASnapIn.DTO;
using VMCASnapIn.ListViews;
using Microsoft.ManagementConsole;
using System;
using System.Windows.Forms;
using VMCASnapIn.Utilities;
using VMwareMMCIDP.UI;
using System.Security.Cryptography.X509Certificates;
using VMCA;
using System.ComponentModel;
using VMCASnapIn.UI.GridEditors;
using System.Drawing.Design;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.Nodes
{
    public class VMCAPersonalCertificatesNode : ChildScopeNode
    {
        const int ACTION_CREATE_SELF_SIGNED_CERT = 1;

        public VMCAPersonalCertificatesNode(VMCAServerDTO dto)
            : base(dto)
        {
            DisplayName = "Certificates";
            Tag = -1;

            this.EnabledStandardVerbs = StandardVerbs.Refresh;
            ImageIndex = SelectedImageIndex = (int)VMCAImageIndex.Certificate;

            InitNode();
        }

        void InitNode()
        {
            MmcListViewDescription lvd = new MmcListViewDescription();
            lvd.DisplayName = "Certificate details";
            lvd.ViewType = typeof(CertificateDetailsListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;
            ViewDescriptions.Add(lvd);
            ViewDescriptions.DefaultIndex = 0;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Create Self Signed Certificate",
                                       "Create Self Signed Certificate", -1, ACTION_CREATE_SELF_SIGNED_CERT));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_CREATE_SELF_SIGNED_CERT:
                    if(CreateSelfSignedCert(ServerDTO))
                        OnRefresh(status);
                    break;
            }
        }

        public static bool CreateSelfSignedCert(VMCAServerDTO serverDTO)
        {
            bool bResult = false;
            MMCActionHelper.CheckedExec(delegate()
            {
                TypeDescriptor.AddAttributes(typeof(PrivateKeyDTO), new EditorAttribute
                    (typeof(PrivateKeyEditor), typeof(UITypeEditor)));
                TypeDescriptor.AddAttributes(typeof(PrivateKeyDTO), new CategoryAttribute("Security"));

                var dto = new CertRequestDTO();
                var frm = new GenericInputForm("Fill Certificate Request", "Create", dto);
                frm.Icon = VMCASnapInEnvironment.Instance.GetIconResource(VMCAIconIndex.cert);
                frm.ApplyDelegate = MiscUtilsService.ApproveCertRequestHandler;
                if (!MMCDlgHelper.ShowForm(frm))
                    return;

                var request = new VMCARequest(serverDTO.VMCAClient);
                dto.FillRequest(request);
                var vmcaCert = request.GetSelfSignedCertificate(dto.PrivateKey.ToString(), dto.NotBefore, dto.NotAfter);
                var cert = vmcaCert.GetX509Certificate2();
                X509Certificate2UI.DisplayCertificate(cert);

                var localCertDTO = new PrivateCertificateDTO
                {
                    Certificate = Convert.ToBase64String(cert.RawData)
                };
                serverDTO.PrivateCertificates.Add(localCertDTO);
                bResult = true;
            });
            return bResult;
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            if (ListView != null)
            {
                var list = ListView as CertificateDetailsListView;
                list.RefreshList();
            }
        }
    }
}
