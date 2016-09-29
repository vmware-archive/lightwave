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
using System.Linq;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.ScopeNodes;
using VMPscHighAvailabilitySnapIn.Utils;
using VMPscHighAvailabilitySnapIn.SnapIn;
using VMPSCHighAvailability.Common.Helpers;

namespace VMPscHighAvailabilitySnapIn.UI
{
    public partial class ServerView : UserControl, IFormViewControl
    {
        ServerFormView _formView;

        /// <summary>
        /// Type of view
        /// </summary>
        public ViewType View { get; set; }

        public ServerView()
        {
            InitializeComponent();
        }

        private void GlobalView_Load(object sender, EventArgs e)
        {
            Dock = DockStyle.Fill;            
        }

        public void LoadData(bool load = true)
        {
            lstdcs.Items.Clear();
            if (!load)
            {
                pnlGlobalView.Visible = false;
                lstNoRecordsView.Visible = true;
                return;
            }
            var node = _formView.ScopeNode as ServerNode;
            var show = !string.IsNullOrEmpty(node.ServerDto.Password);
            lstNoRecordsView.Visible = !show;
            pnlGlobalView.Visible = show;
            if (show)
            {
                var il = new ImageList();
                var image = ResourceHelper.GetToolbarImage();
                il.Images.AddStrip(image);
                lstdcs.SmallImageList = il;
                txtServername.Text = node.ServerDto.Server;
                txtDomainname.Text = node.ServerDto.DomainName;                

                var dataSource = GetDataSource();

                if (dataSource != null)
                {
                    foreach (NodeDto dc in dataSource.OrderByDescending(x => x.NodeType).ToList())
                    {
                        var status = dc.Active
                            ? Constants.Active
                            : Constants.InActive;

                        string[] values = new string[] { dc.Sitename, dc.Name, status, dc.NodeType.ToString() };
                        var imageIndex = dc.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure
                            ? (int)ImageIndex.Infrastructure
                            : (int)ImageIndex.Management;
                        ListViewItem item = new ListViewItem(values) { Tag = dc, ImageIndex = imageIndex };
                        item.BackColor = dc.Active ? Color.LightGreen : Color.Pink;
                        lstdcs.Items.Add(item);
                    }
                }
                GetDomainFunctionalLevel();
            }
        }

        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (ServerFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
        }

        private List<NodeDto> GetDataSource()
        {
            List<NodeDto> ds = null;
            var node = _formView.ScopeNode as ServerNode;
            if (node != null)
                ds = node.Hosts;
            return ds;
        }

        internal void RefreshDataSource()
        {
            LoadData();
        }

        private void GetDomainFunctionalLevel()
        {
            try
            {
                var domainFucntionalLevel = string.Empty;
                var serverNode = _formView.ScopeNode as ServerNode;
                if (serverNode != null)
                {
                    var dto = serverNode.Hosts.FirstOrDefault(x => x.NodeType == NodeType.Infrastructure && x.Active);
                    if (dto != null)
                    {
                        if (serverNode.ServerDto != null)
                        {
                            var serverDto = new ServerDto { Server = dto.Name, Upn = serverNode.ServerDto.Upn, Password = serverNode.ServerDto.Password, DomainName = serverNode.ServerDto.DomainName };
                            domainFucntionalLevel = PscHighAvailabilityAppEnvironment.Instance.Service.GetDomainFunctionalLevel(serverDto);
                        }
                    }
                }
                txtDomainFunctionalLevel.Text  = domainFucntionalLevel;
            }
            catch(Exception exc)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
                MiscUtilsService.ShowError(exc);
            }
        }
    }
}
