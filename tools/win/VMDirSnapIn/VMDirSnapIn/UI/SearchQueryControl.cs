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
using System.IO;
using System.Linq;
using System.Windows.Forms;
using System.Xml.Serialization;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDirInterop.LDAP;
using VMDirSnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class SearchQueryControl : UserControl
    {
        private string searchBase;
        private List<string> attrList;
        private VMDirServerDTO serverDTO;
        public delegate void SearchClicktHandler(object myObject, SearchArgs args);
        public event SearchClicktHandler SearchButtonClicked;

        QueryDTO qdto;
        public SearchQueryControl()
        {
            InitializeComponent();
        }
        private void ClearUI()
        {
            this.comboBoxAttr.Items.Clear();
            this.comboBoxCond.Items.Clear();
            this.comboBoxScope.Items.Clear();
            this.comboBoxScope2.Items.Clear();
            this.comboBoxLogicalOp.Items.Clear();
        }
        public void BindUI(string searchBase, VMDirServerDTO serverDTO)
        {
            this.searchBase = searchBase;
            this.serverDTO = serverDTO;
            var attrTypes = serverDTO.Connection.SchemaManager.GetAttributeTypeManager();
            attrList=attrTypes.Data.Select(x => x.Key).ToList();

            ClearUI();
            this.textBoxBase.Text = searchBase;
            this.textBoxBase2.Text = searchBase;
            this.comboBoxAttr.Items.AddRange(attrList.ToArray());
            this.comboBoxAttr.SelectedIndex = 0;
            this.comboBoxCond.Items.AddRange(VMDirConstants.ConditionList);
            this.comboBoxCond.SelectedIndex = 0;
            this.comboBoxScope.Items.AddRange(VMDirConstants.ScopeList);
            this.comboBoxScope.SelectedIndex = 0;
            this.comboBoxScope2.Items.AddRange(VMDirConstants.ScopeList);
            this.comboBoxScope2.SelectedIndex = 0;
            this.comboBoxLogicalOp.Items.AddRange(VMDirConstants.OperatorList);
            this.comboBoxLogicalOp.SelectedIndex = 0;
        }
        private void buttonAdd_Click(object sender, EventArgs e)
        {
            if (!ValidateAdd())
                return;
            var lvi = new ListViewItem(new string[] { comboBoxAttr.SelectedItem.ToString(), comboBoxCond.SelectedItem.ToString(), textBoxVal.Text });
            listViewConditions.Items.Add(lvi);
        }
        private bool ValidateAdd()
        {
            if (comboBoxAttr.SelectedItem == null)
            {
                if (comboBoxAttr.Items.Contains(comboBoxAttr.Text))
                {
                    comboBoxAttr.SelectedIndex = comboBoxAttr.Items.IndexOf(comboBoxAttr.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_ATTR);
                    return false;
                }
            }
            if (comboBoxCond.SelectedItem == null)
            {
                if (comboBoxCond.Items.Contains(comboBoxCond.Text))
                {
                    comboBoxCond.SelectedIndex = comboBoxCond.Items.IndexOf(comboBoxCond.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_COND);
                    return false;
                }
            }
            if (string.IsNullOrWhiteSpace(this.textBoxVal.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_VAL);
                return false;
            }
            return true;
        }

        private bool ValidateSearch()
        {
            if (string.IsNullOrWhiteSpace(this.textBoxBase.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_SEARCH_BASE);
                return false;
            }
            if (comboBoxScope.SelectedItem == null)
            {
                if (comboBoxScope.Items.Contains(comboBoxScope.Text))
                {
                    comboBoxScope.SelectedIndex = comboBoxScope.Items.IndexOf(comboBoxScope.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_SEARCH_SCOPE);
                    return false;
                }
            }
            if (comboBoxLogicalOp.SelectedItem == null)
            {
                if (comboBoxLogicalOp.Items.Contains(comboBoxLogicalOp.Text))
                {
                    comboBoxLogicalOp.SelectedIndex = comboBoxLogicalOp.Items.IndexOf(comboBoxLogicalOp.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_VAL);
                    return false;
                }
            }
            if (this.tabControl1.SelectedIndex == 0 && listViewConditions.Items.Count <= 0)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_COND_COUNT);
                return false;
            }
            if (this.tabControl1.SelectedIndex == 1 && string.IsNullOrWhiteSpace(textBoxFilterString.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_TEXT_FILTER);
                return false;
            }
            return true;
        }

        private List<FilterDTO> GetFiltersList()
        {
            List<FilterDTO> filters = new List<FilterDTO>();
            foreach (ListViewItem item in listViewConditions.Items)
            {
                var index = comboBoxCond.Items.IndexOf(item.SubItems[1].Text);
                FilterDTO fdto = new FilterDTO(item.SubItems[0].Text, (Condition)index, item.SubItems[2].Text);
                filters.Add(fdto);
            }
            return filters;
        }

        private void buttonView_Click_1(object sender, EventArgs e)
        {
            if (!ValidateSearch())
                return;
            var qdto = GetQuery();
            if (qdto != null)
                MessageBox.Show(qdto.GetFilterString());
        }
        private void buttonSearch_Click_1(object sender, EventArgs e)
        {
            SearchClicked();
        }
        private void buttonSearch2_Click(object sender, EventArgs e)
        {
            SearchClicked();
        }
        private QueryDTO GetQuery()
        {
            QueryDTO qdto = null;
            if (this.tabControl1.SelectedIndex == 0)
            {
                qdto = new BuildQueryDTO(textBoxBase.Text, (LdapScope)comboBoxScope.SelectedIndex, (LogicalOp)comboBoxLogicalOp.SelectedIndex,
                    GetFiltersList(), new string[] { VMDirConstants.ATTR_DN, VMDirConstants.ATTR_OBJECT_CLASS }, 0, IntPtr.Zero, 0);

            }
            else if (this.tabControl1.SelectedIndex == 1)
            {
                qdto = new TextQueryDTO(textBoxBase2.Text, (LdapScope)comboBoxScope2.SelectedIndex, this.textBoxFilterString.Text,
                    new string[] { VMDirConstants.ATTR_DN, VMDirConstants.ATTR_OBJECT_CLASS }, 0, IntPtr.Zero, 0);
            }
            return qdto;
        }
        private void SearchClicked()
        {
            if (!ValidateSearch())
                return;

            qdto = GetQuery();
            if (SearchButtonClicked != null)
            {
                SearchArgs args = new SearchArgs(qdto);
                SearchButtonClicked(new object(), args);
            }
        }

        private void contextMenuStrip1_Opening_1(object sender, CancelEventArgs e)
        {
            if (this.listViewConditions.SelectedIndices.Count == 0)
                e.Cancel = true;
        }

        private void removeToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listViewConditions.SelectedItems)
            {
                this.listViewConditions.Items.Remove(item);
            }
        }

        public void StoreQuery()
        {
            MiscUtilsService.CheckedExec(delegate
           {
               if (!ValidateSearch())
                   return;
               var qdto = GetQuery();
               if (qdto == null)
                   return;
               MMCMiscUtil.SaveObjectToFile(qdto, "Store Query", MMCUIConstants.XML_FILTER);
           });
        }
        public void LoadQuery()
        {
            MiscUtilsService.CheckedExec(delegate
           {
               using (var sfd = new OpenFileDialog())
               {
                   sfd.Title = "Load Query";
                   sfd.Filter = MMCUIConstants.XML_FILTER;
                   if (sfd.ShowDialog() == DialogResult.OK)
                   {
                       try
                       {
                           qdto = LoadQuryOfType(typeof(BuildQueryDTO), sfd.FileName) as BuildQueryDTO;

                       }
                       catch (Exception )
                       {
                           qdto = LoadQuryOfType(typeof(TextQueryDTO), sfd.FileName) as TextQueryDTO;
                       }
                       BindData();
                   }
               }
           });
        }
        private object LoadQuryOfType(Type ty, string filename)
        {
            using (var ms = new MemoryStream())
            {
                var bytes = File.ReadAllBytes(filename);
                ms.Write(bytes, 0, bytes.Length);
                ms.Seek(0, SeekOrigin.Begin);

                var xmlSerializer = new XmlSerializer(ty);
                return xmlSerializer.Deserialize(ms);
            }
        }
        private void BindData()
        {
            if (qdto.GetType() == typeof(BuildQueryDTO))
            {
                var dto = qdto as BuildQueryDTO;
                this.tabControl1.SelectedIndex = 0;
                this.textBoxBase.Text = dto.SearchBase;
                this.comboBoxScope.SelectedIndex = (int)dto.SearchScope;
                this.comboBoxLogicalOp.SelectedIndex = (int)dto.Operator;
                this.listViewConditions.Items.Clear();
                foreach (var item in dto.CondList)
                {
                    ListViewItem lvi = new ListViewItem(item.Attribute);
                    var cond = this.comboBoxCond.Items[(int)item.Condition].ToString();
                    lvi.SubItems.Add(cond);
                    lvi.SubItems.Add(item.Value);
                    this.listViewConditions.Items.Add(lvi);
                }
            }
            else if (qdto.GetType() == typeof(TextQueryDTO))
            {
                var dto = qdto as TextQueryDTO;
                this.tabControl1.SelectedIndex = 1;
                this.textBoxBase2.Text = dto.SearchBase;
                this.comboBoxScope2.SelectedIndex = (int)dto.SearchScope;
                this.textBoxFilterString.Text = dto.GetFilterString();
            }
        }
    }
}
