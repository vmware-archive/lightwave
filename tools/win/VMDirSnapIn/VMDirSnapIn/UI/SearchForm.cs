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
        }
        private void searchQueryControl1_Load(object sender, EventArgs e)
        {
            this.searchQueryControl1.SearchButtonClicked += searchQueryControl1_SearchButtonClicked;
            this.searchQueryControl1.BindUI(_searchBase, _serverDTO);
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
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
        }
        void GetPage()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                _serverDTO.Connection.PagedSearch(_qdto, _pageSize, _cookie, _morePages,
                    delegate(ILdapMessage ldMsg, IntPtr ck, bool moreP, List<ILdapEntry> entries)
                    {
                        _cookie = ck;
                        _morePages = moreP;
                        _totalCount += entries.Count();
                        _pageNumber++;
                        foreach (var entry in entries)
                        {
                            _result.Add(new DirectoryNonExpandableNode(entry.getDN(), MiscUtilsService.GetObjectClass(entry), _serverDTO, this.propertiesControl1));
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
            });
        }
        private void searchQueryControl1_SearchButtonClicked(object sender, SearchArgs args)
        {
            if (_serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            //this.searchResultControl1.Visible = true;
            InitPageSearch(args.Qdto);
            if (args.Qdto == null)
                return;
            this.Text = "Server: " + _serverDTO.Server + "           Search In: " + args.Qdto.SearchBase;
            //this.searchResultControl1.ClearData();
            GetPage();
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
            if (this.searchQueryControl1.Visible)
                this.searchQueryControl1.Visible = false;
            else
                this.searchQueryControl1.Visible = true;
        }
        private void toolStripButtonShowHideOperAttr_Click(object sender, EventArgs e)
        {
            var node = this.resultTreeView.SelectedNode as DirectoryNonExpandableNode;
            if (node != null)
            {
                if (node.ServerDTO.OperationalFlag)
                    node.ServerDTO.OperationalFlag = false;
                else
                    node.ServerDTO.OperationalFlag = true;
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
                    if (string.Equals(node.ObjectClass, VMDirConstants.USER_OC))
                    {
                        cmuResultTreeView.Items.Add(tsmiResetUserPassword);
                        cmuResultTreeView.Items.Add(tsmiVerifyUserPassword);
                    }
                    cmuResultTreeView.Show(resultTreeView, p);
                }
            }
        }
        private void tsmiAddToGroup_Click(object sender, EventArgs e)
        {
            var node = this.resultTreeView.SelectedNode as DirectoryNonExpandableNode;
            if (node != null)
                node.AddUserToGroup();
        }

        private void tsmiResetUserPassword_Click(object sender, EventArgs e)
        {
            var node = this.resultTreeView.SelectedNode as DirectoryNonExpandableNode;
            if (node != null)
                node.ResetPassword();
        }

        private void tsmiVerifyUserPassword_Click(object sender, EventArgs e)
        {
            var node = this.resultTreeView.SelectedNode as DirectoryNonExpandableNode;
            if (node != null)
                node.VerifyPassword();
        }

    }
}
