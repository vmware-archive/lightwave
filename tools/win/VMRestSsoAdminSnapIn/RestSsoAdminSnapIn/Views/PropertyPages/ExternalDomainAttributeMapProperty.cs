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
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class ExternalDomainAttributeMapProperty : UserControl
    {
        private GenericPropertyPage _parent;
        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        private IPropertyDataManager _dataMgr;
        private IdentityProviderDto _providerDto;
        public ExternalDomainAttributeMapProperty(IPropertyDataManager mgr, IdentityProviderDto dto)
        {
            _dataMgr = mgr;
            _providerDto = dto;
            InitializeComponent();
            PropertyPageInit();
        }

        void PropertyPageInit()
        {
            _parent = new GenericPropertyPage { Control = this };
            _parent.Apply += new CancelEventHandler(_parent_Apply);
            _parent.Initialize += new EventHandler(_parent_Initialize);
        }

        void _parent_Initialize(object sender, EventArgs e)
        {
            BindControls();
            HookChanges();
        }
        private void HookChanges()
        {
            dgAttributeMap.CellValueChanged += ContentChanged;
        }
        private void ContentChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }
        void _parent_Apply(object sender, CancelEventArgs e)
        {
            if (IsValid())
            {
                var attributes = GetAttributes();
                _providerDto.AttributesMap = new Dictionary<string, string>(attributes);
                e.Cancel = !_dataMgr.Apply(_providerDto);
            }
        }

        private bool IsValid()
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

        private Dictionary<string, string> GetAttributes()
        {
            var dictionary = new Dictionary<string, string>();

            for (var index = 0; index < dgAttributeMap.Rows.Count - 1; index++)
            {
                var row = dgAttributeMap.Rows[index];
                var id = new AttributeId();
                if (Enum.TryParse(row.Cells[0].Value.ToString(), out id))
                {
                    var description = id.GetDescription();
                    dictionary.Add(description, row.Cells[1].Value.ToString());
                }
            }
            return dictionary;
        }
        private void BindControls()
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
            if (_providerDto.AttributesMap != null)
            {
                var rowId = 0;
                foreach (var attribute in _providerDto.AttributesMap)
                {
                    dgAttributeMap.Rows.Add(1);
                    dgAttributeMap.Rows[rowId].Cells[0].Value = attribute.Key;// GetAttribute(attribute.Key);
                    dgAttributeMap.Rows[rowId].Cells[1].Value = attribute.Value;
                    rowId++;
                }
            }
            dgAttributeMap.Refresh();
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
        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (dgAttributeMap.SelectedRows != null && dgAttributeMap.SelectedRows.Count > 0)
            {
                if (dgAttributeMap.SelectedRows[0].Index != dgAttributeMap.Rows.Count - 1)
                {
                    dgAttributeMap.Rows.RemoveAt(dgAttributeMap.SelectedRows[0].Index);
                    Page.Dirty = true;
                    dgAttributeMap.Refresh();
                }
            }
        }
    }
}
