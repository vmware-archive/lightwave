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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class ExternalDomainSchemaObjectMapProperty : UserControl
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
        public ExternalDomainSchemaObjectMapProperty(IPropertyDataManager mgr, IdentityProviderDto dto)
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
        }
        void _parent_Apply(object sender, CancelEventArgs e)
        {
            var attributes = GetAttributes();
            _providerDto.Schema = new Dictionary<string, SchemaObjectMappingDto>(attributes);
            e.Cancel = !_dataMgr.Apply(_providerDto);
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
        private void BindControls()
        {
            dgAttributes.Items.Clear();

            if (_providerDto.Schema != null)
            {
                foreach (var item in _providerDto.Schema)
                {
                    ObjectId id;
                    if (Enum.TryParse(item.Key, false, out id))
                    {
                        var dto = (SchemaObjectMappingDto)item.Value ;
                        dto.ObjectId = item.Key;
                        var listItem = new ListViewItem(id.GetDescription() + " - " + item.Value.ObjectClass) { Tag = dto };
                        dgAttributes.Items.Add(listItem);
                    }
                }
            }
            dgAttributes.Refresh();
            EnableDisableRemove();
            EnableDisableAdd();
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (dgAttributes.SelectedIndices != null && dgAttributes.SelectedIndices.Count > 0)
            {
                dgAttributes.Items.RemoveAt(dgAttributes.SelectedIndices[0]);
                Page.Dirty = true;
                dgAttributes.Refresh();
            }
        }
        private void btnAdd_Click(object sender, EventArgs e)
        {
            var dataSource = GetUnMappedObjectIds();
            var form = new SchemaObjectMappingView(null, dataSource);
            var result = form.ShowDialog(this);
            if (form.HasChanges)
            {
                Page.Dirty = true;
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
        private void EnableDisableAdd()
        {
            btnAdd.Enabled = dgAttributes.Items.Count < 4;
        }
        private void EnableDisableRemove()
        {
            btnRemove.Enabled = (dgAttributes.Items.Count > 0 && dgAttributes.SelectedIndices != null && dgAttributes.SelectedIndices.Count > 0);
        }
        private void btnRemove_Click(object sender, EventArgs e)
        {
            if (dgAttributes.SelectedIndices != null && dgAttributes.SelectedIndices.Count > 0)
            {
                dgAttributes.Items.RemoveAt(dgAttributes.SelectedIndices[0]);
                Page.Dirty = true;
                dgAttributes.Refresh();
                EnableDisableRemove();
                EnableDisableAdd();
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
                var dataSource = GetUnMappedObjectIds();
                var objectIds = GetAllObjectIds();
                var form = new SchemaObjectMappingView(schemaObject, objectIds);
                var result = form.ShowDialog(this);
                if (form.HasChanges)
                {
                    Page.Dirty = true;
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

        private void dgAttributes_SelectedIndexChanged(object sender, EventArgs e)
        {
            EnableDisableRemove();
            EnableDisableAdd();
        }

        private void dgContext_Opening(object sender, CancelEventArgs e)
        {

        }

        private void ExternalDomainSchemaObjectMapProperty_Load(object sender, EventArgs e)
        {

        }
    }
}
