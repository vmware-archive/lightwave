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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.ManagementConsole;
using VMCASnapIn.DTO;
using SSOAdminSnapIn.UI;
using System.Windows.Forms;
using VMCASnapIn.Services;
using System.Security.Cryptography.X509Certificates;
using System.Runtime.InteropServices;
using System.IO;
using VMCASnapIn.Utilities;
using VMCA.Client;

namespace VMCASnapIn.Nodes
{
    public class VMCAPersonalCertsNode : ScopeNode
    {
        const int ACTION_CREATE_PRIVATE_KEY = 1;
        const int ACTION_CREATE_SELF_SIGNED_CERT = 2;

        public VMCAPersonalCertsNode()
        {
            DisplayName = "Personal Certificates";
            Tag = -1;

            InitNode();
        }

        void InitNode()
        {
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Create Private Key",
                                       "Create Private Key", -1, ACTION_CREATE_PRIVATE_KEY));
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Create Self Signed Certificate",
                                       "Create Self Signed Certificate", -1, ACTION_CREATE_SELF_SIGNED_CERT));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_CREATE_PRIVATE_KEY:
                    CreatePrivateKey();
                    break;
                case ACTION_CREATE_SELF_SIGNED_CERT:
                    CreateSelfSignedCert();
                    break;
            }
        }

        bool ApproveCertRequestHandler(object obj, object objOld)
        {
            try
            {
                var dto = obj as CertRequestDTO;
                if (dto == null) return false;

                if (string.IsNullOrEmpty(dto.Country))
                    throw new Exception("Please enter a country");
                if (dto.Country.Length > 2)
                    throw new Exception("Country must not exceed 2 characters");
                if (string.IsNullOrEmpty(dto.Name))
                    throw new Exception("Please enter a name");
                if (string.IsNullOrEmpty(dto.PrivateKeyString))
                    throw new Exception("Please enter a private key");
            }
            catch (Exception exp)
            {
                MessageBox.Show(exp.Message);
                return false;
            }
            return true;
        }

        void CreateSelfSignedCert()
        {
            try
            {
/*                var dto = new CertRequestDTO();
                var frm = new GenericInputForm("Fill Certificate Request", "Create", dto);
                frm.ApplyDelegate = ApproveCertRequestHandler;
                if (frm.ShowDialog() != DialogResult.OK)
                    return;
                IntPtr cert = new IntPtr();

                UInt32 result = VMCAClientWrapper.VMCACreateSelfSignedCertificate(dto.GetRequestData(), dto.PrivateKey, null, dto.NotBefore.ToTime_t(), dto.NotAfter.ToTime_t(), out cert);
                MiscUtilsService.CheckStatus("CreateSelfSignedCert", result);

                var certString = Marshal.PtrToStringAnsi(cert);
                var crt = new X509Certificate2(ASCIIEncoding.ASCII.GetBytes(certString));
                X509Certificate2UI.DisplayCertificate(crt);

                VMCAClientWrapper.VMCAFreeCertificate(cert);*/
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
            }
        }

        void CreatePrivateKey()
        {
            MiscUtilsService.CheckedExec(delegate() { VMCAKeyPair.Create(256); });
        }
    }
}
