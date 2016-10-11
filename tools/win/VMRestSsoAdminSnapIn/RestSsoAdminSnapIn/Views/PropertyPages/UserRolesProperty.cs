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
    public partial class UserRolesProperty : UserControl
    {
        private readonly bool _isSystemDomain;
        private UserDto _userDto;
        private readonly IPropertyDataManager _dataManager;
        private GenericPropertyPage _parent;
        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        public UserRolesProperty(IPropertyDataManager dataManager, bool systemDomain)
        {
            _dataManager = dataManager;
            _isSystemDomain = systemDomain;
            InitializeComponent();
            PropertyPageInit();
            EnableDisableControls(systemDomain);
        }

        private void EnableDisableControls(bool enable)
        {
            cboRole.Enabled = enable;
            chkActAsUser.Enabled = enable;
            chkIDPAdmin.Enabled = enable;
        }

        void PropertyPageInit()
        {
            _parent = new GenericPropertyPage {Control = this};
            _parent.Apply += _parent_Apply;
            _parent.Initialize += _parent_Initialize;
        }

        void _parent_Initialize(object sender, EventArgs e)
        {
            BindControls();
            HookChanges();
        }

        void _parent_Apply(object sender, CancelEventArgs e)
        {
            _userDto.Role = (UserRole)cboRole.SelectedIndex;
            _userDto.ActAsUsers = chkActAsUser.Checked;
            _userDto.IsIdpAdmin = chkIDPAdmin.Checked;
            var success = _dataManager.Apply(_userDto);
            e.Cancel = !success;
            if (success)
            {
                BindControls();
            }
        }

        void BindControls()
        {
            _userDto = _dataManager.GetData() as UserDto;

            if (_userDto != null)
            {
                cboRole.SelectedIndex = (int)_userDto.Role;
                chkActAsUser.Checked = _userDto.ActAsUsers;
                chkIDPAdmin.Checked = _userDto.IsIdpAdmin;
            }
        }

        void HookChanges()
        {
            cboRole.SelectedIndexChanged += ContentChanged;
            chkActAsUser.CheckedChanged +=ContentChanged;
            chkIDPAdmin.CheckedChanged += ContentChanged;
        }

        void ContentChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }
    }
}
