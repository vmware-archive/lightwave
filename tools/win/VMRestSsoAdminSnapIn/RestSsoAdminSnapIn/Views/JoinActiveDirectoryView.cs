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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class JoinActiveDirectoryView : Form, IView
    {
        private ActiveDirectoryJoinInfoDto _adJoinInfoDto;
        private ServerDto _serverDto;
        public JoinActiveDirectoryView(ServerDto serverDto)
        {
            InitializeComponent();
            _serverDto = serverDto;
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (IsValid())
            {
                var adJoinInfoDto = ViewToDto();
                var auths = SnapInContext.Instance.AuthTokenManager.GetAuthTokens(_serverDto);
                var auth = auths[0];
                ActionHelper.Execute(delegate()
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                    _adJoinInfoDto = service.Adf.JoinActiveDirectory(_serverDto, adJoinInfoDto, auth.Token);
                    MMCDlgHelper.ShowWarning("Join operation completed successfully. Please reboot the node/server for the changes to take effect");
                    Close();
                }, auth);
            }
        }

        private bool IsValid()
        {
            // Validate the user inputs
            return true;
        }

        public ActiveDirectoryJoinRequestDto ViewToDto()
        {
            return new ActiveDirectoryJoinRequestDto
            {
                Username = txtUserName.Text,
                Password = txtPassword.Text,
                Domain = txtDomain.Text,
                OrganizationalUnit = string.IsNullOrEmpty(txtOrgUnit.Text) ? null : txtOrgUnit.Text
            };
        }

        private void ActiveDirectoryView_Load(object sender, EventArgs e)
        {           
        }

        public IDataContext DataContext
        {
            get { return _adJoinInfoDto; }
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}
