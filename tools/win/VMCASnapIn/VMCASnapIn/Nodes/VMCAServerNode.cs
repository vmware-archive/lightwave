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
using System.Security.Cryptography.X509Certificates;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using VMCA;
using VMCASnapIn.DTO;
using VMCASnapIn.ListViews;
using VMCASnapIn.Services;
using VMCASnapIn.UI;
using System.IO;
using VMwareMMCIDP.UI;
using VMwareMMCIDP.UI.Common;
using VMCASnapIn.Utilities;
using System;
using System.Threading.Tasks;
using VMCA.Client;
using VMIdentity.CommonUtils;
using VMCASnapIn;
using System.ComponentModel;
using VMCASnapIn.UI.GridEditors;
using System.Drawing.Design;
using VMCA.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.Nodes
{
    class VMCAServerNode : ChildScopeNode
    {
        const int ACTION_SHOW_ROOT_CERTIFICATE = 1;
        const int ACTION_GET_SERVER_VERSION = 2;
        const int ACTION_ADD_ROOT_CERTIFICATE = 4;
        const int ACTION_LOGIN = 5;
        const int ACTION_LOGOUT = 6;

        public VMCAServerNode(VMCAServerDTO dto):base(dto)
        {
            this.DisplayName = dto.Server;
            this.EnabledStandardVerbs = StandardVerbs.Delete | StandardVerbs.Refresh;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Login",
                "Login", -1, ACTION_LOGIN));
            ImageIndex = SelectedImageIndex = (int)VMCAImageIndex.Server;
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = "dto.Server";
            lvd.ViewType = typeof(MmcListView);
            this.ViewDescriptions.Add(lvd);
            this.ViewDescriptions.DefaultIndex = 0;
            PopulateChildren();
        }

        void AddViewDescription(ScopeNode node, ViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
            this.Children.Add(node);
        }

        public void PopulateChildren()
        {
            this.Children.Clear();
            if (ServerDTO.CanLogin())
            {
                AddChildren();
            }
        }

        void AddChildren()
        {
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = "Certificate details";
            lvd.ViewType = typeof(CertificateDetailsListView);

            var activeNode = new VMCACertsNode(ServerDTO);
            activeNode.DisplayName = "Active Certificates";
            activeNode.Tag = CertificateState.Active;
            activeNode.ImageIndex = activeNode.SelectedImageIndex = (int)VMCAImageIndex.ActiveCertificate;
            AddViewDescription(activeNode, lvd);

            var revokedNode = new VMCACertsNode(ServerDTO);
            revokedNode.DisplayName = "Revoked Certificates";
            revokedNode.Tag = CertificateState.Revoked;
            revokedNode.ImageIndex = revokedNode.SelectedImageIndex = (int)VMCAImageIndex.RevokedCertificate;
            AddViewDescription(revokedNode, lvd);

            var expiredNodes = new VMCACertsNode(ServerDTO);
            expiredNodes.DisplayName = "Expired Certificates";
            expiredNodes.Tag = CertificateState.Expired;
            expiredNodes.ImageIndex = expiredNodes.SelectedImageIndex = (int)VMCAImageIndex.ExpiredCertificate;
            AddViewDescription(expiredNodes, lvd);

            var personalNodes = new VMCAPersonalNode(ServerDTO);
            personalNodes.DisplayName = "Personal Certificates";
            personalNodes.ImageIndex = personalNodes.SelectedImageIndex = (int)VMCAImageIndex.Certificate;
            this.Children.Add(personalNodes);
        }


        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch((int)action.Tag)
            {
                case ACTION_GET_SERVER_VERSION:
                    MMCDlgHelper.ShowMessage(VMCACertificateService.GetVersion(ServerDTO));
                    break;
                case ACTION_SHOW_ROOT_CERTIFICATE:
                    ShowRootCertificate();
                    break;
                case ACTION_ADD_ROOT_CERTIFICATE:
                    AddRootCertificate();
                    break;
                case ACTION_LOGIN:
                    DoLogin();
                    break;
                case ACTION_LOGOUT:
                    DoLogout();
                    break;
            }
        }

        void AddRootCertificate()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var dto = new AddCertificateDTO { PrivateKey = new PrivateKeyDTO() };
                var frm = new FormAddRootCert(dto);
                if (MMCDlgHelper.ShowForm(frm))
                {
                    var cert = File.ReadAllText(dto.Certificate);
                    ServerDTO.VMCAClient.AddRootCertificate(cert, "", dto.PrivateKey.ToString());
                }
            });
        }

        public void ShowRootCertificate()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var cert = ServerDTO.VMCAClient.GetRootCertificate();
                X509Certificate2UI.DisplayCertificate(cert);
            });
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetDeleteMsg("server", ServerDTO.Server)))
                return;
            base.OnDelete(status);
            VMCASnapInEnvironment.Instance.LocalData.RemoveServer(ServerDTO);
            (Parent as VMCARootNode).RefreshAll();
        }
        protected override void OnRefresh(AsyncStatus status)
        {
            PopulateChildren();
        }
        public async Task DoLogin()
        {
            try
            {
                if (!ServerDTO.IsLoggedIn)
                {
                    var frmLogin = new LoginForm(ServerDTO.Server, ServerDTO.UserName, ServerDTO.DomainName);
                    if (MMCDlgHelper.ShowForm(frmLogin))
                    {
                        ServerDTO.Server = frmLogin.Server;
                        ServerDTO.UserName = frmLogin.UserName;
                        ServerDTO.Password = frmLogin.Password;
                        ServerDTO.DomainName = frmLogin.DomainName;
                        await ServerDTO.LogintoServer(frmLogin.UserName, frmLogin.Password, frmLogin.DomainName);
                        if (ServerDTO.IsLoggedIn)
                        {
                            PopulateChildren();
                            this.DisplayName = ServerDTO.Server;
                            this.ActionsPaneItems.Clear();
                            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Logout",
                                               "Logout", -1, ACTION_LOGOUT));
                            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Get Server version",
                                               "Get Server version", -1, ACTION_GET_SERVER_VERSION));
                            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Show root certificate",
                                                       "Show VMCA Root certificate", -1, ACTION_SHOW_ROOT_CERTIFICATE));
                            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add root certificate",
                                                       "Add Root certificate", -1, ACTION_ADD_ROOT_CERTIFICATE));
                            this.ViewDescriptions[0].DisplayName = "Logged in as: " + ServerDTO.UserName + "@" + ServerDTO.DomainName;
                        
                        }
                        else
                        {
                            throw new Exception(CommonConstants.UNABLE_TO_LOGIN);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                MMCDlgHelper.ShowException(e);
            }
        }

        public void DoLogout()
        {
            MMCActionHelper.CheckedExec(delegate
            {
                if (ServerDTO.IsLoggedIn)
                {
                    ServerDTO.Cleanup();
                    this.Children.Clear();
                    this.ActionsPaneItems.Clear();
                    this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Login",
               "Login", -1, ACTION_LOGIN));
                }
            });
        }
    }
}
