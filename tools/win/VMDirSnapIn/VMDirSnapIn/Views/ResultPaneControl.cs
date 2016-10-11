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

using Microsoft.ManagementConsole;
using System;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using VMDir.Common;
using VMDirSnapIn.TreeNodes;
using VMDirSnapIn.Utilities;
using VMIdentity.CommonUtils;
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
            SetToolBarOptions(e);
        }

        private void SetToolBarOptions(TreeViewEventArgs e)
        {
            foreach(ToolStripItem item in toolStrip1.Items)
            {
                item.Enabled=false;
                if (Convert.ToString(item.Tag) == "all")
                    item.Enabled = true;
            }

             var n1 = e.Node as ServerNode;
             if (n1 != null && n1.ServerDTO.IsLoggedIn)
             {
                 foreach (ToolStripItem item in toolStrip1.Items)
                 {
                     if (Convert.ToString(item.Tag) == "server")
                         item.Enabled = true;
                 }
             }
             var n2 = e.Node as DirectoryExpandableNode;
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
                                if (sn.ServerDTO.IsLoggedIn)
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
                                if (dn.ObjectClass.Contains(VMDirConstants.USER_OC))
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

        private void DoActionOnDirectoryExpandableNode(Action<DirectoryExpandableNode> action)
        {
            var node = this.treeViewExplore.SelectedNode as DirectoryExpandableNode;
            if(node==null || action==null)
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
        private void DoActionOnServerNode(Action<ServerNode> action)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
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
        private void DoActionOnServerNodeWithoutConnCheck(Action<ServerNode> action)
        {
            var node = this.treeViewExplore.SelectedNode as ServerNode;
            if (node == null || action == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_OBJ_NODE_SEL);
                return;
            }
            action(node);
        }
        private void DoActionOnRootNode(Action<RootNode> action)
        {
            var node = this.treeViewExplore.SelectedNode as RootNode;
            if (node == null || action == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_OBJ_NODE_SEL);
                return;
            }
            action(node);
        }

        private void tsmiSuperlog_Click(object sender, EventArgs e)
        {
            DoActionOnServerNode(delegate(ServerNode node) { node.SuperLog(); });
        }
        private void tsmiAddNewServer_Click(object sender, EventArgs e)
        {
            DoActionOnRootNode(delegate(RootNode node) { node.AddNewServer(); });
        }
        private void tsmiRootRefresh_Click(object sender, EventArgs e)
        {
            DoActionOnRootNode(delegate(RootNode node) { node.DoRefresh(); });
        }
        private void tsmiLogin_Click(object sender, EventArgs e)
        {
            DoActionOnServerNodeWithoutConnCheck(delegate(ServerNode node) { node.Login();});
        }
        private void tsmiLogout_Click(object sender, EventArgs e)
        {
            DoActionOnServerNode(delegate(ServerNode node) { node.Logout(); });
        }
        private void tsmiRemove_Click(object sender, EventArgs e)
        {
            DoActionOnServerNodeWithoutConnCheck(delegate(ServerNode node) { node.RemoveServer(); });
        }
        private void tsmiServerRefresh_Click(object sender, EventArgs e)
        {
            DoActionOnServerNode(delegate(ServerNode node) { node.DoRefresh(); });
        }
        private void tsmiSetPageSize_Click(object sender, EventArgs e)
        {
            DoActionOnServerNodeWithoutConnCheck(delegate(ServerNode node) { node.SetPageSize(); });
        }
        private void tsmiSearch_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.Search(); });
        }
        private void tsmiFetchNextPage_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.GetNextPage(); });
        }
        private void tsmiNewObject_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddObject(); });
        }
        private void tsmiNewUser_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddUser(); });
        }
        private void tsmiNewGroup_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddGroup(); });
        }
        private void tsmiAddToGroup_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddUserToGroup(); });
        }
        private void tsmiResetUserPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.ResetPassword(); });
        }
        private void tsmiVerifyUserPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.VerifyPassword(); });
        }
        private void tsmiRefreshDirectory_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.DoRefresh(); });
        }
        private void tsmiDelete_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) {performDelete(node);});
        }

        private void tsbShowHideOperationalAttr_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as BaseTreeNode;
            if (node != null && node.ServerDTO!=null)
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
        private void tsbShowOptionalAttr_Click(object sender, EventArgs e)
        {
            var node = this.treeViewExplore.SelectedNode as BaseTreeNode;
            if (node != null && node.ServerDTO != null)
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

        private void performDelete(DirectoryExpandableNode node)
        {
            MiscUtilsService.CheckedExec(delegate()
            {
                if (!MMCDlgHelper.ShowQuestion(string.Format(CommonConstants.CONFIRM_DELETE, "object", Text)))
                    return;
                node.Delete();
                var parent = node.Parent;
                if (parent != null)
                {
                    parent.Nodes.Remove(node);
                }
            });
        }

        private void tsbSetPageSize_Click(object sender, EventArgs e)
        {
            DoActionOnServerNodeWithoutConnCheck(delegate(ServerNode node) { node.SetPageSize(); });
        }
        private void tsbFetchNext_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.GetNextPage(); });
        }
        private void tsbRefresh_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.DoRefresh(); });
        }
        private void tsbSearch_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node){node.Search();});
        }
        private void tsbSuperLog_Click(object sender, EventArgs e)
        {
            DoActionOnServerNode(delegate(ServerNode node) { node.SuperLog(); });
        }
        private void tsbDelete_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { performDelete(node); });
        }
        private void tsbAddObject_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddObject(); });
        }
        private void tsbAddUser_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddUser(); });
        }
        private void tsbAddGroup_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddGroup(); });
        }
        private void tsbAddToGroup_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.AddUserToGroup(); });
        }
        private void tsbResetPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.ResetPassword(); });
        }
        private void tsbVerifyPassword_Click(object sender, EventArgs e)
        {
            DoActionOnDirectoryExpandableNode(delegate(DirectoryExpandableNode node) { node.VerifyPassword(); });
        }

    }
}
