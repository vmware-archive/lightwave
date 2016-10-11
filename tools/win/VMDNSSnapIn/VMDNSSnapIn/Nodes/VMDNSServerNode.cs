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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using VMDNS.Client;
using VMDNS.Common;
using VMDNSSnapIn.UI;
using VMIdentity.CommonUtils;
using VMIdentity.CommonUtils.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDNSSnapIn.Nodes
{
    public class VMDNSServerNode : ScopeNode
    {
        const int ACTION_LOGIN = 1;
        const int ACTION_SERVER_CONFIG = 2;
        const int ACTION_REFRESH = 3;
        Microsoft.ManagementConsole.Action LoginAction;

        public bool IsLoggedIn { get; set; }

        public List<VmDnsZone> ForwardZones { get; set; }

        public List<VmDnsZone> ReverseZones { get; set; }

        public List<string> Forwarders { get; set; }

        public VMDNSServerDTO ServerDTO { get; protected set; }

        public VMDNSServerNode(VMDNSServerDTO dto)
            : base()
        {
            ServerDTO = dto;
            DisplayName = ServerDTO.Server;
            this.ImageIndex = this.SelectedImageIndex = (int)VMDNSTreeImageIndex.Server;

            this.EnabledStandardVerbs = StandardVerbs.Refresh | StandardVerbs.Delete;
            LoginAction = new Microsoft.ManagementConsole.Action("Login",
               "Login", -1, ACTION_LOGIN);
            this.ActionsPaneItems.Add(LoginAction);
        }

        protected override void OnDelete(SyncStatus status)
        {
            base.OnDelete(status);
            RemoveServer();
        }

        void RemoveServer()
        {
            var result = MMCDlgHelper.ShowQuestion(MMCUIConstants.CONFIRM);
            if (result)
            {
                if (VMDNSSnapInEnvironment.Instance.LocalData.SerializableList.Contains(ServerDTO.Server))
                {
                    VMDNSSnapInEnvironment.Instance.LocalData.SerializableList.Remove(ServerDTO.Server);
                }

                var parent = this.Parent as VMDNSRootNode;

                if (parent != null)
                    parent.Children.Remove(this);

            }
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            PopulateChildren();
        }

        void PopulateChildren()
        {
            if (ServerDTO.IsLoggedIn)
            {
                this.Children.Clear();
                this.Children.Add(new VMDNSZonesNode(this));
            }
            else
            {
                UIErrorHelper.ShowError(MMCUIConstants.UNABLE_TO_LOGIN);
            }
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_LOGIN:
                    Login();
                    if (ServerDTO.IsLoggedIn)
                    {
                        this.ActionsPaneItems.Remove(LoginAction);
                        this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDNSConstants.SERVER_CONFIG,
    VMDNSConstants.SERVER_CONFIG, -1, ACTION_SERVER_CONFIG));
                    }
                    break;
                case ACTION_SERVER_CONFIG:
                    var frm = new ServerOptions(this);
                    SnapIn.Console.ShowDialog(frm);
                    break;
            }
        }

        public async void Login()
        {
            try
            {
                var frmLogin = new frmLogin();
                if (SnapIn.Console.ShowDialog(frmLogin) == DialogResult.OK)
                {
                    string Upn = frmLogin.UserName;
                    string[] userAndDomain = Upn.Split('@');
                    if (userAndDomain.Length != 2)
                    {
                        throw new Exception("Username is not UPN format");
                    }
                    else
                    {
                        ServerDTO.UserName = userAndDomain[0];
                        ServerDTO.DomainName = userAndDomain[1];
                        if (string.IsNullOrWhiteSpace(ServerDTO.UserName) || string.IsNullOrWhiteSpace(ServerDTO.DomainName))
                            throw new Exception(MMCUIConstants.VALUES_EMPTY);
                        if(!Network.IsValidIP(ServerDTO.Server))
                            throw new Exception(MMCUIConstants.INVALID_IP);
                        ServerDTO.Password = frmLogin.Password;
                    }
                    ServerDTO.ResetUserPass();
                    PopulateChildren();
                }
            }
            catch (Exception e)
            {
                //TODO - temp fix until we have Error lookup Api implemented on the server.
                UIErrorHelper.ShowError("Unable to connect to the Server");
            }
        }
            //TODO - make login async with timeout

        public void FillZonesInfo()
        {
            FillForwardZones();
            FillReverseZones();
            FillForwarders();
        }

        public void FillForwardZones()
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                ForwardZones = ServerDTO.DNSClient.ListZones(VmDnsZoneType.FORWARD).ToList();
            });
        }

        public void FillReverseZones()
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                ReverseZones = ServerDTO.DNSClient.ListZones(VmDnsZoneType.REVERSE).ToList();
            });
        }

        public void FillForwarders()
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                Forwarders = ServerDTO.DNSClient.GetForwarders().ToList();
            });
        }
       
    }
}
