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
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
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
            this.comboBoxAttr.Items.AddRange(attrList.ToArray());
            this.comboBoxAttrToReturn.Items.AddRange(attrList.ToArray());
            this.comboBoxAttrToReturn.SelectedIndex = 0;
            this.comboBoxAttr.SelectedIndex = 0;
            this.comboBoxCond.Items.AddRange(VMDirConstants.ConditionList);
            this.comboBoxCond.SelectedIndex = 0;
            this.comboBoxScope.Items.AddRange(VMDirConstants.ScopeList);
            this.comboBoxScope.SelectedIndex = 2;
            this.comboBoxLogicalOp.Items.AddRange(VMDirConstants.OperatorList);
            this.comboBoxLogicalOp.SelectedIndex = 0;
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
        private bool ValidateReturnAttrAdd()
        {
            if (comboBoxAttrToReturn.SelectedItem == null)
            {
                if (comboBoxAttrToReturn.Items.Contains(comboBoxAttrToReturn.Text))
                {
                    comboBoxAttrToReturn.SelectedIndex = comboBoxAttrToReturn.Items.IndexOf(comboBoxAttrToReturn.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_ATTR);
                    return false;
                }
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

        private QueryDTO GetQuery()
        {
            QueryDTO qdto = null;
            var lst = new HashSet<string>();
            foreach(ListViewItem item in listViewAttrToReturn.Items)
                lst.Add(item.SubItems[0].Text);
            lst.Add(VMDirConstants.ATTR_OBJECT_CLASS);
            lst.Add(VMDirConstants.ATTR_DN);

            if (this.tabControl1.SelectedIndex == 0)
            {
                qdto = new BuildQueryDTO(textBoxBase.Text, (LdapScope)comboBoxScope.SelectedIndex, (LogicalOp)comboBoxLogicalOp.SelectedIndex,
                    GetFiltersList(), lst.ToArray(), 0, IntPtr.Zero, 0);
            }
            else if (this.tabControl1.SelectedIndex == 1)
            {
                qdto = new TextQueryDTO(textBoxBase.Text, (LdapScope)comboBoxScope.SelectedIndex, this.textBoxFilterString.Text,
                    lst.ToArray(), 0, IntPtr.Zero, 0);
            }
            return qdto;
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
                foreach (var item in dto.AttrToReturn)
                {
                    var lvi = new ListViewItem(new string[] { item });
                    listViewAttrToReturn.Items.Add(lvi);
                    this.comboBoxAttrToReturn.SelectedIndex = 0;
                    this.comboBoxAttrToReturn.Items.Remove(item);
                }
            }
            else if (qdto.GetType() == typeof(TextQueryDTO))
            {
                var dto = qdto as TextQueryDTO;
                this.tabControl1.SelectedIndex = 1;
                this.textBoxBase.Text = dto.SearchBase;
                this.comboBoxScope.SelectedIndex = (int)dto.SearchScope;
                this.textBoxFilterString.Text = dto.GetFilterString();
                foreach (var item in dto.AttrToReturn)
                {
                    var lvi = new ListViewItem(new string[] { item });
                    listViewAttrToReturn.Items.Add(lvi);
                    this.comboBoxAttrToReturn.SelectedIndex = 0;
                    this.comboBoxAttrToReturn.Items.Remove(item);
                }
            }
        }

        private void buttonAttrAdd_Click(object sender, EventArgs e)
        {
            if (ValidateReturnAttrAdd())
            {
                var item = comboBoxAttrToReturn.SelectedItem;
                var lvi = new ListViewItem(new string[] { item.ToString() });
                listViewAttrToReturn.Items.Add(lvi);
                this.comboBoxAttrToReturn.SelectedIndex = 0;
                this.comboBoxAttrToReturn.Items.Remove(item);
            }
        }

        private void buttonAttrRemove_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listViewAttrToReturn.SelectedItems)
            {
                this.listViewAttrToReturn.Items.Remove(item);
                this.comboBoxAttrToReturn.Items.Add(item);
            }
        }

        private void buttonAttrRemoveAll_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listViewAttrToReturn.Items)
            {
                this.comboBoxAttrToReturn.Items.Add(item);
            }
            this.listViewAttrToReturn.Items.Clear();
        }

        private void buttonCondAdd_Click(object sender, EventArgs e)
        {
            if (!ValidateAdd())
                return;
            var lvi = new ListViewItem(new string[] { comboBoxAttr.SelectedItem.ToString(), comboBoxCond.SelectedItem.ToString(), textBoxVal.Text });
            listViewConditions.Items.Add(lvi);
        }

        private void buttonCondRemove_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listViewConditions.SelectedItems)
            {
                this.listViewConditions.Items.Remove(item);
            }
        }

        private void buttonCondRemoveAll_Click(object sender, EventArgs e)
        {
            this.listViewConditions.Items.Clear();
        }

        private void buttonCopyFilter_Click(object sender, EventArgs e)
        {
            var query = GetQuery();
            if (query != null)
            {
                textBoxFilterString.Text=query.GetFilterString();
                tabControl1.SelectedIndex = 1;
            }
        }

        private void buttonFromFile_Click(object sender, EventArgs e)
        {
            List<FilterDTO> filters = new List<FilterDTO>();
            var frm = new ConditionsFromFile(filters, attrList);
            if (frm.ShowDialog() == DialogResult.OK)
            {
                foreach (var item in filters)
                {
                    var lvi = new ListViewItem(new string[] { item.Attribute,VMDirConstants.ConditionList[(int)item.Condition], item.Value });
                    listViewConditions.Items.Add(lvi);
                }
            }
        }

        private void buttonSearch_Click(object sender, EventArgs e)
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

        private void contextMenuStrip2_Opening(object sender, CancelEventArgs e)
        {
            if (this.listViewAttrToReturn.SelectedIndices.Count == 0)
                e.Cancel = true;
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listViewAttrToReturn.SelectedItems)
            {
                this.listViewAttrToReturn.Items.Remove(item);
            }
        }
    }
}
