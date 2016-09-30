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
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class UserGeneralProperty : UserControl
    {
        GenericPropertyPage _parent;
        private readonly bool _isSystemDomain;
        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        UserDto _userDto;
        readonly IPropertyDataManager _dataManager;
        public UserGeneralProperty(IPropertyDataManager mgr, bool systemDomain)
        {
            _dataManager = mgr;
            _isSystemDomain = systemDomain;
            InitializeComponent();
            PropertyPageInit();
            EnableDisableControls(systemDomain);
        }

        void PropertyPageInit()
        {
            pictureBox1.Image = ResourceHelper.GetResourceIcon("Vmware.Tools.RestSsoAdminSnapIn.Images.User.ico").ToBitmap();
            _parent = new GenericPropertyPage {Control = this};
            _parent.Apply += _parent_Apply;
            _parent.Initialize += _parent_Initialize;
        }

        private void EnableDisableControls(bool enable)
        {
            txtFirstName.Enabled = enable;
            txtLastName.Enabled = enable;
            txtDescription.Enabled = enable;
            txtEmail.Enabled = enable;
            chkDisabled.Enabled = enable;
        }

        void _parent_Initialize(object sender, EventArgs e)
        {
            BindControls();
            HookChanges();
        }

        void _parent_Apply(object sender, CancelEventArgs e)
        {
            _userDto.PersonDetails.FirstName = txtFirstName.Text;
            _userDto.PersonDetails.LastName = txtLastName.Text;
            _userDto.PersonDetails.EmailAddress = txtEmail.Text;
            _userDto.PersonDetails.Description = txtDescription.Text;
            _userDto.Disabled = chkDisabled.CheckState == CheckState.Checked;
            e.Cancel = !_dataManager.Apply(_userDto);
        }

        void HookChanges()
        {
            txtFirstName.TextChanged += ContentChanged;
            txtLastName.TextChanged += ContentChanged;
            txtEmail.TextChanged += ContentChanged;
            txtDescription.TextChanged += ContentChanged;
            chkDisabled.CheckedChanged += ContentChanged;
        }

        void ContentChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }

        void BindControls()
        {
            _userDto = _dataManager.GetData() as UserDto;
            if (_userDto != null)
            {
                lblUserName.Text = string.Format("{0}@{1}", _userDto.Name, _userDto.Domain);
                if (_userDto.PasswordDetails != null)
                {
                    lblPasswordLastSetOn.Text = DateTimeHelper.UnixToWindows((long)_userDto.PasswordDetails.LastSet).ToString("dd-MMM-yyyy hh:mm:ss");
                    lblDaysUntilPasswordExpiry.Text = SecondsToDaysAndHours(_userDto.PasswordDetails.Lifetime);
                }
                txtFirstName.Text = _userDto.PersonDetails.FirstName;
                txtLastName.Text = _userDto.PersonDetails.LastName;
                txtEmail.Text = _userDto.PersonDetails.EmailAddress;
                txtDescription.Text = _userDto.PersonDetails.Description;
                chkDisabled.CheckState = _userDto.Disabled ? CheckState.Checked : CheckState.Unchecked;
            }
        }

        private string SecondsToDaysAndHours(long seconds)
        {
            var hours =  seconds / (60 * 60);
            return string.Format("{0} days {1} hours", hours / 24, hours % 24);
        }
    }
}
