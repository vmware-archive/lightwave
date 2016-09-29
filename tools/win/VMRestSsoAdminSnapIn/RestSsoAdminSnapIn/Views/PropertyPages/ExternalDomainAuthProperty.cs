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
using System.ComponentModel;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class ExternalDomainAuthProperty : UserControl
    {
        private GenericPropertyPage _parent;
        private IPropertyDataManager _dataManager;
        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        private IdentityProviderDto _providerDto;
        public ExternalDomainAuthProperty(IPropertyDataManager dataManager, IdentityProviderDto dto)
        {
            _providerDto = dto;
            _dataManager = dataManager;
            InitializeComponent();
            PropertyPageInit();
        }

        void PropertyPageInit()
        {
            _parent = new GenericPropertyPage { Control = this };
            _parent.Apply += new CancelEventHandler(_parent_Apply);
            _parent.Initialize += new EventHandler(_parent_Initialize);
        }

        void _parent_Initialize(object sender, EventArgs e)
        {
            BindControls();
            HookChanges();
        }

        private void HookChanges()
        {
            txtUserName.TextChanged += ContentChanged;
            txtPassword.TextChanged += ContentChanged;
            txtSPN.TextChanged += ContentChanged;
        }
        void ContentChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }

        void _parent_Apply(object sender, CancelEventArgs e)
        {
            _providerDto.AuthenticationType = "PASSWORD";
            _providerDto.Username = txtUserName.Text;
            e.Cancel = !_dataManager.Apply(_providerDto);
        }

        void BindControls()
        {
            txtUserName.Text = _providerDto.Username;
        }

        private void ExternalDomainAuthProperty_Load(object sender, EventArgs e)
        {
            var isADWithWindowsAuth = _providerDto.Type == "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY";
            txtSPN.Visible = isADWithWindowsAuth;
            lblSPN.Visible = isADWithWindowsAuth;
        }
    }
}
