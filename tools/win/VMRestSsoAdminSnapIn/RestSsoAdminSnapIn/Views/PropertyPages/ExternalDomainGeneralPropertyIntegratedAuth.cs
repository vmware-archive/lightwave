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
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class ExternalDomainGeneralPropertyIntegratedAuth : UserControl
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
        public ExternalDomainGeneralPropertyIntegratedAuth(IPropertyDataManager manager, IdentityProviderDto providerDto)
        {
            _providerDto = providerDto;
            _dataManager = manager;
            InitializeComponent();
            PropertyPageInit();
        }

        private void PropertyPageInit()
        {
            pbIcon.Image = ResourceHelper.GetResourceIcon("Vmware.Tools.RestSsoAdminSnapIn.Images.User.ico").ToBitmap();
            _parent = new GenericPropertyPage { Control = this };
            _parent.Apply += new CancelEventHandler(_parent_Apply);
            _parent.Initialize += new EventHandler(_parent_Initialize);
        }

        private void _parent_Initialize(object sender, EventArgs e)
        {
            BindControls();
            HookChanges();
        }
        private void HookChanges()
        {
        }
        private void ContentChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }
        private void _parent_Apply(object sender, CancelEventArgs e)
        {
            _providerDto.UserMachineAccount = rdoMachineAccount.Checked;

            if (!_providerDto.UserMachineAccount)
            {
                _providerDto.Username = txtADUsername.Text;
                _providerDto.Password = txtADPassword.Text;
                _providerDto.ServicePrincipalName = txtADSpn.Text;               
            }
            e.Cancel = !_dataManager.Apply(_providerDto);
        }

        private void BindControls()
        {
            if (_providerDto == null)
                return;

            lblDomainName.Text = _providerDto.Name;
            txtADUsername.Text = _providerDto.Username;
            txtADSpn.Text = _providerDto.ServicePrincipalName;
            rdoMachineAccount.Checked = _providerDto.UserMachineAccount;
            rdoSpn.Checked = !_providerDto.UserMachineAccount;
        }
        private void txtADSpn_TextChanged(object sender, EventArgs e)
        {
            ContentChanged(sender, e);
        }

        private void txtADUsername_TextChanged(object sender, EventArgs e)
        {
            ContentChanged(sender, e);
        }

        private void txtADPassword_TextChanged(object sender, EventArgs e)
        {
            ContentChanged(sender, e);
        }

        private void rdoMachineAccount_CheckedChanged(object sender, EventArgs e)
        {
            ContentChanged(sender, e);
        }

        private void rdoSpn_CheckedChanged(object sender, EventArgs e)
        {
            ContentChanged(sender, e);
        }

        private void cbSiteAffinity_CheckedChanged(object sender, EventArgs e)
        {
            ContentChanged(sender, e);
        }
    }
}
