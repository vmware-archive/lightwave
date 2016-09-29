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
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class AddNewAssertionConsumerService : Form,IView
    {
        private AssertionConsumerServiceDto _dto;
        public AddNewAssertionConsumerService()
        {
            InitializeComponent();
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {            
            if(string.IsNullOrEmpty(txtName.Text.Trim()))
            {
                MMCDlgHelper.ShowWarning("Enter a valid name");
                return;
            }
             if(string.IsNullOrEmpty(txtEndpoint.Text.Trim()) && !Uri.IsWellFormedUriString(txtEndpoint.Text, UriKind.Absolute))
            {
                MMCDlgHelper.ShowWarning("Enter a valid endpoint");
                return;
            }
             if(string.IsNullOrEmpty(txtBinding.Text.Trim()))
            {
                MMCDlgHelper.ShowWarning("Enter a valid binding");
                return;
            }
             _dto = new AssertionConsumerServiceDto { Name = txtName.Text, Endpoint = txtEndpoint.Text, Binding = txtBinding.Text, Index = (int)nudIndex.Value, IsDefault =  chkDefault.Checked };
            DialogResult = DialogResult.OK;
            Close();
        }

        public Dto.IDataContext DataContext
        {
            get { return _dto; }
        }
    }
}
