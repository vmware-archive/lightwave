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

using Microsoft.ManagementConsole;
using System;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using VMDir.Common;
using VMDirSnapIn.TreeNodes;
using VMIdentity.CommonUtils.Log;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.Views
{
    public partial class ResultPaneControl : UserControl, IFormViewControl
    {
        ResultPaneFormView resultPaneFormView = null;
        public ResultPaneControl()
        {
            InitializeComponent();
            this.imageList1.Images.AddRange(VMDirEnvironment.Instance.ImageLst.ToArray());
            this.Dock = DockStyle.Fill;
        }

        void IFormViewControl.Initialize(FormView parentResultPaneFormView)
        {
            resultPaneFormView = (ResultPaneFormView)parentResultPaneFormView;
        }

        public void AddDirRootNode()
        {
            var node = new RootNode(this.propertiesControl1);
            this.treeViewExplore.Nodes.Add(node);
            this.treeViewExplore.SelectedNode = node;
        }

        private void treeViewExplore_AfterSelect(object sender, TreeViewEventArgs e)
        {
            var node = e.Node as BaseTreeNode;
            //VMDirEnvironment.Instance.Logger.Log(node.Text+" selected",LogLevel.Info);
            if (node != null)
                node.DoSelect();
        }

        private void treeViewExplore_AfterExpand(object sender, TreeViewEventArgs e)
        {
            var node = e.Node as BaseTreeNode;
            //VMDirEnvironment.Instance.Logger.Log(node.Text + " expanded", LogLevel.Info);
            if (node != null)
                node.DoExpand();
        }

        void treeViewExplore_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                Point p = new Point(e.X, e.Y);
                TreeNode node = treeViewExplore.GetNodeAt(p);
                if (node != null)
                {
                    treeViewExplore.SelectedNode = node;
                    switch (Convert.ToString(node.Tag))
                    {
                        case "root":
                            cmsRootNode.Show(treeViewExplore, p);
                            break;
                        case "server":
                            var sn = treeViewExplore.GetNodeAt(p) as ServerNode;
                            if (sn != null)
                            {
                                cmsServerNode.Items.Clear();
                                if (sn.IsLoggedIn)
                                {
                                    cmsServerNode.Items.Add(tsmiLogout);
                                }
                                else
                                {
                                    cmsServerNode.Items.Add(tsmiLogin);
                                }
                                cmsServerNode.Items.Add(tsmiRemove);
                                cmsServerNode.Items.Add(tsmiServerRefresh);
                                cmsServerNode.Items.Add(tsmiSuperlog);
                                cmsServerNode.Show(treeViewExplore, p);
                            }
                            break;
                        case "directory":
                            var dn = treeViewExplore.GetNodeAt(p) as DirectoryExpandableNode;
                            if (dn != null)
                            {
                                cmsDirectoryNode.Items.Clear();
                                cmsDirectoryNode.Items.Add(tsmiSearch);
                                cmsDirectoryNode.Items.Add(tsmiFetchNextPage);
                                cmsDirectoryNode.Items.Add(tsmiAdd);
                                if (string.Equals(dn.ObjectClass, VMDirConstants.USER_OC))
                                {
                                    cmsDirectoryNode.Items.Add(tsmiPasswordManagement);
                                }
                                cmsDirectoryNode.Items.Add(tsmiRefreshDirectory);
                                cmsDirectoryNode.Items.Add(tsmiDelete);
                                cmsDirectoryNode.Show(treeViewExplore, p);
                            }
                            break;
                    }
                }
            }
        }
        private void tsmiSuperlog_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node != null)
                node.SuperLog();
        }

        private void tsmiAddNewServer_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as RootNode;
            if (node != null)
                node.AddNewServer();
        }

        private void tsmiRootRefresh_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as RootNode;
            if (node != null)
                node.DoRefresh();
        }

        private void tsmiLogin_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node != null)
                node.Login();
        }

        private void tsmiLogout_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node != null)
                node.Logout();
        }

        private void tsmiRemove_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node != null)
                node.RemoveServer();
        }

        private void tsmiServerRefresh_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node != null)
                node.DoRefresh();
        }

        private void tsmiSetPageSize_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node != null)
                node.SetPageSize();
        }

        private void tsmiSearch_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.Search();
        }

        private void tsmiFetchNextPage_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.GetNextPage();
        }

        private void tsmiNewObject_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.AddObject();
        }

        private void tsmiNewUser_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.AddUser();
        }

        private void tsmiNewGroup_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.AddGroup();
        }

        private void tsmiAddToGroup_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.AddUserToGroup();
        }

        private void tsmiResetUserPassword_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.ResetPassword();
        }

        private void tsmiVerifyUserPassword_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.VerifyPassword();
        }

        private void tsmiRefreshDirectory_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.DoRefresh();
        }

        private void tsmiDelete_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if (node != null)
                node.Delete();
        }

        private void toolStripButtonShowHide_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
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

        private void toolStripButtonSetPageSize_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node != null)
            {
                node.SetPageSize();
            }
            else
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_SER_NODE_SEL);
            }
        }
    }
}
