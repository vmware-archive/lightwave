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
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class TenantConfigurationView : Form, IView
    {
        private readonly ServerDto _serverDto;
        private readonly string _tenantName;
        private TenantConfigurationDto _tenantConfigurationDto;

        public TenantConfigurationView(ServerDto serverDto, string tenantName)
        {
            InitializeComponent();
            _serverDto = serverDto;
            _tenantName = tenantName;
        }
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            RefreshView();
            btnApply.Enabled = false;
        }

        private void RefreshView()
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
            ActionHelper.Execute(delegate()
            {                
                _tenantConfigurationDto = service.Tenant.GetConfig(_serverDto, _tenantName, auth.Token);                
            }, auth);
            propGridInput.SelectedObject = _tenantConfigurationDto;
            propGridInput.Refresh();
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
           {
               var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
               service.Tenant.UpdateConfig(_serverDto, _tenantName, _tenantConfigurationDto, auth.Token);
               service.Tenant.UpdatePasswordAndLockoutConfig(_serverDto, _tenantName, _tenantConfigurationDto, auth.Token);
               _tenantConfigurationDto = service.Tenant.GetConfig(_serverDto, _tenantName, auth.Token);
               propGridInput.SelectedObject = _tenantConfigurationDto;
               propGridInput.Refresh();
           }, auth);
        }

        public Dto.IDataContext DataContext
        {
            get { return _tenantConfigurationDto; }
        }

        private void propGridInput_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
        {
            btnApply.Enabled = true;
        }

    }
}
