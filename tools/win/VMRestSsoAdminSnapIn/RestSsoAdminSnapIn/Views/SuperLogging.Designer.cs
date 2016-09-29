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
 
namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    partial class SuperLogging
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SuperLogging));
            this.btnStatus = new System.Windows.Forms.Button();
            this.listView1 = new System.Windows.Forms.ListView();
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabDetailsView = new System.Windows.Forms.TabControl();
            this.tabGeneral = new System.Windows.Forms.TabPage();
            this.lblLevel = new System.Windows.Forms.Label();
            this.txtProvider = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.txtAccount = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.txtDuration = new System.Windows.Forms.TextBox();
            this.txtStart = new System.Windows.Forms.TextBox();
            this.txtCorrelationId = new System.Windows.Forms.TextBox();
            this.txtEventType = new System.Windows.Forms.TextBox();
            this.lblStatus = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.tabDetail = new System.Windows.Forms.TabPage();
            this.pnlFriendlyView = new System.Windows.Forms.Panel();
            this.webBrowser = new System.Windows.Forms.WebBrowser();
            this.pnlJsonView = new System.Windows.Forms.Panel();
            this.txtJsonRaw = new System.Windows.Forms.TextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.rdoFriendlyView = new System.Windows.Forms.RadioButton();
            this.rdoJsonView = new System.Windows.Forms.RadioButton();
            this.autoRefresh = new System.Windows.Forms.Timer(this.components);
            this.lblStatusMessage = new System.Windows.Forms.Label();
            this.lblCapture = new System.Windows.Forms.Label();
            this.btnAddFilter = new System.Windows.Forms.Button();
            this.chkAutoRefresh = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.btnImport = new System.Windows.Forms.Button();
            this.btnExport = new System.Windows.Forms.Button();
            this.btnClear = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.lastRefreshTime = new System.Windows.Forms.Label();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.txtEventsToCapture = new System.Windows.Forms.NumericUpDown();
            this.recordCount = new System.Windows.Forms.Label();
            this.pnlSuperLoggingStatus = new System.Windows.Forms.Panel();
            this.refreshInterval = new System.Windows.Forms.ComboBox();
            this.lblFilter = new System.Windows.Forms.Label();
            this.lblFileName = new System.Windows.Forms.Label();
            this.pnlRefresh = new System.Windows.Forms.Panel();
            this.pnlImportStatus = new System.Windows.Forms.Panel();
            this.lblImportStatus = new System.Windows.Forms.Label();
            this.lblFile = new System.Windows.Forms.Label();
            this.tabDetailsView.SuspendLayout();
            this.tabGeneral.SuspendLayout();
            this.tabDetail.SuspendLayout();
            this.pnlFriendlyView.SuspendLayout();
            this.pnlJsonView.SuspendLayout();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtEventsToCapture)).BeginInit();
            this.pnlSuperLoggingStatus.SuspendLayout();
            this.pnlRefresh.SuspendLayout();
            this.pnlImportStatus.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnStatus
            // 
            this.btnStatus.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnStatus.Location = new System.Drawing.Point(703, 6);
            this.btnStatus.Name = "btnStatus";
            this.btnStatus.Size = new System.Drawing.Size(59, 23);
            this.btnStatus.TabIndex = 0;
            this.btnStatus.Text = "ON";
            this.btnStatus.UseVisualStyleBackColor = true;
            this.btnStatus.Click += new System.EventHandler(this.btnOn_Click);
            // 
            // listView1
            // 
            this.listView1.AllowColumnReorder = true;
            this.listView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader6,
            this.columnHeader7,
            this.columnHeader5});
            this.listView1.FullRowSelect = true;
            this.listView1.GridLines = true;
            this.listView1.Location = new System.Drawing.Point(0, 0);
            this.listView1.MultiSelect = false;
            this.listView1.Name = "listView1";
            this.listView1.ShowItemToolTips = true;
            this.listView1.Size = new System.Drawing.Size(863, 143);
            this.listView1.TabIndex = 3;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Level";
            this.columnHeader3.Width = 50;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Date and Time";
            this.columnHeader4.Width = 121;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Event Type";
            this.columnHeader1.Width = 115;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Correlation ID";
            this.columnHeader2.Width = 180;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Account";
            this.columnHeader6.Width = 152;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Provider";
            this.columnHeader7.Width = 137;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Duration (ms)";
            this.columnHeader5.Width = 83;
            // 
            // tabDetailsView
            // 
            this.tabDetailsView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabDetailsView.Controls.Add(this.tabGeneral);
            this.tabDetailsView.Controls.Add(this.tabDetail);
            this.tabDetailsView.Location = new System.Drawing.Point(30, 15);
            this.tabDetailsView.Name = "tabDetailsView";
            this.tabDetailsView.SelectedIndex = 0;
            this.tabDetailsView.Size = new System.Drawing.Size(806, 286);
            this.tabDetailsView.TabIndex = 4;
            // 
            // tabGeneral
            // 
            this.tabGeneral.BackColor = System.Drawing.SystemColors.Control;
            this.tabGeneral.Controls.Add(this.lblLevel);
            this.tabGeneral.Controls.Add(this.txtProvider);
            this.tabGeneral.Controls.Add(this.label10);
            this.tabGeneral.Controls.Add(this.txtAccount);
            this.tabGeneral.Controls.Add(this.label8);
            this.tabGeneral.Controls.Add(this.label9);
            this.tabGeneral.Controls.Add(this.txtDuration);
            this.tabGeneral.Controls.Add(this.txtStart);
            this.tabGeneral.Controls.Add(this.txtCorrelationId);
            this.tabGeneral.Controls.Add(this.txtEventType);
            this.tabGeneral.Controls.Add(this.lblStatus);
            this.tabGeneral.Controls.Add(this.label7);
            this.tabGeneral.Controls.Add(this.label6);
            this.tabGeneral.Controls.Add(this.label5);
            this.tabGeneral.Controls.Add(this.label4);
            this.tabGeneral.Location = new System.Drawing.Point(4, 22);
            this.tabGeneral.Name = "tabGeneral";
            this.tabGeneral.Padding = new System.Windows.Forms.Padding(3);
            this.tabGeneral.Size = new System.Drawing.Size(798, 260);
            this.tabGeneral.TabIndex = 0;
            this.tabGeneral.Text = "General";
            // 
            // lblLevel
            // 
            this.lblLevel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblLevel.AutoSize = true;
            this.lblLevel.BackColor = System.Drawing.Color.Lime;
            this.lblLevel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lblLevel.Location = new System.Drawing.Point(698, 17);
            this.lblLevel.Name = "lblLevel";
            this.lblLevel.Size = new System.Drawing.Size(12, 15);
            this.lblLevel.TabIndex = 33;
            this.lblLevel.Text = " ";
            // 
            // txtProvider
            // 
            this.txtProvider.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtProvider.Location = new System.Drawing.Point(133, 140);
            this.txtProvider.Name = "txtProvider";
            this.txtProvider.ReadOnly = true;
            this.txtProvider.Size = new System.Drawing.Size(447, 13);
            this.txtProvider.TabIndex = 32;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(44, 140);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(49, 13);
            this.label10.TabIndex = 31;
            this.label10.Text = "Provider:";
            // 
            // txtAccount
            // 
            this.txtAccount.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtAccount.Location = new System.Drawing.Point(133, 106);
            this.txtAccount.Name = "txtAccount";
            this.txtAccount.ReadOnly = true;
            this.txtAccount.Size = new System.Drawing.Size(447, 13);
            this.txtAccount.TabIndex = 30;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(44, 105);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(50, 13);
            this.label8.TabIndex = 29;
            this.label8.Text = "Account:";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(175, 212);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(63, 13);
            this.label9.TabIndex = 28;
            this.label9.Text = "milliseconds";
            // 
            // txtDuration
            // 
            this.txtDuration.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtDuration.Location = new System.Drawing.Point(133, 212);
            this.txtDuration.Name = "txtDuration";
            this.txtDuration.ReadOnly = true;
            this.txtDuration.Size = new System.Drawing.Size(39, 13);
            this.txtDuration.TabIndex = 27;
            // 
            // txtStart
            // 
            this.txtStart.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtStart.Location = new System.Drawing.Point(133, 175);
            this.txtStart.Name = "txtStart";
            this.txtStart.ReadOnly = true;
            this.txtStart.Size = new System.Drawing.Size(209, 13);
            this.txtStart.TabIndex = 26;
            // 
            // txtCorrelationId
            // 
            this.txtCorrelationId.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtCorrelationId.Location = new System.Drawing.Point(133, 73);
            this.txtCorrelationId.Name = "txtCorrelationId";
            this.txtCorrelationId.ReadOnly = true;
            this.txtCorrelationId.Size = new System.Drawing.Size(447, 13);
            this.txtCorrelationId.TabIndex = 25;
            // 
            // txtEventType
            // 
            this.txtEventType.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtEventType.Location = new System.Drawing.Point(133, 40);
            this.txtEventType.Name = "txtEventType";
            this.txtEventType.ReadOnly = true;
            this.txtEventType.Size = new System.Drawing.Size(447, 13);
            this.txtEventType.TabIndex = 24;
            // 
            // lblStatus
            // 
            this.lblStatus.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblStatus.AutoSize = true;
            this.lblStatus.ForeColor = System.Drawing.Color.Black;
            this.lblStatus.Location = new System.Drawing.Point(712, 19);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(57, 13);
            this.lblStatus.TabIndex = 23;
            this.lblStatus.Text = "SUCCESS";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(44, 212);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(50, 13);
            this.label7.TabIndex = 22;
            this.label7.Text = "Duration:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(44, 175);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(80, 13);
            this.label6.TabIndex = 21;
            this.label6.Text = "Date and Time:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(44, 72);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(74, 13);
            this.label5.TabIndex = 20;
            this.label5.Text = "Correlation ID:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(44, 38);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(65, 13);
            this.label4.TabIndex = 19;
            this.label4.Text = "Event Type:";
            // 
            // tabDetail
            // 
            this.tabDetail.BackColor = System.Drawing.SystemColors.Control;
            this.tabDetail.Controls.Add(this.pnlFriendlyView);
            this.tabDetail.Controls.Add(this.pnlJsonView);
            this.tabDetail.Controls.Add(this.panel1);
            this.tabDetail.Location = new System.Drawing.Point(4, 22);
            this.tabDetail.Name = "tabDetail";
            this.tabDetail.Padding = new System.Windows.Forms.Padding(3);
            this.tabDetail.Size = new System.Drawing.Size(798, 260);
            this.tabDetail.TabIndex = 1;
            this.tabDetail.Text = "Details";
            // 
            // pnlFriendlyView
            // 
            this.pnlFriendlyView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlFriendlyView.Controls.Add(this.webBrowser);
            this.pnlFriendlyView.Location = new System.Drawing.Point(13, 44);
            this.pnlFriendlyView.Name = "pnlFriendlyView";
            this.pnlFriendlyView.Size = new System.Drawing.Size(760, 202);
            this.pnlFriendlyView.TabIndex = 4;
            // 
            // webBrowser
            // 
            this.webBrowser.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.webBrowser.Location = new System.Drawing.Point(32, 10);
            this.webBrowser.MinimumSize = new System.Drawing.Size(20, 20);
            this.webBrowser.Name = "webBrowser";
            this.webBrowser.Size = new System.Drawing.Size(698, 173);
            this.webBrowser.TabIndex = 1;
            // 
            // pnlJsonView
            // 
            this.pnlJsonView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlJsonView.Controls.Add(this.txtJsonRaw);
            this.pnlJsonView.Location = new System.Drawing.Point(10, 42);
            this.pnlJsonView.Name = "pnlJsonView";
            this.pnlJsonView.Size = new System.Drawing.Size(768, 218);
            this.pnlJsonView.TabIndex = 3;
            // 
            // txtJsonRaw
            // 
            this.txtJsonRaw.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtJsonRaw.Location = new System.Drawing.Point(20, 9);
            this.txtJsonRaw.Multiline = true;
            this.txtJsonRaw.Name = "txtJsonRaw";
            this.txtJsonRaw.ReadOnly = true;
            this.txtJsonRaw.Size = new System.Drawing.Size(715, 195);
            this.txtJsonRaw.TabIndex = 0;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.rdoFriendlyView);
            this.panel1.Controls.Add(this.rdoJsonView);
            this.panel1.Location = new System.Drawing.Point(30, 15);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(268, 28);
            this.panel1.TabIndex = 2;
            // 
            // rdoFriendlyView
            // 
            this.rdoFriendlyView.AutoSize = true;
            this.rdoFriendlyView.Location = new System.Drawing.Point(20, 9);
            this.rdoFriendlyView.Name = "rdoFriendlyView";
            this.rdoFriendlyView.Size = new System.Drawing.Size(87, 17);
            this.rdoFriendlyView.TabIndex = 0;
            this.rdoFriendlyView.Text = "Friendly View";
            this.rdoFriendlyView.UseVisualStyleBackColor = true;
            this.rdoFriendlyView.CheckedChanged += new System.EventHandler(this.rdoFriendlyView_CheckedChanged);
            // 
            // rdoJsonView
            // 
            this.rdoJsonView.AutoSize = true;
            this.rdoJsonView.Checked = true;
            this.rdoJsonView.Location = new System.Drawing.Point(132, 9);
            this.rdoJsonView.Name = "rdoJsonView";
            this.rdoJsonView.Size = new System.Drawing.Size(73, 17);
            this.rdoJsonView.TabIndex = 1;
            this.rdoJsonView.TabStop = true;
            this.rdoJsonView.Text = "Json View";
            this.rdoJsonView.UseVisualStyleBackColor = true;
            this.rdoJsonView.CheckedChanged += new System.EventHandler(this.btnJsonView_CheckedChanged);
            // 
            // autoRefresh
            // 
            this.autoRefresh.Tick += new System.EventHandler(this.autoRefresh_Tick);
            // 
            // lblStatusMessage
            // 
            this.lblStatusMessage.AutoSize = true;
            this.lblStatusMessage.Location = new System.Drawing.Point(16, 11);
            this.lblStatusMessage.Name = "lblStatusMessage";
            this.lblStatusMessage.Size = new System.Drawing.Size(0, 13);
            this.lblStatusMessage.TabIndex = 8;
            // 
            // lblCapture
            // 
            this.lblCapture.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblCapture.AutoSize = true;
            this.lblCapture.Location = new System.Drawing.Point(529, 10);
            this.lblCapture.Name = "lblCapture";
            this.lblCapture.Size = new System.Drawing.Size(95, 13);
            this.lblCapture.TabIndex = 9;
            this.lblCapture.Text = "Events to Capture:";
            // 
            // btnAddFilter
            // 
            this.btnAddFilter.Location = new System.Drawing.Point(203, 123);
            this.btnAddFilter.Name = "btnAddFilter";
            this.btnAddFilter.Size = new System.Drawing.Size(116, 23);
            this.btnAddFilter.TabIndex = 10;
            this.btnAddFilter.Text = "Filter Current Log...";
            this.btnAddFilter.UseVisualStyleBackColor = true;
            this.btnAddFilter.Click += new System.EventHandler(this.btnAddFilter_Click);
            // 
            // chkAutoRefresh
            // 
            this.chkAutoRefresh.AutoSize = true;
            this.chkAutoRefresh.Location = new System.Drawing.Point(104, 15);
            this.chkAutoRefresh.Name = "chkAutoRefresh";
            this.chkAutoRefresh.Size = new System.Drawing.Size(109, 17);
            this.chkAutoRefresh.TabIndex = 11;
            this.chkAutoRefresh.Text = "Auto refresh evey";
            this.chkAutoRefresh.UseVisualStyleBackColor = true;
            this.chkAutoRefresh.CheckedChanged += new System.EventHandler(this.chkAutoRefresh_CheckedChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(264, 18);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(47, 13);
            this.label1.TabIndex = 12;
            this.label1.Text = "seconds";
            // 
            // btnRefresh
            // 
            this.btnRefresh.Location = new System.Drawing.Point(20, 10);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(72, 23);
            this.btnRefresh.TabIndex = 13;
            this.btnRefresh.Text = "Refresh";
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // btnImport
            // 
            this.btnImport.Location = new System.Drawing.Point(43, 122);
            this.btnImport.Name = "btnImport";
            this.btnImport.Size = new System.Drawing.Size(72, 23);
            this.btnImport.TabIndex = 14;
            this.btnImport.Text = "Import";
            this.btnImport.UseVisualStyleBackColor = true;
            this.btnImport.Click += new System.EventHandler(this.btnImport_Click);
            // 
            // btnExport
            // 
            this.btnExport.Location = new System.Drawing.Point(124, 122);
            this.btnExport.Name = "btnExport";
            this.btnExport.Size = new System.Drawing.Size(72, 23);
            this.btnExport.TabIndex = 15;
            this.btnExport.Text = "Export";
            this.btnExport.UseVisualStyleBackColor = true;
            this.btnExport.Click += new System.EventHandler(this.btnExport_Click);
            // 
            // btnClear
            // 
            this.btnClear.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnClear.Location = new System.Drawing.Point(769, 6);
            this.btnClear.Name = "btnClear";
            this.btnClear.Size = new System.Drawing.Size(72, 23);
            this.btnClear.TabIndex = 16;
            this.btnClear.Text = "Clear";
            this.btnClear.UseVisualStyleBackColor = true;
            this.btnClear.Click += new System.EventHandler(this.btnClear_Click);
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(64, 675);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(89, 13);
            this.label2.TabIndex = 17;
            this.label2.Text = "Last Updated on:";
            // 
            // lastRefreshTime
            // 
            this.lastRefreshTime.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lastRefreshTime.AutoSize = true;
            this.lastRefreshTime.Location = new System.Drawing.Point(151, 676);
            this.lastRefreshTime.Name = "lastRefreshTime";
            this.lastRefreshTime.Size = new System.Drawing.Size(100, 13);
            this.lastRefreshTime.TabIndex = 18;
            this.lastRefreshTime.Text = "10-Feb-16 11:25:00";
            // 
            // splitContainer1
            // 
            this.splitContainer1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainer1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.splitContainer1.Location = new System.Drawing.Point(45, 157);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.listView1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.tabDetailsView);
            this.splitContainer1.Size = new System.Drawing.Size(865, 470);
            this.splitContainer1.SplitterDistance = 143;
            this.splitContainer1.TabIndex = 21;
            this.toolTip1.SetToolTip(this.splitContainer1, "Drag the splitter up or down to resize your view");
            // 
            // pictureBox1
            // 
            this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(44, 673);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(17, 17);
            this.pictureBox1.TabIndex = 22;
            this.pictureBox1.TabStop = false;
            this.toolTip1.SetToolTip(this.pictureBox1, "Filter is currently applied on the event logs");
            // 
            // txtEventsToCapture
            // 
            this.txtEventsToCapture.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.txtEventsToCapture.Location = new System.Drawing.Point(627, 8);
            this.txtEventsToCapture.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.txtEventsToCapture.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.txtEventsToCapture.Name = "txtEventsToCapture";
            this.txtEventsToCapture.Size = new System.Drawing.Size(61, 20);
            this.txtEventsToCapture.TabIndex = 17;
            this.toolTip1.SetToolTip(this.txtEventsToCapture, "Sets the events to capture");
            this.txtEventsToCapture.Value = new decimal(new int[] {
            500,
            0,
            0,
            0});
            // 
            // recordCount
            // 
            this.recordCount.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.recordCount.AutoSize = true;
            this.recordCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.recordCount.Location = new System.Drawing.Point(756, 133);
            this.recordCount.Name = "recordCount";
            this.recordCount.Size = new System.Drawing.Size(136, 13);
            this.recordCount.TabIndex = 20;
            this.recordCount.Text = "Number of events: 500";
            // 
            // pnlSuperLoggingStatus
            // 
            this.pnlSuperLoggingStatus.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pnlSuperLoggingStatus.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pnlSuperLoggingStatus.Controls.Add(this.txtEventsToCapture);
            this.pnlSuperLoggingStatus.Controls.Add(this.btnStatus);
            this.pnlSuperLoggingStatus.Controls.Add(this.lblStatusMessage);
            this.pnlSuperLoggingStatus.Controls.Add(this.lblCapture);
            this.pnlSuperLoggingStatus.Controls.Add(this.btnClear);
            this.pnlSuperLoggingStatus.Location = new System.Drawing.Point(45, 29);
            this.pnlSuperLoggingStatus.Name = "pnlSuperLoggingStatus";
            this.pnlSuperLoggingStatus.Size = new System.Drawing.Size(855, 39);
            this.pnlSuperLoggingStatus.TabIndex = 23;
            // 
            // refreshInterval
            // 
            this.refreshInterval.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.refreshInterval.FormattingEnabled = true;
            this.refreshInterval.Items.AddRange(new object[] {
            "30",
            "60",
            "120",
            "300",
            "600"});
            this.refreshInterval.Location = new System.Drawing.Point(214, 12);
            this.refreshInterval.Name = "refreshInterval";
            this.refreshInterval.Size = new System.Drawing.Size(45, 21);
            this.refreshInterval.TabIndex = 24;
            this.refreshInterval.SelectedIndexChanged += new System.EventHandler(this.refreshInterval_SelectedIndexChanged);
            // 
            // lblFilter
            // 
            this.lblFilter.AutoSize = true;
            this.lblFilter.ForeColor = System.Drawing.Color.Gray;
            this.lblFilter.Location = new System.Drawing.Point(323, 130);
            this.lblFilter.Name = "lblFilter";
            this.lblFilter.Size = new System.Drawing.Size(80, 13);
            this.lblFilter.TabIndex = 18;
            this.lblFilter.Text = "No filter applied";
            // 
            // lblFileName
            // 
            this.lblFileName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblFileName.AutoSize = true;
            this.lblFileName.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblFileName.Location = new System.Drawing.Point(105, 89);
            this.lblFileName.Name = "lblFileName";
            this.lblFileName.Size = new System.Drawing.Size(57, 13);
            this.lblFileName.TabIndex = 25;
            this.lblFileName.Text = "Filename";
            this.lblFileName.Visible = false;
            // 
            // pnlRefresh
            // 
            this.pnlRefresh.Controls.Add(this.label1);
            this.pnlRefresh.Controls.Add(this.chkAutoRefresh);
            this.pnlRefresh.Controls.Add(this.btnRefresh);
            this.pnlRefresh.Controls.Add(this.refreshInterval);
            this.pnlRefresh.Location = new System.Drawing.Point(409, 113);
            this.pnlRefresh.Name = "pnlRefresh";
            this.pnlRefresh.Size = new System.Drawing.Size(311, 38);
            this.pnlRefresh.TabIndex = 26;
            // 
            // pnlImportStatus
            // 
            this.pnlImportStatus.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pnlImportStatus.Controls.Add(this.lblImportStatus);
            this.pnlImportStatus.Location = new System.Drawing.Point(46, 30);
            this.pnlImportStatus.Name = "pnlImportStatus";
            this.pnlImportStatus.Size = new System.Drawing.Size(854, 38);
            this.pnlImportStatus.TabIndex = 27;
            // 
            // lblImportStatus
            // 
            this.lblImportStatus.AutoSize = true;
            this.lblImportStatus.Location = new System.Drawing.Point(11, 13);
            this.lblImportStatus.Name = "lblImportStatus";
            this.lblImportStatus.Size = new System.Drawing.Size(280, 13);
            this.lblImportStatus.TabIndex = 12;
            this.lblImportStatus.Text = "Import a  Super Log file (in json format) to view the events.";
            // 
            // lblFile
            // 
            this.lblFile.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblFile.AutoSize = true;
            this.lblFile.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblFile.Location = new System.Drawing.Point(44, 89);
            this.lblFile.Name = "lblFile";
            this.lblFile.Size = new System.Drawing.Size(61, 13);
            this.lblFile.TabIndex = 28;
            this.lblFile.Text = "Filename:";
            this.lblFile.Visible = false;
            // 
            // SuperLogging
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(958, 710);
            this.Controls.Add(this.lblFile);
            this.Controls.Add(this.pnlImportStatus);
            this.Controls.Add(this.pnlRefresh);
            this.Controls.Add(this.lblFileName);
            this.Controls.Add(this.lblFilter);
            this.Controls.Add(this.pnlSuperLoggingStatus);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.recordCount);
            this.Controls.Add(this.lastRefreshTime);
            this.Controls.Add(this.btnExport);
            this.Controls.Add(this.btnImport);
            this.Controls.Add(this.btnAddFilter);
            this.MinimumSize = new System.Drawing.Size(928, 727);
            this.Name = "SuperLogging";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = " ";
            this.Load += new System.EventHandler(this.SuperLogging_Load);
            this.Resize += new System.EventHandler(this.SuperLogging_Resize);
            this.tabDetailsView.ResumeLayout(false);
            this.tabGeneral.ResumeLayout(false);
            this.tabGeneral.PerformLayout();
            this.tabDetail.ResumeLayout(false);
            this.pnlFriendlyView.ResumeLayout(false);
            this.pnlJsonView.ResumeLayout(false);
            this.pnlJsonView.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtEventsToCapture)).EndInit();
            this.pnlSuperLoggingStatus.ResumeLayout(false);
            this.pnlSuperLoggingStatus.PerformLayout();
            this.pnlRefresh.ResumeLayout(false);
            this.pnlRefresh.PerformLayout();
            this.pnlImportStatus.ResumeLayout(false);
            this.pnlImportStatus.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnStatus;
        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.TabControl tabDetailsView;
        private System.Windows.Forms.TabPage tabGeneral;
        private System.Windows.Forms.TabPage tabDetail;
        private System.Windows.Forms.Timer autoRefresh;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.RadioButton rdoFriendlyView;
        private System.Windows.Forms.RadioButton rdoJsonView;
        private System.Windows.Forms.Panel pnlJsonView;
        private System.Windows.Forms.TextBox txtJsonRaw;
        private System.Windows.Forms.Panel pnlFriendlyView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.Label lblStatusMessage;
        private System.Windows.Forms.Label lblCapture;
        private System.Windows.Forms.Button btnAddFilter;
        private System.Windows.Forms.CheckBox chkAutoRefresh;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnRefresh;
        private System.Windows.Forms.Button btnImport;
        private System.Windows.Forms.Button btnExport;
        private System.Windows.Forms.Button btnClear;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label lastRefreshTime;
        private System.Windows.Forms.WebBrowser webBrowser;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox txtDuration;
        private System.Windows.Forms.TextBox txtStart;
        private System.Windows.Forms.TextBox txtCorrelationId;
        private System.Windows.Forms.TextBox txtEventType;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.TextBox txtProvider;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox txtAccount;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.Label recordCount;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label lblLevel;
        private System.Windows.Forms.Panel pnlSuperLoggingStatus;
        private System.Windows.Forms.NumericUpDown txtEventsToCapture;
        private System.Windows.Forms.ComboBox refreshInterval;
        private System.Windows.Forms.Label lblFilter;
        private System.Windows.Forms.Label lblFileName;
        private System.Windows.Forms.Panel pnlRefresh;
        private System.Windows.Forms.Panel pnlImportStatus;
        private System.Windows.Forms.Label lblImportStatus;
        private System.Windows.Forms.Label lblFile;
    }
}