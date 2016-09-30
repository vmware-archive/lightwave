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
    partial class NewExternalDomain
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
            this.txtDomainName = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.horizontalLine = new System.Windows.Forms.GroupBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.btnCreate = new System.Windows.Forms.Button();
            this.btnClose = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.cbIdentitySourceType = new System.Windows.Forms.ComboBox();
            this.cboAuthenticationType = new System.Windows.Forms.ComboBox();
            this.txtUserName = new System.Windows.Forms.TextBox();
            this.txtSearchTimeoutSeconds = new System.Windows.Forms.NumericUpDown();
            this.txtPrimaryURL = new System.Windows.Forms.TextBox();
            this.txtGroupBaseDN = new System.Windows.Forms.TextBox();
            this.txtUserBaseDN = new System.Windows.Forms.TextBox();
            this.txtFriendlyName = new System.Windows.Forms.TextBox();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.txtDomainAlias = new System.Windows.Forms.TextBox();
            this.txtADUsername = new System.Windows.Forms.TextBox();
            this.txtADSpn = new System.Windows.Forms.TextBox();
            this.txtADPassword = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.pnlNonAD = new System.Windows.Forms.Panel();
            this.label18 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.cbDNForNestedGroups = new System.Windows.Forms.CheckBox();
            this.label12 = new System.Windows.Forms.Label();
            this.cbGroupSearch = new System.Windows.Forms.CheckBox();
            this.label11 = new System.Windows.Forms.Label();
            this.cbMatchRuleInChain = new System.Windows.Forms.CheckBox();
            this.label10 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.cbSiteAffinity = new System.Windows.Forms.CheckBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.pnlAD = new System.Windows.Forms.Panel();
            this.label17 = new System.Windows.Forms.Label();
            this.rdoSpn = new System.Windows.Forms.RadioButton();
            this.rdoMachineAccount = new System.Windows.Forms.RadioButton();
            this.label14 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label15 = new System.Windows.Forms.Label();
            this.dgAttributes = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnRemove = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.label16 = new System.Windows.Forms.Label();
            this.dgAttributeMapContext = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.dgAttributeMap = new System.Windows.Forms.DataGridView();
            this.attribute = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Mapping = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.horizontalLine.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtSearchTimeoutSeconds)).BeginInit();
            this.pnlNonAD.SuspendLayout();
            this.pnlAD.SuspendLayout();
            this.dgAttributeMapContext.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributeMap)).BeginInit();
            this.SuspendLayout();
            // 
            // txtDomainName
            // 
            this.txtDomainName.Location = new System.Drawing.Point(121, 39);
            this.txtDomainName.Name = "txtDomainName";
            this.txtDomainName.Size = new System.Drawing.Size(256, 20);
            this.txtDomainName.TabIndex = 3;
            this.toolTip1.SetToolTip(this.txtDomainName, "Enter the FQDN of the domain that the identityy source managing. (e.g. example.co" +
        "m)");
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(40, 41);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Domain name:";
            // 
            // horizontalLine
            // 
            this.horizontalLine.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.horizontalLine.Controls.Add(this.groupBox3);
            this.horizontalLine.Location = new System.Drawing.Point(5, 644);
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.Size = new System.Drawing.Size(376, 5);
            this.horizontalLine.TabIndex = 26;
            this.horizontalLine.TabStop = false;
            // 
            // groupBox3
            // 
            this.groupBox3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox3.Location = new System.Drawing.Point(0, -262);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(376, 10);
            this.groupBox3.TabIndex = 27;
            this.groupBox3.TabStop = false;
            // 
            // btnCreate
            // 
            this.btnCreate.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnCreate.Location = new System.Drawing.Point(218, 657);
            this.btnCreate.Name = "btnCreate";
            this.btnCreate.Size = new System.Drawing.Size(75, 23);
            this.btnCreate.TabIndex = 39;
            this.btnCreate.Text = "Cr&eate";
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreate_Click);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(299, 657);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 40;
            this.btnClose.Text = "Cl&ose";
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // toolTip1
            // 
            this.toolTip1.AutomaticDelay = 200;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.toolTip1.ToolTipTitle = "External Domain";
            // 
            // cbIdentitySourceType
            // 
            this.cbIdentitySourceType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbIdentitySourceType.FormattingEnabled = true;
            this.cbIdentitySourceType.Items.AddRange(new object[] {
            "Active Directory (Integrated Windows Authentication)",
            "Active Directory as an LDAP Server",
            "Open LDAP"});
            this.cbIdentitySourceType.Location = new System.Drawing.Point(120, 11);
            this.cbIdentitySourceType.Name = "cbIdentitySourceType";
            this.cbIdentitySourceType.Size = new System.Drawing.Size(258, 21);
            this.cbIdentitySourceType.TabIndex = 30;
            this.toolTip1.SetToolTip(this.cbIdentitySourceType, "Choose an authentication type");
            this.cbIdentitySourceType.SelectedIndexChanged += new System.EventHandler(this.cbIdentitySourceType_SelectedIndexChanged);
            // 
            // cboAuthenticationType
            // 
            this.cboAuthenticationType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cboAuthenticationType.FormattingEnabled = true;
            this.cboAuthenticationType.ItemHeight = 13;
            this.cboAuthenticationType.Items.AddRange(new object[] {
            "ANONYMOUS",
            "USE_KERBEROS",
            "PASSWORD"});
            this.cboAuthenticationType.Location = new System.Drawing.Point(121, 167);
            this.cboAuthenticationType.Name = "cboAuthenticationType";
            this.cboAuthenticationType.Size = new System.Drawing.Size(257, 21);
            this.cboAuthenticationType.TabIndex = 35;
            this.toolTip1.SetToolTip(this.cboAuthenticationType, "Choose an authentication type");
            // 
            // txtUserName
            // 
            this.txtUserName.Location = new System.Drawing.Point(120, 195);
            this.txtUserName.Name = "txtUserName";
            this.txtUserName.Size = new System.Drawing.Size(258, 20);
            this.txtUserName.TabIndex = 11;
            this.toolTip1.SetToolTip(this.txtUserName, "Enter administrator DN of the form: CN=XXX,DC=XXX,DC=XXX");
            // 
            // txtSearchTimeoutSeconds
            // 
            this.txtSearchTimeoutSeconds.Location = new System.Drawing.Point(121, 135);
            this.txtSearchTimeoutSeconds.Maximum = new decimal(new int[] {
            3600,
            0,
            0,
            0});
            this.txtSearchTimeoutSeconds.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.txtSearchTimeoutSeconds.Name = "txtSearchTimeoutSeconds";
            this.txtSearchTimeoutSeconds.Size = new System.Drawing.Size(65, 20);
            this.txtSearchTimeoutSeconds.TabIndex = 9;
            this.toolTip1.SetToolTip(this.txtSearchTimeoutSeconds, "Choose a timeout value");
            this.txtSearchTimeoutSeconds.Value = new decimal(new int[] {
            300,
            0,
            0,
            0});
            // 
            // txtPrimaryURL
            // 
            this.txtPrimaryURL.Location = new System.Drawing.Point(121, 109);
            this.txtPrimaryURL.Name = "txtPrimaryURL";
            this.txtPrimaryURL.Size = new System.Drawing.Size(256, 20);
            this.txtPrimaryURL.TabIndex = 8;
            this.toolTip1.SetToolTip(this.txtPrimaryURL, "Enter the URL of the format: : ldap://<ip address or server>:<port>");
            // 
            // txtGroupBaseDN
            // 
            this.txtGroupBaseDN.Location = new System.Drawing.Point(120, 83);
            this.txtGroupBaseDN.Name = "txtGroupBaseDN";
            this.txtGroupBaseDN.Size = new System.Drawing.Size(257, 20);
            this.txtGroupBaseDN.TabIndex = 7;
            this.toolTip1.SetToolTip(this.txtGroupBaseDN, "Enter group base DN of the form: CN=XXX,DC=XXX,DC=XXX");
            // 
            // txtUserBaseDN
            // 
            this.txtUserBaseDN.Location = new System.Drawing.Point(120, 56);
            this.txtUserBaseDN.Name = "txtUserBaseDN";
            this.txtUserBaseDN.Size = new System.Drawing.Size(256, 20);
            this.txtUserBaseDN.TabIndex = 6;
            this.toolTip1.SetToolTip(this.txtUserBaseDN, "Enter user base DN of the form: CN=XXX,DC=XXX,DC=XXX");
            // 
            // txtFriendlyName
            // 
            this.txtFriendlyName.Location = new System.Drawing.Point(120, 30);
            this.txtFriendlyName.Name = "txtFriendlyName";
            this.txtFriendlyName.Size = new System.Drawing.Size(256, 20);
            this.txtFriendlyName.TabIndex = 5;
            this.toolTip1.SetToolTip(this.txtFriendlyName, "Enter a friendly name for the domain");
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(120, 222);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(258, 20);
            this.txtPassword.TabIndex = 12;
            this.toolTip1.SetToolTip(this.txtPassword, "Enter the password");
            // 
            // txtDomainAlias
            // 
            this.txtDomainAlias.Location = new System.Drawing.Point(120, 4);
            this.txtDomainAlias.Name = "txtDomainAlias";
            this.txtDomainAlias.Size = new System.Drawing.Size(255, 20);
            this.txtDomainAlias.TabIndex = 4;
            this.toolTip1.SetToolTip(this.txtDomainAlias, "Enter the alias for the domain");
            // 
            // txtADUsername
            // 
            this.txtADUsername.Location = new System.Drawing.Point(127, 185);
            this.txtADUsername.Name = "txtADUsername";
            this.txtADUsername.Size = new System.Drawing.Size(243, 20);
            this.txtADUsername.TabIndex = 7;
            this.toolTip1.SetToolTip(this.txtADUsername, "The user principal name that is used to authenticate with this identity source (e" +
        ".g. joe@example.com)");
            // 
            // txtADSpn
            // 
            this.txtADSpn.Location = new System.Drawing.Point(127, 152);
            this.txtADSpn.Name = "txtADSpn";
            this.txtADSpn.Size = new System.Drawing.Size(243, 20);
            this.txtADSpn.TabIndex = 6;
            this.toolTip1.SetToolTip(this.txtADSpn, "Service Principal Name (SPN) that allows Kerberos to identify the Active Director" +
        "y service. The Service Principal Name should follow the format STS/example.com");
            // 
            // txtADPassword
            // 
            this.txtADPassword.Location = new System.Drawing.Point(127, 216);
            this.txtADPassword.Name = "txtADPassword";
            this.txtADPassword.PasswordChar = '*';
            this.txtADPassword.Size = new System.Drawing.Size(243, 20);
            this.txtADPassword.TabIndex = 8;
            this.toolTip1.SetToolTip(this.txtADPassword, "Enter the name of the domain");
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(108, 13);
            this.label1.TabIndex = 29;
            this.label1.Text = "Identity Source Type:";
            // 
            // pnlNonAD
            // 
            this.pnlNonAD.Controls.Add(this.txtDomainAlias);
            this.pnlNonAD.Controls.Add(this.label18);
            this.pnlNonAD.Controls.Add(this.label3);
            this.pnlNonAD.Controls.Add(this.cboAuthenticationType);
            this.pnlNonAD.Controls.Add(this.cbDNForNestedGroups);
            this.pnlNonAD.Controls.Add(this.txtUserName);
            this.pnlNonAD.Controls.Add(this.label12);
            this.pnlNonAD.Controls.Add(this.cbGroupSearch);
            this.pnlNonAD.Controls.Add(this.label11);
            this.pnlNonAD.Controls.Add(this.txtSearchTimeoutSeconds);
            this.pnlNonAD.Controls.Add(this.cbMatchRuleInChain);
            this.pnlNonAD.Controls.Add(this.label10);
            this.pnlNonAD.Controls.Add(this.txtPrimaryURL);
            this.pnlNonAD.Controls.Add(this.label9);
            this.pnlNonAD.Controls.Add(this.txtGroupBaseDN);
            this.pnlNonAD.Controls.Add(this.label8);
            this.pnlNonAD.Controls.Add(this.cbSiteAffinity);
            this.pnlNonAD.Controls.Add(this.txtUserBaseDN);
            this.pnlNonAD.Controls.Add(this.label7);
            this.pnlNonAD.Controls.Add(this.txtFriendlyName);
            this.pnlNonAD.Controls.Add(this.label6);
            this.pnlNonAD.Controls.Add(this.txtPassword);
            this.pnlNonAD.Controls.Add(this.label5);
            this.pnlNonAD.Location = new System.Drawing.Point(1, 62);
            this.pnlNonAD.Name = "pnlNonAD";
            this.pnlNonAD.Size = new System.Drawing.Size(383, 311);
            this.pnlNonAD.TabIndex = 32;
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(58, 254);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(48, 13);
            this.label18.TabIndex = 50;
            this.label18.Text = "Settings:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(46, 6);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(70, 13);
            this.label3.TabIndex = 40;
            this.label3.Text = "Domain alias:";
            // 
            // cbDNForNestedGroups
            // 
            this.cbDNForNestedGroups.AutoSize = true;
            this.cbDNForNestedGroups.Location = new System.Drawing.Point(218, 257);
            this.cbDNForNestedGroups.Name = "cbDNForNestedGroups";
            this.cbDNForNestedGroups.Size = new System.Drawing.Size(158, 17);
            this.cbDNForNestedGroups.TabIndex = 49;
            this.cbDNForNestedGroups.Text = "Base DN for Nested Groups";
            this.cbDNForNestedGroups.UseVisualStyleBackColor = true;
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(52, 198);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(61, 13);
            this.label12.TabIndex = 36;
            this.label12.Text = "User name:";
            // 
            // cbGroupSearch
            // 
            this.cbGroupSearch.AutoSize = true;
            this.cbGroupSearch.Location = new System.Drawing.Point(116, 278);
            this.cbGroupSearch.Name = "cbGroupSearch";
            this.cbGroupSearch.Size = new System.Drawing.Size(92, 17);
            this.cbGroupSearch.TabIndex = 48;
            this.cbGroupSearch.Text = "Group Search";
            this.cbGroupSearch.UseVisualStyleBackColor = true;
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(13, 171);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(101, 13);
            this.label11.TabIndex = 34;
            this.label11.Text = "Authentication type:";
            // 
            // cbMatchRuleInChain
            // 
            this.cbMatchRuleInChain.AutoSize = true;
            this.cbMatchRuleInChain.Location = new System.Drawing.Point(218, 280);
            this.cbMatchRuleInChain.Name = "cbMatchRuleInChain";
            this.cbMatchRuleInChain.Size = new System.Drawing.Size(135, 17);
            this.cbMatchRuleInChain.TabIndex = 47;
            this.cbMatchRuleInChain.Text = "Matching Rule in chain";
            this.cbMatchRuleInChain.UseVisualStyleBackColor = true;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(36, 138);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(79, 13);
            this.label10.TabIndex = 32;
            this.label10.Text = "Timeout (secs):";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(46, 112);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(69, 13);
            this.label9.TabIndex = 30;
            this.label9.Text = "Primary URL:";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(31, 86);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(84, 13);
            this.label8.TabIndex = 28;
            this.label8.Text = "Group base DN:";
            // 
            // cbSiteAffinity
            // 
            this.cbSiteAffinity.AutoSize = true;
            this.cbSiteAffinity.Location = new System.Drawing.Point(116, 255);
            this.cbSiteAffinity.Name = "cbSiteAffinity";
            this.cbSiteAffinity.Size = new System.Drawing.Size(78, 17);
            this.cbSiteAffinity.TabIndex = 41;
            this.cbSiteAffinity.Text = "Site Affinity";
            this.cbSiteAffinity.UseVisualStyleBackColor = true;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(39, 59);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(77, 13);
            this.label7.TabIndex = 26;
            this.label7.Text = "User base DN:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(41, 33);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(75, 13);
            this.label6.TabIndex = 24;
            this.label6.Text = "Friendly name:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(56, 222);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(56, 13);
            this.label5.TabIndex = 38;
            this.label5.Text = "Password:";
            // 
            // pnlAD
            // 
            this.pnlAD.Controls.Add(this.label17);
            this.pnlAD.Controls.Add(this.rdoSpn);
            this.pnlAD.Controls.Add(this.rdoMachineAccount);
            this.pnlAD.Controls.Add(this.txtADPassword);
            this.pnlAD.Controls.Add(this.label14);
            this.pnlAD.Controls.Add(this.txtADSpn);
            this.pnlAD.Controls.Add(this.label13);
            this.pnlAD.Controls.Add(this.txtADUsername);
            this.pnlAD.Controls.Add(this.label4);
            this.pnlAD.Location = new System.Drawing.Point(1, 54);
            this.pnlAD.Name = "pnlAD";
            this.pnlAD.Size = new System.Drawing.Size(383, 245);
            this.pnlAD.TabIndex = 4;
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label17.Location = new System.Drawing.Point(8, 35);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(310, 13);
            this.label17.TabIndex = 39;
            this.label17.Text = "Choose machine account or SPN and provide details:";
            // 
            // rdoSpn
            // 
            this.rdoSpn.AutoSize = true;
            this.rdoSpn.Location = new System.Drawing.Point(10, 115);
            this.rdoSpn.Name = "rdoSpn";
            this.rdoSpn.Size = new System.Drawing.Size(188, 17);
            this.rdoSpn.TabIndex = 5;
            this.rdoSpn.TabStop = true;
            this.rdoSpn.Text = "Use Service Principal Name (SPN)";
            this.rdoSpn.UseVisualStyleBackColor = true;
            this.rdoSpn.CheckedChanged += new System.EventHandler(this.radioButton2_CheckedChanged);
            // 
            // rdoMachineAccount
            // 
            this.rdoMachineAccount.AutoSize = true;
            this.rdoMachineAccount.Location = new System.Drawing.Point(11, 65);
            this.rdoMachineAccount.Name = "rdoMachineAccount";
            this.rdoMachineAccount.Size = new System.Drawing.Size(131, 17);
            this.rdoMachineAccount.TabIndex = 4;
            this.rdoMachineAccount.TabStop = true;
            this.rdoMachineAccount.Text = "Use Machine Account";
            this.rdoMachineAccount.UseVisualStyleBackColor = true;
            this.rdoMachineAccount.CheckedChanged += new System.EventHandler(this.radioButton1_CheckedChanged);
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(67, 218);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(56, 13);
            this.label14.TabIndex = 38;
            this.label14.Text = "Password:";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(4, 154);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(120, 13);
            this.label13.TabIndex = 36;
            this.label13.Text = "Service Principal Name:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(18, 187);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(106, 13);
            this.label4.TabIndex = 34;
            this.label4.Text = "User Principal Name:";
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(8, 525);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(121, 13);
            this.label15.TabIndex = 39;
            this.label15.Text = "Shema Object Mapping:";
            // 
            // dgAttributes
            // 
            this.dgAttributes.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.dgAttributes.FullRowSelect = true;
            this.dgAttributes.GridLines = true;
            this.dgAttributes.Location = new System.Drawing.Point(7, 541);
            this.dgAttributes.MultiSelect = false;
            this.dgAttributes.Name = "dgAttributes";
            this.dgAttributes.Size = new System.Drawing.Size(335, 98);
            this.dgAttributes.TabIndex = 36;
            this.dgAttributes.UseCompatibleStateImageBehavior = false;
            this.dgAttributes.View = System.Windows.Forms.View.Details;
            this.dgAttributes.SelectedIndexChanged += new System.EventHandler(this.dgAttributes_SelectedIndexChanged);
            this.dgAttributes.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.dgAttributes_MouseDoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Object ID - Class Name";
            this.columnHeader1.Width = 300;
            // 
            // btnRemove
            // 
            this.btnRemove.Location = new System.Drawing.Point(348, 614);
            this.btnRemove.Name = "btnRemove";
            this.btnRemove.Size = new System.Drawing.Size(25, 25);
            this.btnRemove.TabIndex = 38;
            this.btnRemove.Text = "-";
            this.btnRemove.UseVisualStyleBackColor = true;
            this.btnRemove.Click += new System.EventHandler(this.btnRemove_Click);
            // 
            // btnAdd
            // 
            this.btnAdd.Location = new System.Drawing.Point(348, 541);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(25, 25);
            this.btnAdd.TabIndex = 37;
            this.btnAdd.Text = "+";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(4, 376);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(73, 13);
            this.label16.TabIndex = 46;
            this.label16.Text = "Attribute Map:";
            // 
            // dgAttributeMapContext
            // 
            this.dgAttributeMapContext.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem1});
            this.dgAttributeMapContext.Name = "dgContext";
            this.dgAttributeMapContext.Size = new System.Drawing.Size(108, 26);
            this.dgAttributeMapContext.Click += new System.EventHandler(this.dgAttributeMapContext_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(107, 22);
            this.toolStripMenuItem1.Text = "Delete";
            // 
            // dgAttributeMap
            // 
            this.dgAttributeMap.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgAttributeMap.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.attribute,
            this.Mapping});
            this.dgAttributeMap.ContextMenuStrip = this.dgAttributeMapContext;
            this.dgAttributeMap.Location = new System.Drawing.Point(7, 392);
            this.dgAttributeMap.MultiSelect = false;
            this.dgAttributeMap.Name = "dgAttributeMap";
            this.dgAttributeMap.Size = new System.Drawing.Size(366, 126);
            this.dgAttributeMap.TabIndex = 35;
            // 
            // attribute
            // 
            this.attribute.HeaderText = "Attribute";
            this.attribute.Name = "attribute";
            this.attribute.Width = 200;
            // 
            // Mapping
            // 
            this.Mapping.HeaderText = "Mapping";
            this.Mapping.Name = "Mapping";
            this.Mapping.Width = 120;
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Location = new System.Drawing.Point(7, 136);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(0, 10);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Location = new System.Drawing.Point(11, 293);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(0, 10);
            this.groupBox1.TabIndex = 17;
            this.groupBox1.TabStop = false;
            // 
            // NewExternalDomain
            // 
            this.AcceptButton = this.btnCreate;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(386, 688);
            this.Controls.Add(this.pnlAD);
            this.Controls.Add(this.dgAttributeMap);
            this.Controls.Add(this.label16);
            this.Controls.Add(this.dgAttributes);
            this.Controls.Add(this.btnRemove);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.label15);
            this.Controls.Add(this.cbIdentitySourceType);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.txtDomainName);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.horizontalLine);
            this.Controls.Add(this.btnCreate);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.pnlNonAD);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "NewExternalDomain";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "New external domain";
            this.Load += new System.EventHandler(this.NewExternalDomain_Load);
            this.horizontalLine.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.txtSearchTimeoutSeconds)).EndInit();
            this.pnlNonAD.ResumeLayout(false);
            this.pnlNonAD.PerformLayout();
            this.pnlAD.ResumeLayout(false);
            this.pnlAD.PerformLayout();
            this.dgAttributeMapContext.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dgAttributeMap)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtDomainName;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.ComboBox cbIdentitySourceType;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Panel pnlNonAD;
        private System.Windows.Forms.ComboBox cboAuthenticationType;
        private System.Windows.Forms.TextBox txtUserName;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.NumericUpDown txtSearchTimeoutSeconds;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox txtPrimaryURL;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox txtGroupBaseDN;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox txtUserBaseDN;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox txtFriendlyName;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox txtDomainAlias;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Panel pnlAD;
        private System.Windows.Forms.TextBox txtADPassword;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox txtADSpn;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.TextBox txtADUsername;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.RadioButton rdoSpn;
        private System.Windows.Forms.RadioButton rdoMachineAccount;
        private System.Windows.Forms.CheckBox cbSiteAffinity;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.ListView dgAttributes;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.Button btnRemove;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.ContextMenuStrip dgAttributeMapContext;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
        private System.Windows.Forms.DataGridView dgAttributeMap;
        private System.Windows.Forms.DataGridViewTextBoxColumn attribute;
        private System.Windows.Forms.DataGridViewTextBoxColumn Mapping;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.CheckBox cbMatchRuleInChain;
        private System.Windows.Forms.CheckBox cbGroupSearch;
        private System.Windows.Forms.CheckBox cbDNForNestedGroups;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox1;

    }
}