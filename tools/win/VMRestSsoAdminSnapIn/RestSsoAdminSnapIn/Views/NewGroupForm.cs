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
    public partial class NewGroupForm : Form, IView
    {
        private GroupDto _groupDto;

        public NewGroupForm()
        {
            InitializeComponent();
        }

        private bool ViewToDataContext()
        {
            if (string.IsNullOrWhiteSpace(txtGroupName.Text))
            {
                MMCDlgHelper.ShowWarning("Please enter a valid name for the group");
                return false;
            }
            _groupDto = new GroupDto { GroupName = txtGroupName.Text, GroupDetails = new GroupDetailsDto{ Description = txtDescription.Text } };
            return true;
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (ViewToDataContext())
            {
                DialogResult = DialogResult.OK;
                Close();
            }
        }

        public IDataContext DataContext
        {
            get { return _groupDto; }
        }
    }
}
