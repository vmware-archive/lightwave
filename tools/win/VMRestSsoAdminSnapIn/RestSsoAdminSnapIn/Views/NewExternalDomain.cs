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
using System.Linq;
using System.Collections.Generic;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewExternalDomain : Form
    {
        private readonly ServiceGateway _service;
        private readonly ServerDto _serverDto;
        private readonly string _tenantName;

        public NewExternalDomain(ServiceGateway service, ServerDto serverDto, string tenantName)
        {
            _service = service;
            _serverDto = serverDto;
            _tenantName = tenantName;
            InitializeComponent();
            cboAuthenticationType.SelectedIndex = 2;
        }

        private IdentityProviderDto ViewToDto()
        {
            var isAd = (cbIdentitySourceType.SelectedValue.ToString() == "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY");
            var attributes = GetAttributes();
            var attrmap = GetAttributeMap();
            var providerDto = new IdentityProviderDto
            {
                DomainType = DomainType.EXTERNAL_DOMAIN.ToString(),
                Name = txtDomainName.Text,
                Alias = isAd ? null : txtDomainAlias.Text,
                Type = cbIdentitySourceType.SelectedValue.ToString(),
                AuthenticationType = cboAuthenticationType.Text.ToUpper(),
                FriendlyName = isAd ? null : txtFriendlyName.Text,
                SearchTimeOutInSeconds = isAd ? 0 : (long)txtSearchTimeoutSeconds.Value,
                Username = isAd ? (rdoSpn.Checked ? txtADUsername.Text : null) : txtUserName.Text,
                Password = isAd ? (rdoSpn.Checked ? txtADPassword.Text : null) : txtPassword.Text,
                UserMachineAccount = isAd ? rdoMachineAccount.Checked : false,
                ServicePrincipalName = isAd ? txtADSpn.Text : null,
                UserBaseDN = isAd ? null : txtUserBaseDN.Text,
                GroupBaseDN = isAd ? null : txtGroupBaseDN.Text,
                ConnectionStrings = isAd ? null : new List<string> { txtPrimaryURL.Text },
                SiteAffinityEnabled = cbSiteAffinity.Checked,
                BaseDnForNestedGroupsEnabled = cbDNForNestedGroups.Checked,
                DirectGroupsSearchEnabled = cbGroupSearch.Checked,
                MatchingRuleInChainEnabled = cbMatchRuleInChain.Checked,
                Schema = new Dictionary<string, SchemaObjectMappingDto>(attributes),
                AttributesMap = new Dictionary<string, string>(attrmap)
            };
            return providerDto;
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (AtttributeMapIsValid())
            {
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
                ActionHelper.Execute(delegate()
                {
                    var provider = ViewToDto();
                    var result = _service.IdentityProvider.Create(_serverDto, _tenantName, provider, auth.Token);
                    this.DialogResult = DialogResult.OK;
                }, auth);
            }
        }

        private void NewExternalDomain_Load(object sender, EventArgs e)
        {
            PopulateIdentityySourceTypes();
        }
        private bool AtttributeMapIsValid()
        {
            var dictionary = new Dictionary<string, string>();
            for (var index = 0; index < dgAttributeMap.Rows.Count - 1; index++)
            {
                var row = dgAttributeMap.Rows[index];
                var value = row.Cells[0].Value.ToString();
                if (row.Cells[1].Value == null || (row.Cells[1].Value != null && string.IsNullOrEmpty(row.Cells[1].Value.ToString().Trim())))
                {
                    MMCDlgHelper.ShowWarning("Mapping for attribute " + value + "  is empty or invalid.");
                    return false;
                }

                if (dictionary.ContainsKey(value))
                {
                    MMCDlgHelper.ShowWarning("Multiple mapping exist for the same attribute");
                    return false;
                }
                dictionary.Add(value, value);
            }
            return true;
        }

        private void PopulateIdentityySourceTypes()
        {
            var items = new List<IdentityySourceTypeDto>{
                new IdentityySourceTypeDto{Key="IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY", Value="Active Directory (Integrated Windows Authentication)"},
                new IdentityySourceTypeDto{Key="IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING", Value="Active Directory as an LDAP Server"},
                new IdentityySourceTypeDto{Key="IDENTITY_STORE_TYPE_LDAP", Value="Open LDAP"}                
            };
            cbIdentitySourceType.DataSource = items;
            cbIdentitySourceType.DisplayMember = "Value";
            cbIdentitySourceType.ValueMember = "Key";
            cbIdentitySourceType.SelectedValue = "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY";
            rdoMachineAccount.Checked = true;
            cbIdentitySourceType_SelectedIndexChanged(cbIdentitySourceType, EventArgs.Empty);
            EnableDisableRemove();
            EnableDisableAdd();
            SetupAttributeMap();
        }

        private void cbIdentitySourceType_SelectedIndexChanged(object sender, EventArgs e)
        {
            var readOnly = (cbIdentitySourceType.SelectedValue.ToString() == "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY");
            pnlAD.Visible = readOnly;
            pnlNonAD.Visible = !readOnly;
            txtFriendlyName.ReadOnly = readOnly;
            txtUserBaseDN.ReadOnly = readOnly;
            txtGroupBaseDN.ReadOnly = readOnly;
            txtPrimaryURL.ReadOnly = readOnly;
            txtSearchTimeoutSeconds.Enabled = !readOnly;
            cboAuthenticationType.Enabled = !readOnly;
            if (readOnly)
            {
                cboAuthenticationType.SelectedIndex = 1;
            }
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            txtADUsername.ReadOnly = true;
            txtADSpn.ReadOnly = true;
            txtADPassword.ReadOnly = true;
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            txtADUsername.ReadOnly = false;
            txtADSpn.ReadOnly = false;
            txtADPassword.ReadOnly = false;
        }

        private Dictionary<string, SchemaObjectMappingDto> GetAttributes()
        {
            var dictionary = new Dictionary<string, SchemaObjectMappingDto>();

            for (var index = 0; index < dgAttributes.Items.Count; index++)
            {
                var item = dgAttributes.Items[index].Tag as SchemaObjectMappingDto;
                dictionary.Add(item.ObjectId, item);
            }
            return dictionary;
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            var dataSource = GetUnMappedObjectIds();
            var form = new SchemaObjectMappingView(null, dataSource);
            var result = form.ShowDialog(this);
            if (form.HasChanges)
            {
                var dto = (SchemaObjectMappingDto)form.DataContext;
                ObjectId id;
                if (Enum.TryParse(dto.ObjectId, false, out id))
                {
                    var listItem = new ListViewItem(id.GetDescription() + " - " + dto.ObjectClass) { Tag = dto };
                    dgAttributes.Items.Add(listItem);
                }
            }
        }
        private List<KeyValuePair> GetUnMappedObjectIds()
        {
            var keyValuePairs = GetAllObjectIds();
            var mappedObjectIds = GetAttributes();
            var dataSource = keyValuePairs.Where(x => !mappedObjectIds.ContainsKey(x.Key)).ToList();
            return dataSource;
        }
        private static List<KeyValuePair> GetAllObjectIds()
        {
            var id = new ObjectId();
            var keyValuePairs = id.ToKeyValueList();
            return keyValuePairs;
        }
        private string GetKeyFromDescription(string description)
        {
            var id = new AttributeId();
            return id.GetByDescription(description).ToString();
        }
        private List<KeyValuePair> GetAllAttributeIds()
        {
            var id = new AttributeId();
            var keyValuePairs = id.ToKeyValueList();
            return keyValuePairs;
        }

        private void btnRemove_Click(object sender, EventArgs e)
        {
            if (dgAttributes.SelectedIndices != null && dgAttributes.SelectedIndices.Count > 0)
            {
                dgAttributes.Items.RemoveAt(dgAttributes.SelectedIndices[0]);
                dgAttributes.Refresh();
                EnableDisableAdd();
                EnableDisableRemove();
            }
        }

        private void dgAttributes_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (dgAttributes.SelectedIndices != null && dgAttributes.SelectedIndices.Count > 0)
            {
                var schemaObject = dgAttributes.Items[dgAttributes.SelectedIndices[0]].Tag as SchemaObjectMappingDto;
                var desc = dgAttributes.Items[dgAttributes.SelectedIndices[0]].Text.Split('-')[0].Trim();
                var obj = new ObjectId();
                var objectId = obj.GetByDescription(desc);
                schemaObject.ObjectId = objectId.ToString();
                var datasource = GetAllObjectIds();
                var form = new SchemaObjectMappingView(schemaObject, datasource);
                var result = form.ShowDialog(this);
                if (form.HasChanges)
                {
                    dgAttributes.Items.RemoveAt(dgAttributes.SelectedIndices[0]);
                    var dto = (SchemaObjectMappingDto)form.DataContext;
                    ObjectId id;
                    if (Enum.TryParse(dto.ObjectId, false, out id))
                    {
                        var listItem = new ListViewItem(id.GetDescription() + " - " + dto.ObjectClass) { Tag = dto };
                        dgAttributes.Items.Add(listItem);
                    }
                }
            }
        }
        private void EnableDisableAdd()
        {
            btnAdd.Enabled = dgAttributes.Items.Count < 4;
        }
        private void EnableDisableRemove()
        {
            btnRemove.Enabled = (dgAttributes.Items.Count > 0 && dgAttributes.SelectedIndices != null && dgAttributes.SelectedIndices.Count > 0);
        }

        private void dgAttributes_SelectedIndexChanged(object sender, EventArgs e)
        {
            EnableDisableRemove();
            EnableDisableAdd();
        }
        private Dictionary<string, string> GetAttributeMap()
        {
            var dictionary = new Dictionary<string, string>();

            for (var index = 0; index < dgAttributeMap.Rows.Count - 1; index++)
            {
                var row = dgAttributeMap.Rows[index];
                dictionary.Add(row.Cells[0].Value.ToString(), row.Cells[1].Value.ToString());
            }
            return dictionary;
        }
        private void SetupAttributeMap()
        {
            dgAttributeMap.Rows.Clear();
            dgAttributeMap.Columns.Clear();
            var attributeColumn = new DataGridViewTextBoxColumn()
            {
                Width = 180,
                HeaderText = "Attribute ID",
            };
            dgAttributeMap.Columns.Add(attributeColumn);

            var valueColumn = new DataGridViewTextBoxColumn()
            {
                Width = 220,
                HeaderText = "Value"
            };
            dgAttributeMap.Columns.Add(valueColumn);
            dgAttributeMap.Refresh();
        }

        private void dgAttributeMapContext_Click(object sender, EventArgs e)
        {
            if (dgAttributeMap.SelectedRows != null && dgAttributeMap.SelectedRows.Count > 0)
            {
                if (dgAttributeMap.SelectedRows[0].Index != dgAttributeMap.Rows.Count - 1)
                {
                    dgAttributeMap.Rows.RemoveAt(dgAttributeMap.SelectedRows[0].Index);
                    dgAttributeMap.Refresh();
                }
            }
        }
    }

    public class IdentityySourceTypeDto
    {
        public string Key { get; set; }
        public string Value { get; set; }
    }
}
