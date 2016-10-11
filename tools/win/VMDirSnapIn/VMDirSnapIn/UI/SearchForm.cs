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
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirInterop.Interfaces;
using VMDirSnapIn.Utilities;
using VMDirSnapIn.TreeNodes;
using VMwareMMCIDP.UI.Common.Utilities;
using System.Linq;
using VMIdentity.CommonUtils;
using VMDir.Common;
using System.Drawing;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace VMDirSnapIn.UI
{
    public partial class SearchForm : Form
    {
        private string _searchBase;
        private VMDirServerDTO _serverDTO;
        private int _pageSize;
        private IntPtr _cookie = IntPtr.Zero;
        private int _totalCount = 0;
        private int _pageNumber = 1;
        private bool _morePages = false;
        private QueryDTO _qdto;
        private List<DirectoryNonExpandableNode> _result;
        private int _currPage { get; set; }
        private int _totalPage { get; set; }

        private List<string> _returnedAttr;

        private delegate void DelegateWithNode(TreeView tv, TreeNode[] childNames);
        private delegate void DelegateSelectNode(TreeView tv, int index);

        public SearchForm(string searchBase, VMDirServerDTO serverDTO)
        {
            InitializeComponent();
            _searchBase = searchBase;
            _serverDTO = serverDTO;
            _pageSize = VMDirConstants.DEFAULT_PAGE_SIZE;
            _result = new List<DirectoryNonExpandableNode>();
            resultStatusLabel.Text = "";
            tableLayoutPanel2.Visible = false;
            _returnedAttr = new List<string>();
        }
        private void searchQueryControl1_Load(object sender, EventArgs e)
        {
            this.searchQueryControl1.SearchButtonClicked += searchQueryControl1_SearchButtonClicked;
            this.searchQueryControl1.BindUI(_searchBase, _serverDTO);
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            foreach (ToolStripItem item in toolStrip1.Items)
            {
                if (Convert.ToString(item.Tag) != "all")
                    item.Enabled = false;
            }
        }
        void InitPageSearch(QueryDTO q)
        {
            _qdto = q;
            _cookie = IntPtr.Zero;
            _totalCount = 0;
            _pageNumber = 1;
            _morePages = false;
            _result.Clear();
            resultTreeView.Nodes.Clear();
            resultStatusLabel.Text = "";
            _returnedAttr.Clear();
            _returnedAttr.AddRange(q.AttrToReturn);
        }

        private async Task GetPage()
        {
            resultStatusLabel.Text = VMDirConstants.STAT_SR_FETCHING_PG;
            IntPtr _timeout = Marshal.AllocCoTaskMem(sizeof(int));
            Marshal.WriteInt32(_timeout, VMDirConstants.SEARCH_TIMEOUT_IN_SEC);
            try
            {
                _qdto.TimeOut = _timeout;
                _serverDTO.Connection.PagedSearch(_qdto, _pageSize, _cookie, _morePages,
                    delegate(ILdapMessage ldMsg, IntPtr ck, bool moreP, List<ILdapEntry> entries)
                    {
                        _cookie = ck;
                        _morePages = moreP;
                        _totalCount += entries.Count();
                        _pageNumber++;
                        foreach (var entry in entries)
                        {
                            var ocList = new List<string>(entry.getAttributeValues(VMDirConstants.ATTR_OBJECT_CLASS).Select(x=>x.StringValue).ToArray());
                            var node = new DirectoryNonExpandableNode(entry.getDN(), ocList, _serverDTO, this.propertiesControl1);
                            node.NodeProperties = _serverDTO.Connection.GetEntryProperties(entry);
                            _result.Add(node);
                        }
                    });

                _totalPage = _totalCount / _pageSize;
                if (_totalCount % _pageSize > 0)
                    _totalPage++;

                if (_morePages)
                {
                    resultStatusLabel.Text = VMDirConstants.STAT_SR_MORE_PG;
                }
                else
                {
                    resultStatusLabel.Text = VMDirConstants.STAT_SR_NO_MORE_PG;
                }
            }
            catch (Exception e)
            {
                resultStatusLabel.Text = VMDirConstants.STAT_SR_FAILED_PG;
                VMDirEnvironment.Instance.Logger.LogException(e);
                MiscUtilsService.ShowError(e);
            }
            finally
            {
                Marshal.FreeCoTaskMem(_timeout);
            }
        }
        private async void searchQueryControl1_SearchButtonClicked(object sender, SearchArgs args)
        {
            if (_serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            if (args.Qdto == null)
                return;
            
            tableLayoutPanel2.Visible = true;
            tsbExportResult.Enabled = true;
            InitPageSearch(args.Qdto);
            propertiesControl1.ClearView();
            propertiesControl1.SetEditState(false);
            this.Text = "Server: " + _serverDTO.Server + "           Search In: " + args.Qdto.SearchBase;
            await GetPage();
            if (_result.Count > 0)
            {
                resultTreeView.Nodes.AddRange(_result.ToArray());
                _currPage = 1;
            }
            else
            {
                resultStatusLabel.Text = VMDirConstants.STAT_SR_NO_MATCH;
            }

            currPageTextBox.Text = _currPage.ToString();
        }

        private void saveToolStripButton_Click(object sender, EventArgs e)
        {
            this.searchQueryControl1.StoreQuery();
        }

        private void openToolStripButton_Click(object sender, EventArgs e)
        {
            this.searchQueryControl1.LoadQuery();
        }

        private void toolStripButtonShowHide_Click(object sender, EventArgs e)
        {
            if (this.searchQueryControl1.Visible && this.tableLayoutPanel2.Visible)
                this.searchQueryControl1.Visible = false;
            else
                this.searchQueryControl1.Visible = true;
        }
        private void toolStripButtonShowHideOperAttr_Click(object sender, EventArgs e)
        {
            var node = this.resultTreeView.SelectedNode as DirectoryNonExpandableNode;
            if (node != null)
            {
                if (node.ServerDTO.OperationalAttrFlag)
                    node.ServerDTO.OperationalAttrFlag = false;
                else
                    node.ServerDTO.OperationalAttrFlag = true;
                node.DoSelect();
            }
            else
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_OBJ_NODE_SEL);
            }
        }

        private void toolStripButtonSetPage_Click(object sender, EventArgs e)
        {
            var frm = new SetPageSizeForm(_pageSize);
            if (frm.ShowDialog() == DialogResult.OK)
                _pageSize = frm.PageSize;
        }

        private void toolStripButtonFetchNextPage_Click(object sender, EventArgs e)
        {
            if (_serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            if (_morePages)
                GetPage();
            else
                MMCDlgHelper.ShowInformation(VMDirConstants.WRN_NO_MORE_PAGES);
        }

        void resultTreeView_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
        {
            var node = e.Node as DirectoryNonExpandableNode;
            if (node != null)
                node.DoSelect();
            SetToolBarOptions(e);
        }
        private void SetToolBarOptions(TreeViewEventArgs e)
        {
            foreach (ToolStripItem item in toolStrip1.Items)
            {
                item.Enabled = false;
                if (Convert.ToString(item.Tag) == "all")
                    item.Enabled = true;
            }

            var n2 = e.Node as DirectoryNonExpandableNode;
            if (n2 != null && n2.ServerDTO.IsLoggedIn)
            {
                foreach (ToolStripItem item in toolStrip1.Items)
                {
                    if (Convert.ToString(item.Tag) == "directory")
                        item.Enabled = true;
                    else if (Convert.ToString(item.Tag) == "user" && n2.ObjectClass.Contains(VMDirConstants.USER_OC))
                        item.Enabled = true;
                }
            }
        }
        private void PrevButton_Click(object sender, EventArgs e)
        {
            _currPage--;
            if (_currPage >= 1)
            {
                resultTreeView.Nodes.Clear();
                var lst = new List<DirectoryNonExpandableNode>();
                for (int i = (_currPage - 1) * _pageSize; i < _currPage * _pageSize && i < _result.Count; i++)
                {
                    lst.Add(_result[i]);
                }
                resultTreeView.Nodes.AddRange(lst.ToArray());
                currPageTextBox.Text = _currPage.ToString();
            }
            else
            {
                _currPage++;
            }
        }

        private void NextButton_Click(object sender, EventArgs e)
        {
            _currPage++;
            if (_currPage > _totalPage && _morePages)
                GetPage();
            if (_currPage <= _totalPage && _currPage != 0)
            {
                resultTreeView.Nodes.Clear();
                var lst = new List<DirectoryNonExpandableNode>();
                for (int i = (_currPage - 1) * _pageSize; i < _currPage * _pageSize && i < _result.Count; i++)
                {
                    lst.Add(_result[i]);
                }
                resultTreeView.Nodes.AddRange(lst.ToArray());
                currPageTextBox.Text = _currPage.ToString();
            }
            else
            {
                _currPage--;
            }
        }
        void resultTreeView_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                Point p = new Point(e.X, e.Y);
                DirectoryNonExpandableNode node = resultTreeView.GetNodeAt(p) as DirectoryNonExpandableNode;
                if (node != null)
                {
                    resultTreeView.SelectedNode = node;
                    cmuResultTreeView.Items.Clear();
                    cmuResultTreeView.Items.Add(tsmiAddToGroup);
                    cmuResultTreeView.Items.Add(tsmiDelete);
                    cmuResultTreeView.Items.Add(tsmiRefresh);
                    if (node.ObjectClass.Contains(VMDirConstants.USER_OC))
                    {
                        cmuResultTreeView.Items.Add(tsmiResetUserPassword);
                        cmuResultTreeView.Items.Add(tsmiVerifyUserPassword);
                    }
                    cmuResultTreeView.Show(resultTreeView, p);
                }
            }
        }

        private void DoActionOnDirectoryNonExpandableNode(Action<DirectoryNonExpandableNode> action)
        {
            var node = this.resultTreeView.SelectedNode as DirectoryNonExpandableNode;
            if (node == null || action == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_OBJ_NODE_SEL);
                return;
            }
            if (node.ServerDTO == null || node.ServerDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            action(node);
        }

        private void tsmiAddToGroup_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.AddUserToGroup(); });
        }
        private void tsmiResetUserPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.ResetPassword(); });
        }
        private void tsmiVerifyUserPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.VerifyPassword(); });
        }

        private void toolStripButtonShowHideOptionalAttr_Click(object sender, EventArgs e)
        {
            var node = this.resultTreeView.SelectedNode as DirectoryNonExpandableNode;
            if (node != null)
            {
                if (node.ServerDTO.OptionalAttrFlag)
                    node.ServerDTO.OptionalAttrFlag = false;
                else
                    node.ServerDTO.OptionalAttrFlag = true;
                node.DoSelect();
            }
            else
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_OBJ_NODE_SEL);
            }
        }

        private void performDelete(DirectoryNonExpandableNode node)
        {
            MiscUtilsService.CheckedExec(delegate()
            {
                if (!MMCDlgHelper.ShowQuestion(string.Format(CommonConstants.CONFIRM_DELETE, "object", Text)))
                    return;
                node.Delete();
                this.resultTreeView.Nodes.Remove(node);
                if (_result != null)
                {
                    _result.Remove(node);
                }
            });
        } 
        private void tsmiDelete_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { performDelete(node); });
        }

        private void tsbRefresh_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.RefreshProperties(); });
        }

        private void tsbDelete_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { performDelete(node); });
        }
        private void tsbAddToGroup_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.AddUserToGroup(); });
        }
        private void tsbResetPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.ResetPassword(); });
        }
        private void tsbVerifyPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.VerifyPassword(); });
        }
        private void tsbExportResult_Click(object sender, EventArgs e)
        {
            if (_serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            MiscUtilsService.CheckedExec(delegate
            {
                if (_result != null && _result.Count > 0)
                {
                    var attrTypes = _serverDTO.Connection.SchemaManager.GetAttributeTypeManager();
                    var attrList = attrTypes.Data.Select(x => x.Key).ToList();
                    var frm = new ExportResult(_result, _returnedAttr, _currPage, _pageSize);
                    frm.ShowDialog();
                }
                else
                {
                    MMCDlgHelper.ShowWarning("There is no result to export.");
                }
            });
        }

        private void tsmiRefresh_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryNonExpandableNode(delegate(DirectoryNonExpandableNode node) { node.RefreshProperties(); });
        }

    }
}
