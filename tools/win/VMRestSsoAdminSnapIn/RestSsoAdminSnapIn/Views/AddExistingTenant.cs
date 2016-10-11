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
using System.Drawing;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class AddExistingTenant : Form, IView
    {
        private TenantDto _tenantDto;
        public AddExistingTenant()
        {
            InitializeComponent();
        }
        private void btnOK_Click(object sender, EventArgs e)
        {
            _tenantDto = new TenantDto { Name = txtTenantName.Text };
            Close();
        }
        void EnableOk()
        {
            btnOK.Enabled = txtTenantName.Text.Length > 0;
        }
        public IDataContext DataContext
        {
            get { return _tenantDto; }
        }

        private void txtTenantName_TextChanged(object sender, EventArgs e)
        {
            EnableOk();
        }
        private void AddExistingTenant_Load(object sender, EventArgs e)
        {
            this.Icon = ResourceHelper.GetResourceIcon("Vmware.Tools.RestSsoAdminSnapIn.Images.User.ico");
        }
    }
}
