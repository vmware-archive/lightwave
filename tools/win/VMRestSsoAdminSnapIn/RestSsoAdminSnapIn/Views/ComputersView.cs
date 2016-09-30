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
using System.Linq;
using System.Collections.Generic;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class ComputersView : Form
    {
        private readonly ServerDto _serverDto;
        private string _systemTenant;
        public ComputersView(ServerDto serverDto, string systemTenant)
        {
            InitializeComponent();
            _serverDto = serverDto;
            _systemTenant = systemTenant;
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void ComputersView_Load(object sender, EventArgs e)
        {
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstComputers.SmallImageList = il;
            var auths = SnapInContext.Instance.AuthTokenManager.GetAuthTokens(_serverDto);
            var auth = auths.FirstOrDefault(x=>x.ServerDto.Tenant == _systemTenant);
            
            ActionHelper.Execute(delegate()
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var computers = service.Server.GetComputers(_serverDto, auth.Token);
                foreach (var computer in computers)
                {
                    var item = new ListViewItem(new[] { computer.HostName, computer.DomainController?"YES":"NO" }) { ImageIndex = (int)TreeImageIndex.Computers };
                    lstComputers.Items.Add(item);
                    lstComputers.Columns[0].Width = 250;
                    lstComputers.Columns[1].Width = 100;
                }
            }, auth);
        }
    }
}
