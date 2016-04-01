﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using VMAFD.Client;
using VMIdentity.CommonUtils;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.Common.Helpers;
using VMPscHighAvailabilitySnapIn.ScopeNodes;
using VMPscHighAvailabilitySnapIn.SnapIn;
using VMPscHighAvailabilitySnapIn.Utils;

namespace VMPscHighAvailabilitySnapIn.UI
{
    /// <summary>
    /// Management view control
    /// </summary>
    public partial class ManagementViewControl : UserControl, IFormViewControl
    {
        #region Auto-generated code

        private ListView lstdcs;
        private ColumnHeader columnHeader1;
        private ColumnHeader columnHeader2;
        private ColumnHeader columnHeader3;
        private Button btnHA;
        private Label lblStatus;
        private Label lblLastRefreshed;
        private CheckBox chkAutoRefresh;
        private ComboBox cbInterval;
        private Label lblSeconds;
        private ColumnHeader columnHeader5;
        private ListView lstServices;
        private ColumnHeader columnHeader6;
        private ColumnHeader columnHeader11;
        private ColumnHeader columnHeader9;
        private ColumnHeader columnHeader10;
        private Label label1;
        private GroupBox groupBox1;
        private Label label4;
        private TextBox txtDomainControllerName;
        private Button btnRefresh;
        private PictureBox pictureBox1;
        private Label lblSitename;
        private ToolTip toolTip1;
        private System.ComponentModel.IContainer components;
        private Label label3;
        private Label lblState;
        private PictureBox pcHealth;
        private PictureBox pictureBox3;
        private Label label5;
        private Label lblSelectedDomainController;
        private TextBox lblName;
        private Label label7;
        private TextBox txtIpAddress;
        private ColumnHeader columnHeader4;
        private Label label2;       

        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ManagementViewControl));
            this.lstdcs = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnHA = new System.Windows.Forms.Button();
            this.lblStatus = new System.Windows.Forms.Label();
            this.lblLastRefreshed = new System.Windows.Forms.Label();
            this.chkAutoRefresh = new System.Windows.Forms.CheckBox();
            this.cbInterval = new System.Windows.Forms.ComboBox();
            this.lblSeconds = new System.Windows.Forms.Label();
            this.lstServices = new System.Windows.Forms.ListView();
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader11 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader9 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader10 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtDomainControllerName = new System.Windows.Forms.TextBox();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.lblSitename = new System.Windows.Forms.Label();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.label3 = new System.Windows.Forms.Label();
            this.lblState = new System.Windows.Forms.Label();
            this.pcHealth = new System.Windows.Forms.PictureBox();
            this.pictureBox3 = new System.Windows.Forms.PictureBox();
            this.label5 = new System.Windows.Forms.Label();
            this.lblSelectedDomainController = new System.Windows.Forms.Label();
            this.lblName = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.txtIpAddress = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pcHealth)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).BeginInit();
            this.SuspendLayout();
            // 
            // lstdcs
            // 
            this.lstdcs.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstdcs.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader5,
            this.columnHeader2,
            this.columnHeader3});
            this.lstdcs.FullRowSelect = true;
            this.lstdcs.GridLines = true;
            this.lstdcs.HideSelection = false;
            this.lstdcs.Location = new System.Drawing.Point(13, 235);
            this.lstdcs.MultiSelect = false;
            this.lstdcs.Name = "lstdcs";
            this.lstdcs.Size = new System.Drawing.Size(596, 191);
            this.lstdcs.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lstdcs.TabIndex = 0;
            this.lstdcs.UseCompatibleStateImageBehavior = false;
            this.lstdcs.View = System.Windows.Forms.View.Details;
            this.lstdcs.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lstdcs_ColumnClick);
            this.lstdcs.SelectedIndexChanged += new System.EventHandler(this.lstdcs_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Host name";
            this.columnHeader1.Width = 240;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Affinitized";
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Status of Services";
            this.columnHeader2.Width = 220;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Status";
            // 
            // btnHA
            // 
            this.btnHA.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnHA.Location = new System.Drawing.Point(513, 94);
            this.btnHA.Name = "btnHA";
            this.btnHA.Size = new System.Drawing.Size(91, 25);
            this.btnHA.TabIndex = 1;
            this.btnHA.Text = "Enable HA";
            this.btnHA.UseVisualStyleBackColor = true;
            this.btnHA.Click += new System.EventHandler(this.btnHA_Click);
            // 
            // lblStatus
            // 
            this.lblStatus.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblStatus.AutoSize = true;
            this.lblStatus.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblStatus.Location = new System.Drawing.Point(547, 58);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(41, 13);
            this.lblStatus.TabIndex = 2;
            this.lblStatus.Text = "label1";
            // 
            // lblLastRefreshed
            // 
            this.lblLastRefreshed.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblLastRefreshed.AutoSize = true;
            this.lblLastRefreshed.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblLastRefreshed.Location = new System.Drawing.Point(496, 211);
            this.lblLastRefreshed.Name = "lblLastRefreshed";
            this.lblLastRefreshed.Size = new System.Drawing.Size(35, 13);
            this.lblLastRefreshed.TabIndex = 3;
            this.lblLastRefreshed.Text = "label1";
            // 
            // chkAutoRefresh
            // 
            this.chkAutoRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.chkAutoRefresh.AutoSize = true;
            this.chkAutoRefresh.Location = new System.Drawing.Point(162, 209);
            this.chkAutoRefresh.Name = "chkAutoRefresh";
            this.chkAutoRefresh.Size = new System.Drawing.Size(88, 17);
            this.chkAutoRefresh.TabIndex = 4;
            this.chkAutoRefresh.Text = "Auto-Refresh";
            this.chkAutoRefresh.UseVisualStyleBackColor = true;
            this.chkAutoRefresh.CheckedChanged += new System.EventHandler(this.chkAutoRefresh_CheckedChanged);
            // 
            // cbInterval
            // 
            this.cbInterval.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbInterval.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbInterval.FormattingEnabled = true;
            this.cbInterval.Items.AddRange(new object[] {
            "30",
            "60",
            "120",
            "300",
            "600"});
            this.cbInterval.Location = new System.Drawing.Point(248, 207);
            this.cbInterval.Name = "cbInterval";
            this.cbInterval.Size = new System.Drawing.Size(36, 21);
            this.cbInterval.TabIndex = 5;
            this.cbInterval.SelectedIndexChanged += new System.EventHandler(this.cbInterval_SelectedIndexChanged);
            // 
            // lblSeconds
            // 
            this.lblSeconds.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblSeconds.AutoSize = true;
            this.lblSeconds.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblSeconds.Location = new System.Drawing.Point(284, 211);
            this.lblSeconds.Name = "lblSeconds";
            this.lblSeconds.Size = new System.Drawing.Size(47, 13);
            this.lblSeconds.TabIndex = 6;
            this.lblSeconds.Text = "seconds";
            // 
            // lstServices
            // 
            this.lstServices.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstServices.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader6,
            this.columnHeader4,
            this.columnHeader11,
            this.columnHeader9,
            this.columnHeader10});
            this.lstServices.FullRowSelect = true;
            this.lstServices.GridLines = true;
            this.lstServices.HideSelection = false;
            this.lstServices.Location = new System.Drawing.Point(13, 477);
            this.lstServices.MultiSelect = false;
            this.lstServices.Name = "lstServices";
            this.lstServices.Size = new System.Drawing.Size(596, 238);
            this.lstServices.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lstServices.TabIndex = 7;
            this.lstServices.UseCompatibleStateImageBehavior = false;
            this.lstServices.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Service Name";
            this.columnHeader6.Width = 120;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Description";
            this.columnHeader4.Width = 202;
            // 
            // columnHeader11
            // 
            this.columnHeader11.Text = "Port";
            // 
            // columnHeader9
            // 
            this.columnHeader9.Text = "Status";
            this.columnHeader9.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // columnHeader10
            // 
            this.columnHeader10.Text = "Last Heartbeat";
            this.columnHeader10.Width = 140;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(15, 458);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(120, 13);
            this.label1.TabIndex = 8;
            this.label1.Text = "Services hosted on ";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(15, 210);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(117, 13);
            this.label2.TabIndex = 9;
            this.label2.Text = "Domain Controllers:";
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Location = new System.Drawing.Point(3, 77);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(607, 10);
            this.groupBox1.TabIndex = 11;
            this.groupBox1.TabStop = false;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(15, 169);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(171, 13);
            this.label4.TabIndex = 12;
            this.label4.Text = "Affinitized Domain Controller:";
            // 
            // txtDomainControllerName
            // 
            this.txtDomainControllerName.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtDomainControllerName.Location = new System.Drawing.Point(200, 170);
            this.txtDomainControllerName.Name = "txtDomainControllerName";
            this.txtDomainControllerName.ReadOnly = true;
            this.txtDomainControllerName.Size = new System.Drawing.Size(398, 13);
            this.txtDomainControllerName.TabIndex = 13;
            // 
            // btnRefresh
            // 
            this.btnRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnRefresh.Location = new System.Drawing.Point(337, 203);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(64, 25);
            this.btnRefresh.TabIndex = 14;
            this.btnRefresh.Text = "Refresh";
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(7, 38);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(29, 36);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 15;
            this.pictureBox1.TabStop = false;
            // 
            // lblSitename
            // 
            this.lblSitename.AutoSize = true;
            this.lblSitename.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblSitename.Location = new System.Drawing.Point(35, 97);
            this.lblSitename.Name = "lblSitename";
            this.lblSitename.Size = new System.Drawing.Size(25, 13);
            this.lblSitename.TabIndex = 17;
            this.lblSitename.Text = "Site";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(15, 134);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(41, 13);
            this.label3.TabIndex = 18;
            this.label3.Text = "State:";
            // 
            // lblState
            // 
            this.lblState.AutoSize = true;
            this.lblState.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblState.Location = new System.Drawing.Point(61, 135);
            this.lblState.Name = "lblState";
            this.lblState.Size = new System.Drawing.Size(47, 13);
            this.lblState.TabIndex = 19;
            this.lblState.Text = "seconds";
            // 
            // pcHealth
            // 
            this.pcHealth.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.pcHealth.Image = ((System.Drawing.Image)(resources.GetObject("pcHealth.Image")));
            this.pcHealth.Location = new System.Drawing.Point(526, 56);
            this.pcHealth.Name = "pcHealth";
            this.pcHealth.Size = new System.Drawing.Size(16, 16);
            this.pcHealth.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pcHealth.TabIndex = 20;
            this.pcHealth.TabStop = false;
            // 
            // pictureBox3
            // 
            this.pictureBox3.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox3.Image")));
            this.pictureBox3.Location = new System.Drawing.Point(17, 94);
            this.pictureBox3.Name = "pictureBox3";
            this.pictureBox3.Size = new System.Drawing.Size(16, 16);
            this.pictureBox3.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox3.TabIndex = 21;
            this.pictureBox3.TabStop = false;
            // 
            // label5
            // 
            this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(425, 211);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(71, 13);
            this.label5.TabIndex = 22;
            this.label5.Text = "Refreshed at ";
            // 
            // lblSelectedDomainController
            // 
            this.lblSelectedDomainController.AutoSize = true;
            this.lblSelectedDomainController.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblSelectedDomainController.Location = new System.Drawing.Point(130, 458);
            this.lblSelectedDomainController.Name = "lblSelectedDomainController";
            this.lblSelectedDomainController.Size = new System.Drawing.Size(108, 13);
            this.lblSelectedDomainController.TabIndex = 23;
            this.lblSelectedDomainController.Text = "domain controller:";
            // 
            // lblName
            // 
            this.lblName.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.lblName.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblName.Location = new System.Drawing.Point(41, 50);
            this.lblName.Name = "lblName";
            this.lblName.ReadOnly = true;
            this.lblName.Size = new System.Drawing.Size(479, 22);
            this.lblName.TabIndex = 35;
            this.lblName.TabStop = false;
            this.lblName.Text = "label1";
            // 
            // label7
            // 
            this.label7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(345, 101);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(61, 13);
            this.label7.TabIndex = 37;
            this.label7.Text = "IP Address:";
            // 
            // txtIpAddress
            // 
            this.txtIpAddress.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.txtIpAddress.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtIpAddress.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtIpAddress.Location = new System.Drawing.Point(407, 101);
            this.txtIpAddress.Name = "txtIpAddress";
            this.txtIpAddress.ReadOnly = true;
            this.txtIpAddress.Size = new System.Drawing.Size(91, 13);
            this.txtIpAddress.TabIndex = 38;
            this.txtIpAddress.TabStop = false;
            this.txtIpAddress.Text = "192.168.255.255";
            // 
            // ManagementViewControl
            // 
            this.Controls.Add(this.txtIpAddress);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.cbInterval);
            this.Controls.Add(this.lblLastRefreshed);
            this.Controls.Add(this.lblName);
            this.Controls.Add(this.lblSelectedDomainController);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.pictureBox3);
            this.Controls.Add(this.pcHealth);
            this.Controls.Add(this.lblState);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.lblSitename);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.btnRefresh);
            this.Controls.Add(this.txtDomainControllerName);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lstServices);
            this.Controls.Add(this.lblSeconds);
            this.Controls.Add(this.chkAutoRefresh);
            this.Controls.Add(this.lblStatus);
            this.Controls.Add(this.btnHA);
            this.Controls.Add(this.lstdcs);
            this.MinimumSize = new System.Drawing.Size(616, 730);
            this.Name = "ManagementViewControl";
            this.Size = new System.Drawing.Size(616, 730);
            this.Load += new System.EventHandler(this.ManagementViewControl_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pcHealth)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        ManagementFormView _formView;
        private Timer _timer;
        private bool _autoRefresh;
        private bool legacyMode;
        private ServerDto _serverDto;
        private ManagementDto _dto;
        private IEnumerable<NodeDto> _infraDtos;
        

        public ManagementViewControl()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            Dock = DockStyle.Fill;
            var il = new ImageList();
            var image = ResourceHelper.GetToolbarImage();
            il.Images.AddStrip(image);
            lstdcs.SmallImageList = il;
            lstServices.SmallImageList = il;
            _autoRefresh = false;
            _timer = new Timer();
            _timer.Interval = Constants.DefaultTimerRefreshInterval * Constants.MilliSecsMultiplier;
            _timer.Tick += timer_Tick;
            LoadCachedDetails();
            var description = CdcDcStateHelper.GetHealthDescription();
            toolTip1.SetToolTip(pcHealth, description);
            cbInterval.SelectedIndex = 0;
        }

        void timer_Tick(object sender, EventArgs e)
        {
            try
            { 
                RefreshView();
            }
            catch (Exception exc)
            {
                MiscUtilsService.ShowError(exc);
                _timer.Enabled = false;
                chkAutoRefresh.Checked = false;
            }
        }

        public void RefreshDataSource()
        {
            try
            { 
                RefreshView();
            }
            catch (Exception exc)
            {
                MiscUtilsService.ShowError(exc);
            }
        }

        private void LoadCachedDetails()
        {
            var node = _formView.ScopeNode as ManagementNode;
            lblName.Text = node.DisplayName;
            var serverNode = node.GetServerNode();
            var siteName = node.GetSiteName();
            lblSitename.Text = siteName;
            //_infraDtos = serverNode.Hosts.Where(x => x.NodeType == NodeType.Infrastructure && x.Sitename == siteName);
            _serverDto = new ServerDto { Server = node.DisplayName, Upn = node.ServerDto.Upn, Password = node.ServerDto.Password, DomainName = node.ServerDto.DomainName };
            _dto = serverNode.Hosts.First(x => x.Name == node.DisplayName) as ManagementDto;
            _infraDtos = _dto.DomainControllers;
            txtIpAddress.Text = _dto.Ip;
            if (_dto.State == null)
            {
                _dto = PscHighAvailabilityAppEnvironment.Instance.Service.GetManagementNodeDetails(_serverDto);
            }
            UpdateState();
            foreach (InfrastructureDto dc in _infraDtos)
            {
                var affinitized = _dto.DomainController.Name == dc.Name ? "Yes" : string.Empty;
                var services = CdcDcStateHelper.GetActiveServiceDesc(dc);
                var status = dc.Active
                    ? Constants.Active
                    : Constants.InActive;
                var values = new string[] { dc.Name, affinitized, services, status };
                ListViewItem item = new ListViewItem(values) { Tag = dc, ImageIndex = (int)ImageIndex.Infrastructure };
                item.BackColor = dc.Active ? Color.LightGreen : Color.Pink;
                lstdcs.Items.Add(item);
            }
            lblLastRefreshed.Text = DateTime.Now.ToString(Constants.DateFormat);
            if (lstdcs.Items.Count > 0)
            {
                lstdcs.Items[node.SelectedInfrastructureItem].Selected = true;
            }
            UpdateServices();
        }

        private void RefreshView()
        {
                var node = _formView.ScopeNode as ManagementNode;
                var serverNode = node.GetServerNode();
                var siteName = node.GetSiteName();
                if (serverNode!= null && serverNode.Hosts != null)
                {
                    _dto = serverNode.Hosts.First(x => x.Name == node.DisplayName) as ManagementDto;
                    _infraDtos = _dto.DomainControllers;
                    UpdateState();

                    foreach (ListViewItem item in lstdcs.Items)
                    {
                        var infDto = item.Tag as InfrastructureDto;
                        var dto = _infraDtos.First(x => x.Name == infDto.Name);
                        item.Tag = dto;
                        item.BackColor = dto.Active ? Color.LightGreen : Color.Pink;
                        var status = dto.Active
                        ? Constants.Active
                        : Constants.InActive;
                        var services = CdcDcStateHelper.GetActiveServiceDesc((InfrastructureDto)dto);
                        item.SubItems[2].Text = services;
                        item.SubItems[3].Text = status;
                        item.SubItems[1].Text = (item.SubItems[0].Text == _dto.DomainController.Name) ? "Yes" : string.Empty;
                        item.Tag = dto;
                    }
                    if (lstdcs.Items.Count > 0)
                    {
                        lstdcs.Items[node.SelectedInfrastructureItem].Selected = true;
                    }
                    UpdateServices();
                }
        }

        private void UpdateState()
        {
            lblState.Text = _dto.State.Description;
            var health = CdcDcStateHelper.GetHealth(_dto.State, _infraDtos);
            lblStatus.Text = health.ToString().ToUpper();
            var description = CdcDcStateHelper.GetHealthDescription(health);
            toolTip1.SetToolTip(lblStatus, description);
            lblStatus.ForeColor = HealthHelper.GetHealthColor(health);
            lblLastRefreshed.Text = DateTime.Now.ToString(Constants.DateFormat);
            btnHA.Text = _dto.Legacy ? Constants.EnableDefaultHA : Constants.EnableLegacy;
            legacyMode = _dto.Legacy;
            txtDomainControllerName.Text = _dto.DomainController.Name;
        }       

        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (ManagementFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
        }

        private void ChangeMode()
        {
            try
            {
                var node = _formView.ScopeNode as ManagementNode;
                var serverDto = new ServerDto { Server = node.DisplayName, Upn = node.ServerDto.Upn, Password = node.ServerDto.Password };
                PscHighAvailabilityAppEnvironment.Instance.Service.SetLegacyMode(!legacyMode, serverDto);
                _dto.Legacy = !legacyMode;
                btnHA.Text = _dto.Legacy ? Constants.EnableDefaultHA : Constants.EnableLegacy;
                legacyMode = _dto.Legacy;
                var state = _dto.Legacy ? CDC_DC_STATE.CDC_DC_STATE_LEGACY : CDC_DC_STATE.CDC_DC_STATE_NO_DC_LIST;
                _dto.State = CdcDcStateHelper.GetStateDescription(state);
            }
            catch(Exception exc)
            {
                MiscUtilsService.ShowError(exc);
            }
            UpdateState();
        }
        private void UpdateServices()
        {   
            lstServices.Items.Clear();

            if (lstdcs.SelectedItems != null && lstdcs.SelectedItems.Count > 0)
            {
                var dto = lstdcs.SelectedItems[0].Tag as InfrastructureDto;
                lblSelectedDomainController.Text = dto.Name;
                foreach (ServiceDto service in dto.Services)
                {
                    var status = service.Alive ? Constants.Active : Constants.InActive;
                    var hb = DateTimeConverter.ToDurationAgo(service.LastHeartbeat);
                    var port = service.Port == 0 ? string.Empty : service.Port.ToString();
                    var values = new string[] { service.ServiceName,service.Description, port, status, hb };
                    ListViewItem item = new ListViewItem(values) { ImageIndex = (int)ImageIndex.Service };
                    item.BackColor = service.Alive ? Color.LightGreen : Color.Pink;
                    lstServices.Items.Add(item);
                }
            }
           lstServices.Refresh();
        }
        private void lstdcs_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lstdcs.SelectedItems != null && lstdcs.SelectedItems.Count > 0)
            {
                var node = _formView.ScopeNode as ManagementNode;
                node.SelectedInfrastructureItem = lstdcs.SelectedIndices[0];
                UpdateServices();
            }
        }

        private void chkAutoRefresh_CheckedChanged(object sender, EventArgs e)
        {
            _autoRefresh = !_autoRefresh;
            _timer.Enabled = _autoRefresh;
            cbInterval.Enabled = _autoRefresh;
        }

        private void cbInterval_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cbInterval.SelectedIndex > -1 && cbInterval.SelectedItem != null)
            {
                var interval = int.Parse(cbInterval.SelectedItem.ToString());
                _timer.Interval =  interval* Constants.MilliSecsMultiplier;
            }
        }

        private void btnHA_Click(object sender, EventArgs e)
        {
            var mode = legacyMode ? Constants.HA : Constants.Legacy;
            var result = MessageBox.Show(Constants.ModeChange + mode + "?", Constants.Confirm, MessageBoxButtons.YesNo);

            if(result == DialogResult.Yes)
                ChangeMode();
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            try
            {
                RefreshView();
            }
            catch (Exception exc)
            {
                MiscUtilsService.ShowError(exc);
            }
        }

        private void lstdcs_ColumnClick(object sender, ColumnClickEventArgs e)
        {            
        }

        private void ManagementViewControl_Load(object sender, EventArgs e)
        {
        }

        internal void Cleanup()
        {
            if(_timer!= null)
            {
                _timer.Enabled = false;
                _timer = null;
            }
        }
    }

    public enum SortOrder
    {
        Hostname = 0,
        Affinitized,
        Service,
        Status
    }
}
