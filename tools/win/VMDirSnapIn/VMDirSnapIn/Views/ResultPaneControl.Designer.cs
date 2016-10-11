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

using VMDirSnapIn.UI;
namespace VMDirSnapIn.Views
{
    partial class ResultPaneControl
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ResultPaneControl));
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbSetPageSize = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbFetchNext = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator15 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbShowHideOperationalAttr = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbShowOptionalAttr = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbRefresh = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator6 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbSearch = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator7 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbSuperLog = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator8 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbDelete = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator14 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbAddObject = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator9 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbAddUser = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator10 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbAddGroup = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator11 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbAddToGroup = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator12 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbResetPassword = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator13 = new System.Windows.Forms.ToolStripSeparator();
            this.tsbVerifyPassword = new System.Windows.Forms.ToolStripButton();
            this.treeViewExplore = new System.Windows.Forms.TreeView();
            this.imageList1 = new System.Windows.Forms.ImageList(this.components);
            this.propertiesControl1 = new UI.PropertiesControl();
            this.toolStripButtonResetPwd = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonCheckPwd = new System.Windows.Forms.ToolStripButton();
            this.tsmiSuperlog = new System.Windows.Forms.ToolStripMenuItem();
            this.cmsServerNode = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmiLogin = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiLogout = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiRemove = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiServerRefresh = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiSetPageSize = new System.Windows.Forms.ToolStripMenuItem();
            this.cmsDirectoryNode = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmiSearch = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiFetchNextPage = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiAdd = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiNewObject = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiNewUser = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiNewGroup = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiAddToGroup = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiPasswordManagement = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiResetUserPassword = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiVerifyUserPassword = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiRefreshDirectory = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiAddNewServer = new System.Windows.Forms.ToolStripMenuItem();
            this.cmsRootNode = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tableLayoutPanel1.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.cmsServerNode.SuspendLayout();
            this.cmsDirectoryNode.SuspendLayout();
            this.cmsRootNode.SuspendLayout();
            this.SuspendLayout();
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 75F));
            this.tableLayoutPanel1.Controls.Add(this.toolStrip1, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.treeViewExplore, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.propertiesControl1, 1, 1);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 30F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(600, 600);
            this.tableLayoutPanel1.TabIndex = 6;
            // 
            // toolStrip1
            // 
            this.toolStrip1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.tableLayoutPanel1.SetColumnSpan(this.toolStrip1, 2);
            this.toolStrip1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripSeparator5,
            this.tsbSetPageSize,
            this.toolStripSeparator1,
            this.tsbFetchNext,
            this.toolStripSeparator15,
            this.tsbShowHideOperationalAttr,
            this.toolStripSeparator3,
            this.tsbShowOptionalAttr,
            this.toolStripSeparator4,
            this.tsbRefresh,
            this.toolStripSeparator6,
            this.tsbSearch,
            this.toolStripSeparator7,
            this.tsbSuperLog,
            this.toolStripSeparator8,
            this.tsbDelete,
            this.toolStripSeparator14,
            this.tsbAddObject,
            this.toolStripSeparator9,
            this.tsbAddUser,
            this.toolStripSeparator10,
            this.tsbAddGroup,
            this.toolStripSeparator11,
            this.tsbAddToGroup,
            this.toolStripSeparator12,
            this.tsbResetPassword,
            this.toolStripSeparator13,
            this.tsbVerifyPassword});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(600, 30);
            this.toolStrip1.TabIndex = 9;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // toolStripSeparator5
            // 
            this.toolStripSeparator5.Name = "toolStripSeparator5";
            this.toolStripSeparator5.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbSetPageSize
            // 
            this.tsbSetPageSize.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbSetPageSize.Image = ((System.Drawing.Image)(resources.GetObject("tsbSetPageSize.Image")));
            this.tsbSetPageSize.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbSetPageSize.Name = "tsbSetPageSize";
            this.tsbSetPageSize.Size = new System.Drawing.Size(23, 27);
            this.tsbSetPageSize.Tag = "all";
            this.tsbSetPageSize.Text = "Set Page Size";
            this.tsbSetPageSize.Click += new System.EventHandler(this.tsbSetPageSize_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbFetchNext
            // 
            this.tsbFetchNext.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbFetchNext.Image = ((System.Drawing.Image)(resources.GetObject("tsbFetchNext.Image")));
            this.tsbFetchNext.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbFetchNext.Name = "tsbFetchNext";
            this.tsbFetchNext.Size = new System.Drawing.Size(23, 27);
            this.tsbFetchNext.Tag = "directory";
            this.tsbFetchNext.Text = "Fetch Next Page";
            this.tsbFetchNext.Click += new System.EventHandler(this.tsbFetchNext_Click);
            // 
            // toolStripSeparator15
            // 
            this.toolStripSeparator15.Name = "toolStripSeparator15";
            this.toolStripSeparator15.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbShowHideOperationalAttr
            // 
            this.tsbShowHideOperationalAttr.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbShowHideOperationalAttr.Image = ((System.Drawing.Image)(resources.GetObject("tsbShowHideOperationalAttr.Image")));
            this.tsbShowHideOperationalAttr.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbShowHideOperationalAttr.Name = "tsbShowHideOperationalAttr";
            this.tsbShowHideOperationalAttr.Size = new System.Drawing.Size(23, 27);
            this.tsbShowHideOperationalAttr.Tag = "all";
            this.tsbShowHideOperationalAttr.Text = "Show/Hide Operational Attributes";
            this.tsbShowHideOperationalAttr.Click += new System.EventHandler(this.tsbShowHideOperationalAttr_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbShowOptionalAttr
            // 
            this.tsbShowOptionalAttr.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbShowOptionalAttr.Image = ((System.Drawing.Image)(resources.GetObject("tsbShowOptionalAttr.Image")));
            this.tsbShowOptionalAttr.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbShowOptionalAttr.Name = "tsbShowOptionalAttr";
            this.tsbShowOptionalAttr.Size = new System.Drawing.Size(23, 27);
            this.tsbShowOptionalAttr.Tag = "all";
            this.tsbShowOptionalAttr.Text = "Show/Hide Optional Attribute";
            this.tsbShowOptionalAttr.Click += new System.EventHandler(this.tsbShowOptionalAttr_Click);
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            this.toolStripSeparator4.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbRefresh
            // 
            this.tsbRefresh.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbRefresh.Image = ((System.Drawing.Image)(resources.GetObject("tsbRefresh.Image")));
            this.tsbRefresh.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbRefresh.Name = "tsbRefresh";
            this.tsbRefresh.Size = new System.Drawing.Size(23, 27);
            this.tsbRefresh.Tag = "directory";
            this.tsbRefresh.Text = "Refresh";
            this.tsbRefresh.Click += new System.EventHandler(this.tsbRefresh_Click);
            // 
            // toolStripSeparator6
            // 
            this.toolStripSeparator6.Name = "toolStripSeparator6";
            this.toolStripSeparator6.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbSearch
            // 
            this.tsbSearch.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbSearch.Image = ((System.Drawing.Image)(resources.GetObject("tsbSearch.Image")));
            this.tsbSearch.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbSearch.Name = "tsbSearch";
            this.tsbSearch.Size = new System.Drawing.Size(23, 27);
            this.tsbSearch.Tag = "directory";
            this.tsbSearch.Text = "Search";
            this.tsbSearch.Click += new System.EventHandler(this.tsbSearch_Click);
            // 
            // toolStripSeparator7
            // 
            this.toolStripSeparator7.Name = "toolStripSeparator7";
            this.toolStripSeparator7.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbSuperLog
            // 
            this.tsbSuperLog.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbSuperLog.Image = ((System.Drawing.Image)(resources.GetObject("tsbSuperLog.Image")));
            this.tsbSuperLog.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbSuperLog.Name = "tsbSuperLog";
            this.tsbSuperLog.Size = new System.Drawing.Size(23, 27);
            this.tsbSuperLog.Tag = "all";
            this.tsbSuperLog.Text = "Superlog";
            this.tsbSuperLog.Click += new System.EventHandler(this.tsbSuperLog_Click);
            // 
            // toolStripSeparator8
            // 
            this.toolStripSeparator8.Name = "toolStripSeparator8";
            this.toolStripSeparator8.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbDelete
            // 
            this.tsbDelete.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbDelete.Image = ((System.Drawing.Image)(resources.GetObject("tsbDelete.Image")));
            this.tsbDelete.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbDelete.Name = "tsbDelete";
            this.tsbDelete.Size = new System.Drawing.Size(23, 27);
            this.tsbDelete.Tag = "directory";
            this.tsbDelete.Text = "Delete";
            this.tsbDelete.Click += new System.EventHandler(this.tsbDelete_Click);
            // 
            // toolStripSeparator14
            // 
            this.toolStripSeparator14.Name = "toolStripSeparator14";
            this.toolStripSeparator14.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbAddObject
            // 
            this.tsbAddObject.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbAddObject.Image = ((System.Drawing.Image)(resources.GetObject("tsbAddObject.Image")));
            this.tsbAddObject.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbAddObject.Name = "tsbAddObject";
            this.tsbAddObject.Size = new System.Drawing.Size(23, 27);
            this.tsbAddObject.Tag = "directory";
            this.tsbAddObject.Text = "Add New Object";
            this.tsbAddObject.Click += new System.EventHandler(this.tsbAddObject_Click);
            // 
            // toolStripSeparator9
            // 
            this.toolStripSeparator9.Name = "toolStripSeparator9";
            this.toolStripSeparator9.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbAddUser
            // 
            this.tsbAddUser.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbAddUser.Image = ((System.Drawing.Image)(resources.GetObject("tsbAddUser.Image")));
            this.tsbAddUser.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbAddUser.Name = "tsbAddUser";
            this.tsbAddUser.Size = new System.Drawing.Size(23, 27);
            this.tsbAddUser.Tag = "directory";
            this.tsbAddUser.Text = "Add New User";
            this.tsbAddUser.Click += new System.EventHandler(this.tsbAddUser_Click);
            // 
            // toolStripSeparator10
            // 
            this.toolStripSeparator10.Name = "toolStripSeparator10";
            this.toolStripSeparator10.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbAddGroup
            // 
            this.tsbAddGroup.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbAddGroup.Image = ((System.Drawing.Image)(resources.GetObject("tsbAddGroup.Image")));
            this.tsbAddGroup.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbAddGroup.Name = "tsbAddGroup";
            this.tsbAddGroup.Size = new System.Drawing.Size(23, 27);
            this.tsbAddGroup.Tag = "directory";
            this.tsbAddGroup.Text = "Add New Group";
            this.tsbAddGroup.Click += new System.EventHandler(this.tsbAddGroup_Click);
            // 
            // toolStripSeparator11
            // 
            this.toolStripSeparator11.Name = "toolStripSeparator11";
            this.toolStripSeparator11.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbAddToGroup
            // 
            this.tsbAddToGroup.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbAddToGroup.Image = ((System.Drawing.Image)(resources.GetObject("tsbAddToGroup.Image")));
            this.tsbAddToGroup.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbAddToGroup.Name = "tsbAddToGroup";
            this.tsbAddToGroup.Size = new System.Drawing.Size(23, 27);
            this.tsbAddToGroup.Tag = "directory";
            this.tsbAddToGroup.Text = "Add To Group";
            this.tsbAddToGroup.Click += new System.EventHandler(this.tsbAddToGroup_Click);
            // 
            // toolStripSeparator12
            // 
            this.toolStripSeparator12.Name = "toolStripSeparator12";
            this.toolStripSeparator12.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbResetPassword
            // 
            this.tsbResetPassword.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbResetPassword.Image = ((System.Drawing.Image)(resources.GetObject("tsbResetPassword.Image")));
            this.tsbResetPassword.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbResetPassword.Name = "tsbResetPassword";
            this.tsbResetPassword.Size = new System.Drawing.Size(23, 27);
            this.tsbResetPassword.Tag = "user";
            this.tsbResetPassword.Text = "Reset User Password";
            this.tsbResetPassword.Click += new System.EventHandler(this.tsbResetPassword_Click);
            // 
            // toolStripSeparator13
            // 
            this.toolStripSeparator13.Name = "toolStripSeparator13";
            this.toolStripSeparator13.Size = new System.Drawing.Size(6, 30);
            // 
            // tsbVerifyPassword
            // 
            this.tsbVerifyPassword.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbVerifyPassword.Image = ((System.Drawing.Image)(resources.GetObject("tsbVerifyPassword.Image")));
            this.tsbVerifyPassword.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbVerifyPassword.Name = "tsbVerifyPassword";
            this.tsbVerifyPassword.Size = new System.Drawing.Size(23, 27);
            this.tsbVerifyPassword.Tag = "user";
            this.tsbVerifyPassword.Text = "Verify User Password";
            this.tsbVerifyPassword.Click += new System.EventHandler(this.tsbVerifyPassword_Click);
            // 
            // treeViewExplore
            // 
            this.treeViewExplore.Dock = System.Windows.Forms.DockStyle.Fill;
            this.treeViewExplore.ImageIndex = 0;
            this.treeViewExplore.ImageList = this.imageList1;
            this.treeViewExplore.Location = new System.Drawing.Point(3, 33);
            this.treeViewExplore.Name = "treeViewExplore";
            this.treeViewExplore.SelectedImageIndex = 0;
            this.treeViewExplore.ShowLines = false;
            this.treeViewExplore.Size = new System.Drawing.Size(144, 564);
            this.treeViewExplore.TabIndex = 8;
            this.treeViewExplore.AfterExpand += new System.Windows.Forms.TreeViewEventHandler(this.treeViewExplore_AfterExpand);
            this.treeViewExplore.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeViewExplore_AfterSelect);
            this.treeViewExplore.MouseUp += new System.Windows.Forms.MouseEventHandler(this.treeViewExplore_MouseUp);
            // 
            // imageList1
            // 
            this.imageList1.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
            this.imageList1.ImageSize = new System.Drawing.Size(16, 16);
            this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
            // 
            // propertiesControl1
            // 
            this.propertiesControl1.AutoScroll = true;
            this.propertiesControl1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.propertiesControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propertiesControl1.Location = new System.Drawing.Point(153, 33);
            this.propertiesControl1.Name = "propertiesControl1";
            this.propertiesControl1.Size = new System.Drawing.Size(444, 564);
            this.propertiesControl1.TabIndex = 9;
            // 
            // toolStripButtonResetPwd
            // 
            this.toolStripButtonResetPwd.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonResetPwd.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonResetPwd.Name = "toolStripButtonResetPwd";
            this.toolStripButtonResetPwd.Size = new System.Drawing.Size(23, 17);
            this.toolStripButtonResetPwd.Text = "Reset User Password";
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 20);
            // 
            // toolStripButtonCheckPwd
            // 
            this.toolStripButtonCheckPwd.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCheckPwd.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCheckPwd.Name = "toolStripButtonCheckPwd";
            this.toolStripButtonCheckPwd.Size = new System.Drawing.Size(23, 17);
            this.toolStripButtonCheckPwd.Text = "Verify User Password";
            // 
            // tsmiSuperlog
            // 
            this.tsmiSuperlog.Name = "tsmiSuperlog";
            this.tsmiSuperlog.Size = new System.Drawing.Size(142, 22);
            this.tsmiSuperlog.Text = "Superlog";
            this.tsmiSuperlog.Click += new System.EventHandler(this.tsmiSuperlog_Click);
            // 
            // cmsServerNode
            // 
            this.cmsServerNode.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiLogin,
            this.tsmiLogout,
            this.tsmiRemove,
            this.tsmiServerRefresh,
            this.tsmiSetPageSize,
            this.tsmiSuperlog});
            this.cmsServerNode.Name = "cmsServerNode";
            this.cmsServerNode.Size = new System.Drawing.Size(143, 136);
            // 
            // tsmiLogin
            // 
            this.tsmiLogin.Name = "tsmiLogin";
            this.tsmiLogin.Size = new System.Drawing.Size(142, 22);
            this.tsmiLogin.Text = "Login";
            this.tsmiLogin.Click += new System.EventHandler(this.tsmiLogin_Click);
            // 
            // tsmiLogout
            // 
            this.tsmiLogout.Name = "tsmiLogout";
            this.tsmiLogout.Size = new System.Drawing.Size(142, 22);
            this.tsmiLogout.Text = "Logout";
            this.tsmiLogout.Click += new System.EventHandler(this.tsmiLogout_Click);
            // 
            // tsmiRemove
            // 
            this.tsmiRemove.Name = "tsmiRemove";
            this.tsmiRemove.Size = new System.Drawing.Size(142, 22);
            this.tsmiRemove.Text = "Remove";
            this.tsmiRemove.Click += new System.EventHandler(this.tsmiRemove_Click);
            // 
            // tsmiServerRefresh
            // 
            this.tsmiServerRefresh.Name = "tsmiServerRefresh";
            this.tsmiServerRefresh.Size = new System.Drawing.Size(142, 22);
            this.tsmiServerRefresh.Text = "Refresh";
            this.tsmiServerRefresh.Click += new System.EventHandler(this.tsmiServerRefresh_Click);
            // 
            // tsmiSetPageSize
            // 
            this.tsmiSetPageSize.Name = "tsmiSetPageSize";
            this.tsmiSetPageSize.Size = new System.Drawing.Size(142, 22);
            this.tsmiSetPageSize.Text = "Set Page Size";
            this.tsmiSetPageSize.Click += new System.EventHandler(this.tsmiSetPageSize_Click);
            // 
            // cmsDirectoryNode
            // 
            this.cmsDirectoryNode.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiSearch,
            this.tsmiFetchNextPage,
            this.tsmiAdd,
            this.tsmiPasswordManagement,
            this.tsmiRefreshDirectory,
            this.tsmiDelete});
            this.cmsDirectoryNode.Name = "cmsDirectoryNode";
            this.cmsDirectoryNode.Size = new System.Drawing.Size(199, 136);
            // 
            // tsmiSearch
            // 
            this.tsmiSearch.Name = "tsmiSearch";
            this.tsmiSearch.Size = new System.Drawing.Size(198, 22);
            this.tsmiSearch.Text = "Search";
            this.tsmiSearch.Click += new System.EventHandler(this.tsmiSearch_Click);
            // 
            // tsmiFetchNextPage
            // 
            this.tsmiFetchNextPage.Name = "tsmiFetchNextPage";
            this.tsmiFetchNextPage.Size = new System.Drawing.Size(198, 22);
            this.tsmiFetchNextPage.Text = "Fetch Next Page";
            this.tsmiFetchNextPage.Click += new System.EventHandler(this.tsmiFetchNextPage_Click);
            // 
            // tsmiAdd
            // 
            this.tsmiAdd.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiNewObject,
            this.tsmiNewUser,
            this.tsmiNewGroup,
            this.tsmiAddToGroup});
            this.tsmiAdd.Name = "tsmiAdd";
            this.tsmiAdd.Size = new System.Drawing.Size(198, 22);
            this.tsmiAdd.Text = "Add";
            // 
            // tsmiNewObject
            // 
            this.tsmiNewObject.Name = "tsmiNewObject";
            this.tsmiNewObject.Size = new System.Drawing.Size(149, 22);
            this.tsmiNewObject.Text = "New Object";
            this.tsmiNewObject.Click += new System.EventHandler(this.tsmiNewObject_Click);
            // 
            // tsmiNewUser
            // 
            this.tsmiNewUser.Name = "tsmiNewUser";
            this.tsmiNewUser.Size = new System.Drawing.Size(149, 22);
            this.tsmiNewUser.Text = "New User";
            this.tsmiNewUser.Click += new System.EventHandler(this.tsmiNewUser_Click);
            // 
            // tsmiNewGroup
            // 
            this.tsmiNewGroup.Name = "tsmiNewGroup";
            this.tsmiNewGroup.Size = new System.Drawing.Size(149, 22);
            this.tsmiNewGroup.Text = "New Group";
            this.tsmiNewGroup.Click += new System.EventHandler(this.tsmiNewGroup_Click);
            // 
            // tsmiAddToGroup
            // 
            this.tsmiAddToGroup.Name = "tsmiAddToGroup";
            this.tsmiAddToGroup.Size = new System.Drawing.Size(149, 22);
            this.tsmiAddToGroup.Text = "Add To Group";
            this.tsmiAddToGroup.Click += new System.EventHandler(this.tsmiAddToGroup_Click);
            // 
            // tsmiPasswordManagement
            // 
            this.tsmiPasswordManagement.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiResetUserPassword,
            this.tsmiVerifyUserPassword});
            this.tsmiPasswordManagement.Name = "tsmiPasswordManagement";
            this.tsmiPasswordManagement.Size = new System.Drawing.Size(198, 22);
            this.tsmiPasswordManagement.Text = "Password Management";
            // 
            // tsmiResetUserPassword
            // 
            this.tsmiResetUserPassword.Name = "tsmiResetUserPassword";
            this.tsmiResetUserPassword.Size = new System.Drawing.Size(183, 22);
            this.tsmiResetUserPassword.Text = "Reset User Password";
            this.tsmiResetUserPassword.Click += new System.EventHandler(this.tsmiResetUserPassword_Click);
            // 
            // tsmiVerifyUserPassword
            // 
            this.tsmiVerifyUserPassword.Name = "tsmiVerifyUserPassword";
            this.tsmiVerifyUserPassword.Size = new System.Drawing.Size(183, 22);
            this.tsmiVerifyUserPassword.Text = "Verify User Password";
            this.tsmiVerifyUserPassword.Click += new System.EventHandler(this.tsmiVerifyUserPassword_Click);
            // 
            // tsmiRefreshDirectory
            // 
            this.tsmiRefreshDirectory.Name = "tsmiRefreshDirectory";
            this.tsmiRefreshDirectory.Size = new System.Drawing.Size(198, 22);
            this.tsmiRefreshDirectory.Text = "Refresh";
            this.tsmiRefreshDirectory.Click += new System.EventHandler(this.tsmiRefreshDirectory_Click);
            // 
            // tsmiDelete
            // 
            this.tsmiDelete.Name = "tsmiDelete";
            this.tsmiDelete.Size = new System.Drawing.Size(198, 22);
            this.tsmiDelete.Text = "Delete";
            this.tsmiDelete.Click += new System.EventHandler(this.tsmiDelete_Click);
            // 
            // tsmiAddNewServer
            // 
            this.tsmiAddNewServer.Name = "tsmiAddNewServer";
            this.tsmiAddNewServer.Size = new System.Drawing.Size(158, 22);
            this.tsmiAddNewServer.Text = "Add New Server";
            this.tsmiAddNewServer.Click += new System.EventHandler(this.tsmiAddNewServer_Click);
            // 
            // cmsRootNode
            // 
            this.cmsRootNode.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiAddNewServer});
            this.cmsRootNode.Name = "cmsRootNode";
            this.cmsRootNode.Size = new System.Drawing.Size(159, 48);
            // 
            // ResultPaneControl
            // 
            this.AutoScroll = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.tableLayoutPanel1);
            this.Name = "ResultPaneControl";
            this.Size = new System.Drawing.Size(600, 600);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.cmsServerNode.ResumeLayout(false);
            this.cmsDirectoryNode.ResumeLayout(false);
            this.cmsRootNode.ResumeLayout(false);
            this.ResumeLayout(false);

        }


        #endregion

        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.TreeView treeViewExplore;
        public UI.PropertiesControl propertiesControl1;
        private System.Windows.Forms.ImageList imageList1;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton tsbShowHideOperationalAttr;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripButton toolStripButtonResetPwd;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripButton toolStripButtonCheckPwd;
        private System.Windows.Forms.ToolStripButton tsbSetPageSize;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator5;
        private System.Windows.Forms.ToolStripMenuItem tsmiSuperlog;
        private System.Windows.Forms.ContextMenuStrip cmsServerNode;
        private System.Windows.Forms.ToolStripMenuItem tsmiLogout;
        private System.Windows.Forms.ToolStripMenuItem tsmiLogin;
        private System.Windows.Forms.ToolStripMenuItem tsmiRemove;
        private System.Windows.Forms.ToolStripMenuItem tsmiServerRefresh;
        private System.Windows.Forms.ToolStripMenuItem tsmiSetPageSize;
        private System.Windows.Forms.ContextMenuStrip cmsDirectoryNode;
        private System.Windows.Forms.ToolStripMenuItem tsmiRefreshDirectory;
        private System.Windows.Forms.ToolStripMenuItem tsmiDelete;
        private System.Windows.Forms.ToolStripMenuItem tsmiSearch;
        private System.Windows.Forms.ToolStripMenuItem tsmiFetchNextPage;
        private System.Windows.Forms.ToolStripMenuItem tsmiAdd;
        private System.Windows.Forms.ToolStripMenuItem tsmiNewObject;
        private System.Windows.Forms.ToolStripMenuItem tsmiNewUser;
        private System.Windows.Forms.ToolStripMenuItem tsmiNewGroup;
        private System.Windows.Forms.ToolStripMenuItem tsmiPasswordManagement;
        private System.Windows.Forms.ToolStripMenuItem tsmiResetUserPassword;
        private System.Windows.Forms.ToolStripMenuItem tsmiVerifyUserPassword;
        private System.Windows.Forms.ToolStripMenuItem tsmiAddNewServer;
        private System.Windows.Forms.ContextMenuStrip cmsRootNode;
        private System.Windows.Forms.ToolStripMenuItem tsmiAddToGroup;
        private System.Windows.Forms.ToolStripButton tsbShowOptionalAttr;
        private System.Windows.Forms.ToolStripButton tsbDelete;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator6;
        private System.Windows.Forms.ToolStripButton tsbRefresh;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator7;
        private System.Windows.Forms.ToolStripButton tsbSearch;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator8;
        private System.Windows.Forms.ToolStripButton tsbAddObject;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator9;
        private System.Windows.Forms.ToolStripButton tsbAddUser;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator10;
        private System.Windows.Forms.ToolStripButton tsbAddGroup;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator11;
        private System.Windows.Forms.ToolStripButton tsbAddToGroup;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator12;
        private System.Windows.Forms.ToolStripButton tsbResetPassword;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator13;
        private System.Windows.Forms.ToolStripButton tsbVerifyPassword;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator14;
        private System.Windows.Forms.ToolStripButton tsbFetchNext;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator15;
        private System.Windows.Forms.ToolStripButton tsbSuperLog;
    }
}
