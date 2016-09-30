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
    public partial class AdvancedExternalDomain : Form
    {
        public IdentityProviderDto IdentityProviderDto { get; set; }
        public bool IsAD;

        public AdvancedExternalDomain()
        {
            InitializeComponent();
        }

        private void ViewToDto()
        {
            var attrmap = GetAttributeMap();
            var attributes = GetAttributes();
            IdentityProviderDto.BaseDnForNestedGroupsEnabled = cbDNForNestedGroups.Checked;
            IdentityProviderDto.DirectGroupsSearchEnabled = cbGroupSearch.Checked;
            IdentityProviderDto.MatchingRuleInChainEnabled = cbMatchRuleInChain.Checked;
            IdentityProviderDto.Schema = new Dictionary<string, SchemaObjectMappingDto>(attributes);
            IdentityProviderDto.AttributesMap = new Dictionary<string, string>(attrmap);
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (AtttributeMapIsValid())
            {
                ViewToDto();
                this.Close();
            }
        }

        private void NewExternalDomain_Load(object sender, EventArgs e)
        {
            if (IdentityProviderDto == null)
                IdentityProviderDto = new IdentityProviderDto();
            else
                DtoToView();
            cbDNForNestedGroups.Visible = !IsAD;
            cbGroupSearch.Visible = !IsAD;
            cbMatchRuleInChain.Visible = !IsAD;
            lblSettings.Visible = !IsAD;
        }

        private void DtoToView()
        {
            cbDNForNestedGroups.Checked = IdentityProviderDto.BaseDnForNestedGroupsEnabled;
            cbGroupSearch.Checked = IdentityProviderDto.DirectGroupsSearchEnabled;
            cbMatchRuleInChain.Checked = IdentityProviderDto.MatchingRuleInChainEnabled;
            if (IdentityProviderDto.Schema == null)
            IdentityProviderDto.Schema = new Dictionary<string, SchemaObjectMappingDto>();
            if (IdentityProviderDto.AttributesMap == null)
            IdentityProviderDto.AttributesMap = new Dictionary<string, string>();
            SetAttributeMap();
            SetAttributes();
        }

        private void SetAttributes()
        {
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
            if (IdentityProviderDto.AttributesMap != null)
            {
                var rowId = 0;
                foreach (var attribute in IdentityProviderDto.AttributesMap)
                {
                    dgAttributeMap.Rows.Add(1);
                    dgAttributeMap.Rows[rowId].Cells[0].Value = attribute.Key;// GetAttribute(attribute.Key);
                    dgAttributeMap.Rows[rowId].Cells[1].Value = attribute.Value;
                    rowId++;
                }
            }
            dgAttributeMap.Refresh();
        }

        private void SetAttributeMap()
        {
            dgAttributes.Items.Clear();

            if (IdentityProviderDto.Schema != null)
            {
                foreach (var item in IdentityProviderDto.Schema)
                {
                    ObjectId id;
                    if (Enum.TryParse(item.Key, false, out id))
                    {
                        var dto = (SchemaObjectMappingDto)item.Value;
                        dto.ObjectId = item.Key;
                        var listItem = new ListViewItem(id.GetDescription() + " - " + item.Value.ObjectClass) { Tag = dto };
                        dgAttributes.Items.Add(listItem);
                    }
                }
            }
            dgAttributes.Refresh();
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

        private void btnClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }   
}
