/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using VMDirSnapIn.Services;
using VMDirSnapIn.Utilities;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.UI
{
    public partial class SchemaBrowser : Form
    {
        string _currentObject = "";
        VMDirServerDTO _serverDTO;
        List<string> _classList;
        List<AttributeTypeDTO> _requiredAttributesList, _optionalAttributesList;
        ContentRuleDTO _contentRule;

        bool[] _needRefresh = { true, true, true, true, true, true };

        public SchemaBrowser(VMDirServerDTO dto)
        {
            _serverDTO = dto;

            InitializeComponent();

            Bind();
        }

        void Bind()
        {
            if (_serverDTO.Connection == null)
            {
                _serverDTO.Connection = new LdapConnectionService(_serverDTO.Server, _serverDTO.BindDN, _serverDTO.Password);
                _serverDTO.Connection.CreateConnection();
            }

            _classList = _serverDTO.Connection.SchemaManager.GetObjectClassManager().Data.Select(x => x.Key).ToList();
            lstObjectClasses.VirtualListSize = _classList.Count;
            _classList.Sort();
        }

        private void lstObjectClasses_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            e.Item = new ListViewItem(_classList[e.ItemIndex]);
        }

        private void tabProperties_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshTabs();
        }

        void RefreshTabs()
        {
            if (_needRefresh[tabProperties.SelectedIndex])
            {
                switch (tabProperties.SelectedIndex)
                {
                    case 0:
                        RefreshHeirarchy();
                        break;
                    case 1:
                        RefreshRequiredAttributes();
                        break;
                    case 2:
                        RefreshOptionalAttributes();
                        break;
                    case 3:
                        RefreshDitContentRules();
                        break;
                }
            }
        }

        void RefreshHeirarchy()
        {
            treeHeirarchy.Nodes.Clear();
            if (lstObjectClasses.SelectedIndices.Count > 0)
            {
                var node = treeHeirarchy.Nodes.Add(_currentObject);
                var mgr = _serverDTO.Connection.SchemaManager;
                var dto = mgr.GetObjectClass(_currentObject);
                while (dto != null)
                {
                    dto = mgr.GetObjectClass(dto.SuperClass);
                    if (dto != null)
                        node = node.Nodes.Add(dto.Name);
                }

                treeHeirarchy.ExpandAll();
            }
        }

        void RefreshRequiredAttributes()
        {
            lstRequiredAttrs.VirtualListSize = 0;
            if (lstObjectClasses.SelectedIndices.Count > 0)
            {
                _requiredAttributesList = _serverDTO.Connection.SchemaManager.GetRequiredAttributes(_currentObject);
                _requiredAttributesList.Sort(LdapTypesService.AttributeTypeSort);
                lstRequiredAttrs.VirtualListSize = _requiredAttributesList.Count;
            }
        }

        void RefreshOptionalAttributes()
        {
            lstOptionalAttrs.VirtualListSize = 0;
            if (lstObjectClasses.SelectedIndices.Count > 0)
            {
                _optionalAttributesList = _serverDTO.Connection.SchemaManager.GetOptionalAttributes(_currentObject);
                _optionalAttributesList.Sort(LdapTypesService.AttributeTypeSort);
                lstOptionalAttrs.VirtualListSize = _optionalAttributesList.Count;
            }
        }

        private void lstObjectClasses_SelectedIndexChanged(object sender, EventArgs e)
        {
            _contentRule = null;
            _currentObject = string.Empty;
            if (lstObjectClasses.SelectedIndices.Count > 0)
                _currentObject = _classList[lstObjectClasses.SelectedIndices[0]];
            for (int i = 0; i < 6; ++i)
                _needRefresh[i] = true;
            RefreshTabs();
        }

        private void lstRequiredAttrs_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            var val = _requiredAttributesList[e.ItemIndex];
            e.Item = new ListViewItem(new string[] { val.Name, val.SyntaxName, val.Description });
        }

        private void lstOptionalAttrs_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            var val = _optionalAttributesList[e.ItemIndex];
            e.Item = new ListViewItem(new string[] { val.Name, val.SyntaxName, val.Description });
        }

        private void tabContentRules_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshDitContentRules();
        }

        void RefreshDitContentRules()
        {
            if (!_needRefresh[2 + tabContentRules.SelectedIndex]) return;

            if (!FillContentRule())
            {
                lstContentAux.VirtualListSize = lstContentMay.VirtualListSize = lstContentMust.VirtualListSize = 0;
                return;
            }

            switch (tabContentRules.SelectedIndex)
            {
                case 0:
                    RefreshDitContentRulesAux();
                    break;
                case 1:
                    RefreshDitContentRulesMust();
                    break;
                case 2:
                    RefreshDitContentRulesMay();
                    break;
            }
        }

        bool FillContentRule()
        {
            if (_contentRule == null)
                _contentRule = _serverDTO.Connection.SchemaManager.GetContentRule(_currentObject);
            return _contentRule != null;
        }

        void RefreshDitContentRulesAux()
        {
            lstContentAux.VirtualListSize = _contentRule.Aux == null ? 0 : _contentRule.Aux.Count;
        }

        void RefreshDitContentRulesMust()
        {
            lstContentMust.VirtualListSize = _contentRule.Must == null ? 0 : _contentRule.Must.Count;
        }

        void RefreshDitContentRulesMay()
        {
            lstContentMay.VirtualListSize = _contentRule.May == null ? 0 : _contentRule.May.Count;
        }

        private void lstContentAux_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            var val = _contentRule.Aux[e.ItemIndex];
            e.Item = new ListViewItem(new string[] { val, "", "" });
        }

        ListViewItem GetItem(string val)
        {
            string syntaxName = "", description = "";
            var attributeDTO = _serverDTO.Connection.SchemaManager.GetAttributeType(val);
            if (attributeDTO != null)
            {
                syntaxName = attributeDTO.SyntaxName;
                description = attributeDTO.Description;
            }
            return new ListViewItem(new string[] { val, syntaxName, description });
        }

        private void lstContentMust_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            var val = _contentRule.Must[e.ItemIndex];
            e.Item = GetItem(val);
        }

        private void lstContentMay_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            var val = _contentRule.May[e.ItemIndex];
            e.Item = GetItem(val);
        }

        private void lstObjectClasses_OnKeyPress(object sender, KeyPressEventArgs e)
        {
            SearchListByKeyPress.findAndSelect((ListView)sender, e.KeyChar);
        }

        private void lstContentAux_KeyPress(object sender, KeyPressEventArgs e)
        {
            SearchListByKeyPress.findAndSelect((ListView)sender, e.KeyChar);
        }

        private void lstOptionalAttrs_KeyPress(object sender, KeyPressEventArgs e)
        {
            SearchListByKeyPress.findAndSelect((ListView)sender, e.KeyChar);
        }

        private void lstRequiredAttrs_KeyPress(object sender, KeyPressEventArgs e)
        {
            SearchListByKeyPress.findAndSelect((ListView)sender, e.KeyChar);
        }

    }
}
