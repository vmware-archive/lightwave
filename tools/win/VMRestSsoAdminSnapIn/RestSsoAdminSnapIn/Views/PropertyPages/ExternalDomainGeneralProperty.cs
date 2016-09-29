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
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class ExternalDomainGeneralProperty : UserControl
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
        public ExternalDomainGeneralProperty(IPropertyDataManager manager, IdentityProviderDto providerDto)
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
            txtDomainName.TextChanged += ContentChanged;
            txtDomainAlias.TextChanged += ContentChanged;
            txtUserBaseDN.TextChanged += ContentChanged;
            txtGroupBaseDN.TextChanged += ContentChanged;
            txtPrimaryURL.TextChanged += ContentChanged;
            cbSiteAffinity.CheckedChanged += ContentChanged;
        }
        private void ContentChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }
        private void _parent_Apply(object sender, CancelEventArgs e)
        {
            if (IsValid())
            {
                _providerDto.Name = txtDomainName.Text;
                _providerDto.Alias = txtDomainAlias.Text;
                _providerDto.UserBaseDN = txtUserBaseDN.Text;
                _providerDto.GroupBaseDN = txtGroupBaseDN.Text;
                _providerDto.ConnectionStrings[0] = txtPrimaryURL.Text;
                if (_providerDto.ConnectionStrings.Count > 1)
                {
                    _providerDto.ConnectionStrings[1] = txtSecondaryUrl.Text;
                }
                else
                {
                    _providerDto.ConnectionStrings.Add(txtSecondaryUrl.Text);
                }
                _providerDto.SiteAffinityEnabled = cbSiteAffinity.Checked;
                _providerDto.BaseDnForNestedGroupsEnabled = cbDNForNestedGroups.Checked;
                _providerDto.DirectGroupsSearchEnabled = cbGroupSearch.Checked;
                _providerDto.MatchingRuleInChainEnabled = cbMatchRuleInChain.Checked;
                e.Cancel = !_dataManager.Apply(_providerDto);
            }
        }

        private bool IsValid()
        {
           if(string.IsNullOrEmpty(txtDomainAlias.Text))
           {
               MMCDlgHelper.ShowWarning("Domain Alias cannot be left empty");
                   return false;
           }
           if (string.IsNullOrEmpty(txtUserBaseDN.Text))
           {
               MMCDlgHelper.ShowWarning("Base DN for users cannot be left empty");
               return false;
           }
           if (string.IsNullOrEmpty(txtGroupBaseDN.Text))
           {
               MMCDlgHelper.ShowWarning("Base DN for Group cannot be left empty");
               return false;
           }
           if (string.IsNullOrEmpty(txtPrimaryURL.Text))
           {
               MMCDlgHelper.ShowWarning("Primary URL cannot be left empty");
               return false;
           }
           return true;
        }

        private void BindControls()
        {
            if (_providerDto == null)
                return;

            lblDomainName.Text = _providerDto.Name;
            txtDomainName.Text = _providerDto.Name;
            txtDomainAlias.Text = _providerDto.Alias;

            txtUserBaseDN.Text = _providerDto.UserBaseDN;
            txtGroupBaseDN.Text = _providerDto.GroupBaseDN;
            txtPrimaryURL.Text = _providerDto.ConnectionStrings[0];
            if (_providerDto.ConnectionStrings.Count > 1)
            {
                txtSecondaryUrl.Text = _providerDto.ConnectionStrings[1];
            }
            cbSiteAffinity.Checked = _providerDto.SiteAffinityEnabled;
            cbDNForNestedGroups.Checked = _providerDto.BaseDnForNestedGroupsEnabled;
            cbGroupSearch.Checked = _providerDto.DirectGroupsSearchEnabled;
            cbMatchRuleInChain.Checked = _providerDto.MatchingRuleInChainEnabled;
        }
    }
}
