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
    public partial class SolutionUserGeneralProperty : UserControl
    {
        private GenericPropertyPage _parent;
        private IPropertyDataManager _dataManager;
        private SolutionUserDto _userDto;

        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        public SolutionUserGeneralProperty(IPropertyDataManager dataManager)
        {
            _dataManager = dataManager;
            _userDto = dataManager.GetData() as SolutionUserDto;
            InitializeComponent();
            PropertyPageInit();
        }

        void PropertyPageInit()
        {
            pictureBox1.Image = ResourceHelper.GetResourceIcon("Vmware.Tools.RestSsoAdminSnapIn.Images.User.ico").ToBitmap();
            _parent = new GenericPropertyPage {Control = this};
            _parent.Initialize += _parent_Initialize;
            _parent.Apply += _parent_Apply;
        }
        void _parent_Apply(object sender, CancelEventArgs e)
        {
            _userDto.Description = txtDescription.Text;
            _userDto.Disabled = chkDisabled.Checked;
            e.Cancel = !_dataManager.Apply(_userDto);
        }
        void _parent_Initialize(object sender, EventArgs e)
        {
            BindControls();
        }

        void BindControls()
        {
            lblUserName.Text = string.Format("{0}@{1}", _userDto.Name, _userDto.Domain);
            txtDescription.Text = _userDto.Description;
            chkDisabled.Checked = _userDto.Disabled;
        }

        private void txtDescription_TextChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }

        private void chkDisabled_CheckedChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }
    }
}
