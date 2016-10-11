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
namespace Vmware.Tools.RestSsoAdminSnapIn.Views.Wizard
{
    partial class AddExternalDomain
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AddExternalDomain));
            this.btnNext = new System.Windows.Forms.Button();
            this.pnlStep1 = new System.Windows.Forms.Panel();
            this.button14 = new System.Windows.Forms.Button();
            this.button13 = new System.Windows.Forms.Button();
            this.button12 = new System.Windows.Forms.Button();
            this.button11 = new System.Windows.Forms.Button();
            this.rdoopenLdap = new System.Windows.Forms.RadioButton();
            this.rdoADLdap = new System.Windows.Forms.RadioButton();
            this.rdoADWindowsAuth = new System.Windows.Forms.RadioButton();
            this.lblCaption = new System.Windows.Forms.Label();
            this.pnlStep2 = new System.Windows.Forms.Panel();
            this.txtFriendlyName = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.pnlConnectionString = new System.Windows.Forms.Panel();
            this.txtPrimaryConnectionString = new System.Windows.Forms.TextBox();
            this.txtSecondaryConnectionString = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.pnlProtect = new System.Windows.Forms.Panel();
            this.label1 = new System.Windows.Forms.Label();
            this.chkProtect = new System.Windows.Forms.CheckBox();
            this.txtConnectionString = new System.Windows.Forms.TextBox();
            this.rdoSpecificDomain = new System.Windows.Forms.RadioButton();
            this.rdoAnyDomain = new System.Windows.Forms.RadioButton();
            this.button16 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            this.txtGroupDN = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.txtUserDN = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtDomainAlias = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtDomainName = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.pnlStep3 = new System.Windows.Forms.Panel();
            this.button18 = new System.Windows.Forms.Button();
            this.button15 = new System.Windows.Forms.Button();
            this.button6 = new System.Windows.Forms.Button();
            this.button7 = new System.Windows.Forms.Button();
            this.lstCertificateChain = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnRemoveCert = new System.Windows.Forms.Button();
            this.label11 = new System.Windows.Forms.Label();
            this.btnAddCert = new System.Windows.Forms.Button();
            this.pnlStep4 = new System.Windows.Forms.Panel();
            this.radioButton2 = new System.Windows.Forms.RadioButton();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.txtSPN = new System.Windows.Forms.TextBox();
            this.lblSPN = new System.Windows.Forms.Label();
            this.button17 = new System.Windows.Forms.Button();
            this.button10 = new System.Windows.Forms.Button();
            this.button8 = new System.Windows.Forms.Button();
            this.button9 = new System.Windows.Forms.Button();
            this.button5 = new System.Windows.Forms.Button();
            this.label7 = new System.Windows.Forms.Label();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.label12 = new System.Windows.Forms.Label();
            this.txtUsername = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.restSsoAdminInstaller1 = new Vmware.Tools.RestSsoAdminSnapIn.Installer.RestSsoAdminInstaller();
            this.pnlStep1.SuspendLayout();
            this.pnlStep2.SuspendLayout();
            this.pnlConnectionString.SuspendLayout();
            this.pnlProtect.SuspendLayout();
            this.pnlStep3.SuspendLayout();
            this.pnlStep4.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnNext
            // 
            this.btnNext.Location = new System.Drawing.Point(246, 290);
            this.btnNext.Name = "btnNext";
            this.btnNext.Size = new System.Drawing.Size(75, 22);
            this.btnNext.TabIndex = 0;
            this.btnNext.Text = "Next";
            this.btnNext.UseVisualStyleBackColor = true;
            this.btnNext.Click += new System.EventHandler(this.btnNext_Click);
            // 
            // pnlStep1
            // 
            this.pnlStep1.Controls.Add(this.button14);
            this.pnlStep1.Controls.Add(this.button13);
            this.pnlStep1.Controls.Add(this.button12);
            this.pnlStep1.Controls.Add(this.button11);
            this.pnlStep1.Controls.Add(this.rdoopenLdap);
            this.pnlStep1.Controls.Add(this.rdoADLdap);
            this.pnlStep1.Controls.Add(this.btnNext);
            this.pnlStep1.Controls.Add(this.rdoADWindowsAuth);
            this.pnlStep1.Controls.Add(this.lblCaption);
            this.pnlStep1.Location = new System.Drawing.Point(6, 2);
            this.pnlStep1.Name = "pnlStep1";
            this.pnlStep1.Size = new System.Drawing.Size(483, 317);
            this.pnlStep1.TabIndex = 2;
            // 
            // button14
            // 
            this.helpProvider1.SetHelpString(this.button14, "Choose this option if you need to connect to a generic LDAP server");
            this.button14.Image = global::Vmware.Tools.RestSsoAdminSnapIn.Properties.Resources.help;
            this.button14.Location = new System.Drawing.Point(135, 197);
            this.button14.Name = "button14";
            this.helpProvider1.SetShowHelp(this.button14, true);
            this.button14.Size = new System.Drawing.Size(24, 24);
            this.button14.TabIndex = 24;
            this.button14.UseVisualStyleBackColor = true;
            this.button14.Click += new System.EventHandler(this.button14_Click);
            // 
            // button13
            // 
            this.helpProvider1.SetHelpString(this.button13, "Choose this option if the users will authenticate to Active Directory using LDAP");
            this.button13.Image = global::Vmware.Tools.RestSsoAdminSnapIn.Properties.Resources.help;
            this.button13.Location = new System.Drawing.Point(274, 146);
            this.button13.Name = "button13";
            this.helpProvider1.SetShowHelp(this.button13, true);
            this.button13.Size = new System.Drawing.Size(24, 24);
            this.button13.TabIndex = 23;
            this.button13.UseVisualStyleBackColor = true;
            this.button13.Click += new System.EventHandler(this.button13_Click);
            // 
            // button12
            // 
            this.helpProvider1.SetHelpString(this.button12, "Choose this option if the users will be authenticated automatically using the cli" +
        "ent integration plugin");
            this.button12.Image = global::Vmware.Tools.RestSsoAdminSnapIn.Properties.Resources.help;
            this.button12.Location = new System.Drawing.Point(381, 96);
            this.button12.Name = "button12";
            this.helpProvider1.SetShowHelp(this.button12, true);
            this.button12.Size = new System.Drawing.Size(24, 24);
            this.button12.TabIndex = 22;
            this.button12.UseVisualStyleBackColor = true;
            this.button12.Click += new System.EventHandler(this.button12_Click);
            // 
            // button11
            // 
            this.button11.Enabled = false;
            this.button11.Location = new System.Drawing.Point(165, 290);
            this.button11.Name = "button11";
            this.button11.Size = new System.Drawing.Size(75, 23);
            this.button11.TabIndex = 21;
            this.button11.Text = "Back";
            this.button11.UseVisualStyleBackColor = true;
            this.button11.Click += new System.EventHandler(this.button11_Click);
            // 
            // rdoopenLdap
            // 
            this.rdoopenLdap.AutoSize = true;
            this.rdoopenLdap.Location = new System.Drawing.Point(22, 199);
            this.rdoopenLdap.Name = "rdoopenLdap";
            this.rdoopenLdap.Size = new System.Drawing.Size(114, 17);
            this.rdoopenLdap.TabIndex = 6;
            this.rdoopenLdap.TabStop = true;
            this.rdoopenLdap.Text = "Open LDAP server";
            this.rdoopenLdap.UseVisualStyleBackColor = true;
            this.rdoopenLdap.CheckedChanged += new System.EventHandler(this.rdoopenLdap_CheckedChanged);
            // 
            // rdoADLdap
            // 
            this.rdoADLdap.AutoSize = true;
            this.rdoADLdap.Location = new System.Drawing.Point(22, 148);
            this.rdoADLdap.Name = "rdoADLdap";
            this.rdoADLdap.Size = new System.Drawing.Size(255, 17);
            this.rdoADLdap.TabIndex = 4;
            this.rdoADLdap.TabStop = true;
            this.rdoADLdap.Text = "Microsoft Active Directory (R) as an LDAP server";
            this.rdoADLdap.UseVisualStyleBackColor = true;
            this.rdoADLdap.CheckedChanged += new System.EventHandler(this.rdoADLdap_CheckedChanged);
            // 
            // rdoADWindowsAuth
            // 
            this.rdoADWindowsAuth.AutoSize = true;
            this.rdoADWindowsAuth.Enabled = false;
            this.rdoADWindowsAuth.Location = new System.Drawing.Point(22, 99);
            this.rdoADWindowsAuth.Name = "rdoADWindowsAuth";
            this.rdoADWindowsAuth.Size = new System.Drawing.Size(360, 17);
            this.rdoADWindowsAuth.TabIndex = 1;
            this.rdoADWindowsAuth.TabStop = true;
            this.rdoADWindowsAuth.Text = "Microsoft Active Directory (R) using Windows Integrated Authentication";
            this.rdoADWindowsAuth.UseVisualStyleBackColor = true;
            this.rdoADWindowsAuth.CheckedChanged += new System.EventHandler(this.rdoADWindowsAuth_CheckedChanged);
            // 
            // lblCaption
            // 
            this.lblCaption.AutoSize = true;
            this.lblCaption.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblCaption.Location = new System.Drawing.Point(19, 37);
            this.lblCaption.Name = "lblCaption";
            this.lblCaption.Size = new System.Drawing.Size(302, 13);
            this.lblCaption.TabIndex = 0;
            this.lblCaption.Text = "What type of Identity Source would you like to add?";
            // 
            // pnlStep2
            // 
            this.pnlStep2.Controls.Add(this.txtFriendlyName);
            this.pnlStep2.Controls.Add(this.label6);
            this.pnlStep2.Controls.Add(this.pnlConnectionString);
            this.pnlStep2.Controls.Add(this.pnlProtect);
            this.pnlStep2.Controls.Add(this.button16);
            this.pnlStep2.Controls.Add(this.button3);
            this.pnlStep2.Controls.Add(this.button4);
            this.pnlStep2.Controls.Add(this.txtGroupDN);
            this.pnlStep2.Controls.Add(this.label5);
            this.pnlStep2.Controls.Add(this.txtUserDN);
            this.pnlStep2.Controls.Add(this.label4);
            this.pnlStep2.Controls.Add(this.txtDomainAlias);
            this.pnlStep2.Controls.Add(this.label3);
            this.pnlStep2.Controls.Add(this.txtDomainName);
            this.pnlStep2.Controls.Add(this.label2);
            this.pnlStep2.Location = new System.Drawing.Point(5, 5);
            this.pnlStep2.Name = "pnlStep2";
            this.pnlStep2.Size = new System.Drawing.Size(492, 317);
            this.pnlStep2.TabIndex = 10;
            this.pnlStep2.Paint += new System.Windows.Forms.PaintEventHandler(this.pnlStep2_Paint);
            // 
            // txtFriendlyName
            // 
            this.txtFriendlyName.ForeColor = System.Drawing.Color.Gray;
            this.txtFriendlyName.Location = new System.Drawing.Point(124, 5);
            this.txtFriendlyName.Name = "txtFriendlyName";
            this.txtFriendlyName.Size = new System.Drawing.Size(350, 20);
            this.txtFriendlyName.TabIndex = 30;
            this.txtFriendlyName.Text = "Contoso Domain";
            this.txtFriendlyName.Enter += new System.EventHandler(this.txtFriendlyName_Enter);
            this.txtFriendlyName.Leave += new System.EventHandler(this.txtFriendlyName_Leave);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(40, 8);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(75, 13);
            this.label6.TabIndex = 29;
            this.label6.Text = "Friendly name:";
            // 
            // pnlConnectionString
            // 
            this.pnlConnectionString.Controls.Add(this.txtPrimaryConnectionString);
            this.pnlConnectionString.Controls.Add(this.txtSecondaryConnectionString);
            this.pnlConnectionString.Controls.Add(this.label8);
            this.pnlConnectionString.Controls.Add(this.label9);
            this.pnlConnectionString.Controls.Add(this.button1);
            this.pnlConnectionString.Controls.Add(this.button2);
            this.pnlConnectionString.Location = new System.Drawing.Point(21, 159);
            this.pnlConnectionString.Name = "pnlConnectionString";
            this.pnlConnectionString.Size = new System.Drawing.Size(455, 55);
            this.pnlConnectionString.TabIndex = 27;
            // 
            // txtPrimaryConnectionString
            // 
            this.txtPrimaryConnectionString.ForeColor = System.Drawing.Color.Gray;
            this.txtPrimaryConnectionString.Location = new System.Drawing.Point(159, 6);
            this.txtPrimaryConnectionString.Name = "txtPrimaryConnectionString";
            this.txtPrimaryConnectionString.Size = new System.Drawing.Size(228, 20);
            this.txtPrimaryConnectionString.TabIndex = 11;
            this.txtPrimaryConnectionString.Text = "ldap://contoso.com:389 or ldaps://contoso.com:3268";
            this.txtPrimaryConnectionString.TextChanged += new System.EventHandler(this.txtPrimaryConnectionString_TextChanged);
            this.txtPrimaryConnectionString.Enter += new System.EventHandler(this.txtPrimaryConnectionString_Enter);
            this.txtPrimaryConnectionString.Leave += new System.EventHandler(this.txtPrimaryConnectionString_Leave);
            // 
            // txtSecondaryConnectionString
            // 
            this.txtSecondaryConnectionString.ForeColor = System.Drawing.Color.Gray;
            this.txtSecondaryConnectionString.Location = new System.Drawing.Point(159, 31);
            this.txtSecondaryConnectionString.Name = "txtSecondaryConnectionString";
            this.txtSecondaryConnectionString.Size = new System.Drawing.Size(228, 20);
            this.txtSecondaryConnectionString.TabIndex = 22;
            this.txtSecondaryConnectionString.Text = "ldap://contoso.com:389 or ldaps://contoso.com:3268";
            this.txtSecondaryConnectionString.TextChanged += new System.EventHandler(this.txtSecondaryConnectionString_TextChanged);
            this.txtSecondaryConnectionString.Enter += new System.EventHandler(this.txtSecondaryConnectionString_Enter);
            this.txtSecondaryConnectionString.Leave += new System.EventHandler(this.txtSecondaryConnectionString_Leave);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label8.Location = new System.Drawing.Point(49, 36);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(107, 13);
            this.label8.TabIndex = 21;
            this.label8.Text = "Secondary (optional):";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label9.Location = new System.Drawing.Point(110, 9);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(44, 13);
            this.label9.TabIndex = 10;
            this.label9.Text = "Primary:";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(393, 4);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(59, 22);
            this.button1.TabIndex = 22;
            this.button1.Text = "Import";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(394, 29);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(59, 22);
            this.button2.TabIndex = 24;
            this.button2.Text = "Import";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click_1);
            // 
            // pnlProtect
            // 
            this.pnlProtect.Controls.Add(this.label1);
            this.pnlProtect.Controls.Add(this.chkProtect);
            this.pnlProtect.Controls.Add(this.txtConnectionString);
            this.pnlProtect.Controls.Add(this.rdoSpecificDomain);
            this.pnlProtect.Controls.Add(this.rdoAnyDomain);
            this.pnlProtect.Location = new System.Drawing.Point(3, 136);
            this.pnlProtect.Name = "pnlProtect";
            this.pnlProtect.Size = new System.Drawing.Size(480, 147);
            this.pnlProtect.TabIndex = 28;
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(92, 126);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(89, 13);
            this.label1.TabIndex = 21;
            this.label1.Text = "Connection URL:";
            // 
            // chkProtect
            // 
            this.chkProtect.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.chkProtect.AutoSize = true;
            this.chkProtect.Location = new System.Drawing.Point(140, 101);
            this.chkProtect.Name = "chkProtect";
            this.chkProtect.Size = new System.Drawing.Size(309, 17);
            this.chkProtect.TabIndex = 19;
            this.chkProtect.Text = "Protect LDAP communication using SSL certificate (LDAPS)";
            this.chkProtect.UseVisualStyleBackColor = true;
            this.chkProtect.CheckedChanged += new System.EventHandler(this.chkProtect_CheckedChanged);
            // 
            // txtConnectionString
            // 
            this.txtConnectionString.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.txtConnectionString.Location = new System.Drawing.Point(183, 123);
            this.txtConnectionString.Name = "txtConnectionString";
            this.txtConnectionString.ReadOnly = true;
            this.txtConnectionString.Size = new System.Drawing.Size(293, 20);
            this.txtConnectionString.TabIndex = 20;
            this.txtConnectionString.Text = "ldap://contonso.com";
            this.txtConnectionString.TextChanged += new System.EventHandler(this.txtConnectionString_TextChanged);
            // 
            // rdoSpecificDomain
            // 
            this.rdoSpecificDomain.AutoSize = true;
            this.rdoSpecificDomain.Location = new System.Drawing.Point(120, 5);
            this.rdoSpecificDomain.Name = "rdoSpecificDomain";
            this.rdoSpecificDomain.Size = new System.Drawing.Size(204, 17);
            this.rdoSpecificDomain.TabIndex = 18;
            this.rdoSpecificDomain.TabStop = true;
            this.rdoSpecificDomain.Text = "Connect to specific domain controllers";
            this.rdoSpecificDomain.UseVisualStyleBackColor = true;
            this.rdoSpecificDomain.CheckedChanged += new System.EventHandler(this.rdoSpecificDomain_CheckedChanged);
            // 
            // rdoAnyDomain
            // 
            this.rdoAnyDomain.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.rdoAnyDomain.AutoSize = true;
            this.rdoAnyDomain.Location = new System.Drawing.Point(119, 80);
            this.rdoAnyDomain.Name = "rdoAnyDomain";
            this.rdoAnyDomain.Size = new System.Drawing.Size(246, 17);
            this.rdoAnyDomain.TabIndex = 10;
            this.rdoAnyDomain.TabStop = true;
            this.rdoAnyDomain.Text = "Connect to any domain controller in the domain";
            this.rdoAnyDomain.UseVisualStyleBackColor = true;
            this.rdoAnyDomain.CheckedChanged += new System.EventHandler(this.rdoAnyDomain_CheckedChanged);
            // 
            // button16
            // 
            this.helpProvider1.SetHelpString(this.button16, "This will be the suffix of User Principal Names (UPN) by which the users will aut" +
        "henticate from this identity sourc");
            this.button16.Image = global::Vmware.Tools.RestSsoAdminSnapIn.Properties.Resources.help;
            this.button16.Location = new System.Drawing.Point(12, 27);
            this.button16.Margin = new System.Windows.Forms.Padding(0);
            this.button16.Name = "button16";
            this.helpProvider1.SetShowHelp(this.button16, true);
            this.button16.Size = new System.Drawing.Size(24, 24);
            this.button16.TabIndex = 26;
            this.button16.UseVisualStyleBackColor = true;
            this.button16.Click += new System.EventHandler(this.button16_Click);
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(153, 288);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(75, 23);
            this.button3.TabIndex = 11;
            this.button3.Text = "Back";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // button4
            // 
            this.button4.Location = new System.Drawing.Point(234, 289);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(75, 22);
            this.button4.TabIndex = 10;
            this.button4.Text = "Next";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.button4_Click);
            // 
            // txtGroupDN
            // 
            this.txtGroupDN.ForeColor = System.Drawing.Color.Gray;
            this.txtGroupDN.Location = new System.Drawing.Point(124, 113);
            this.txtGroupDN.Name = "txtGroupDN";
            this.txtGroupDN.Size = new System.Drawing.Size(350, 20);
            this.txtGroupDN.TabIndex = 17;
            this.txtGroupDN.Text = "CN=groups, CN=Configuration, DC=contoso, DC=com";
            this.txtGroupDN.Enter += new System.EventHandler(this.txtGroupDN_Enter);
            this.txtGroupDN.Leave += new System.EventHandler(this.txtGroupDN_Leave);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(16, 116);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(100, 13);
            this.label5.TabIndex = 16;
            this.label5.Text = "Base DN of groups:";
            // 
            // txtUserDN
            // 
            this.txtUserDN.ForeColor = System.Drawing.Color.Gray;
            this.txtUserDN.Location = new System.Drawing.Point(124, 86);
            this.txtUserDN.Name = "txtUserDN";
            this.txtUserDN.Size = new System.Drawing.Size(350, 20);
            this.txtUserDN.TabIndex = 15;
            this.txtUserDN.Text = "CN=users, CN=Configuration, DC=contoso, DC=com";
            this.txtUserDN.Enter += new System.EventHandler(this.txtUserDN_Enter);
            this.txtUserDN.Leave += new System.EventHandler(this.txtUserDN_Leave);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(25, 89);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(93, 13);
            this.label4.TabIndex = 14;
            this.label4.Text = "Base DN of users:";
            // 
            // txtDomainAlias
            // 
            this.txtDomainAlias.ForeColor = System.Drawing.Color.Gray;
            this.txtDomainAlias.Location = new System.Drawing.Point(124, 58);
            this.txtDomainAlias.Name = "txtDomainAlias";
            this.txtDomainAlias.Size = new System.Drawing.Size(350, 20);
            this.txtDomainAlias.TabIndex = 13;
            this.txtDomainAlias.Text = "Contoso";
            this.txtDomainAlias.Enter += new System.EventHandler(this.txtDomainAlias_Enter);
            this.txtDomainAlias.Leave += new System.EventHandler(this.txtDomainAlias_Leave);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(42, 61);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(71, 13);
            this.label3.TabIndex = 12;
            this.label3.Text = "Domain Alias:";
            // 
            // txtDomainName
            // 
            this.txtDomainName.ForeColor = System.Drawing.Color.Gray;
            this.txtDomainName.Location = new System.Drawing.Point(124, 31);
            this.txtDomainName.Name = "txtDomainName";
            this.txtDomainName.Size = new System.Drawing.Size(350, 20);
            this.txtDomainName.TabIndex = 11;
            this.txtDomainName.Text = "contoso.com";
            this.txtDomainName.TextChanged += new System.EventHandler(this.txtDomainName_TextChanged);
            this.txtDomainName.Enter += new System.EventHandler(this.txtDomainName_Enter);
            this.txtDomainName.Leave += new System.EventHandler(this.txtDomainName_Leave);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(40, 34);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Domain name:";
            // 
            // pnlStep3
            // 
            this.pnlStep3.Controls.Add(this.button18);
            this.pnlStep3.Controls.Add(this.button15);
            this.pnlStep3.Controls.Add(this.button6);
            this.pnlStep3.Controls.Add(this.button7);
            this.pnlStep3.Controls.Add(this.lstCertificateChain);
            this.pnlStep3.Controls.Add(this.btnRemoveCert);
            this.pnlStep3.Controls.Add(this.label11);
            this.pnlStep3.Controls.Add(this.btnAddCert);
            this.pnlStep3.Location = new System.Drawing.Point(6, 4);
            this.pnlStep3.Name = "pnlStep3";
            this.pnlStep3.Size = new System.Drawing.Size(491, 317);
            this.pnlStep3.TabIndex = 21;
            // 
            // button18
            // 
            this.helpProvider1.SetHelpString(this.button18, "Choose this option if the users will authenticate to Active Directory using LDAP");
            this.button18.Image = global::Vmware.Tools.RestSsoAdminSnapIn.Properties.Resources.help;
            this.button18.Location = new System.Drawing.Point(91, 23);
            this.button18.Name = "button18";
            this.helpProvider1.SetShowHelp(this.button18, true);
            this.button18.Size = new System.Drawing.Size(24, 24);
            this.button18.TabIndex = 44;
            this.button18.UseVisualStyleBackColor = true;
            this.button18.Click += new System.EventHandler(this.button18_Click);
            // 
            // button15
            // 
            this.button15.Location = new System.Drawing.Point(417, 77);
            this.button15.Name = "button15";
            this.button15.Size = new System.Drawing.Size(58, 23);
            this.button15.TabIndex = 43;
            this.button15.Text = "View";
            this.button15.UseVisualStyleBackColor = true;
            this.button15.Click += new System.EventHandler(this.button15_Click);
            // 
            // button6
            // 
            this.button6.Location = new System.Drawing.Point(150, 290);
            this.button6.Name = "button6";
            this.button6.Size = new System.Drawing.Size(75, 23);
            this.button6.TabIndex = 42;
            this.button6.Text = "Back";
            this.button6.UseVisualStyleBackColor = true;
            this.button6.Click += new System.EventHandler(this.button6_Click);
            // 
            // button7
            // 
            this.button7.Location = new System.Drawing.Point(231, 291);
            this.button7.Name = "button7";
            this.button7.Size = new System.Drawing.Size(75, 22);
            this.button7.TabIndex = 41;
            this.button7.Text = "Next";
            this.button7.UseVisualStyleBackColor = true;
            this.button7.Click += new System.EventHandler(this.button7_Click);
            // 
            // lstCertificateChain
            // 
            this.lstCertificateChain.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader5});
            this.lstCertificateChain.FullRowSelect = true;
            this.lstCertificateChain.GridLines = true;
            this.lstCertificateChain.Location = new System.Drawing.Point(18, 50);
            this.lstCertificateChain.MultiSelect = false;
            this.lstCertificateChain.Name = "lstCertificateChain";
            this.lstCertificateChain.Size = new System.Drawing.Size(395, 224);
            this.lstCertificateChain.TabIndex = 40;
            this.lstCertificateChain.TabStop = false;
            this.lstCertificateChain.UseCompatibleStateImageBehavior = false;
            this.lstCertificateChain.View = System.Windows.Forms.View.Details;
            this.lstCertificateChain.SelectedIndexChanged += new System.EventHandler(this.lstCertificateChain_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Subject";
            this.columnHeader1.Width = 100;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Issuer";
            this.columnHeader2.Width = 150;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Valid From";
            this.columnHeader3.Width = 80;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Valid To";
            this.columnHeader4.Width = 80;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Thumbprint";
            this.columnHeader5.Width = 150;
            // 
            // btnRemoveCert
            // 
            this.btnRemoveCert.Enabled = false;
            this.btnRemoveCert.Location = new System.Drawing.Point(418, 251);
            this.btnRemoveCert.Name = "btnRemoveCert";
            this.btnRemoveCert.Size = new System.Drawing.Size(59, 23);
            this.btnRemoveCert.TabIndex = 39;
            this.btnRemoveCert.Text = "Remove";
            this.btnRemoveCert.UseVisualStyleBackColor = true;
            this.btnRemoveCert.Click += new System.EventHandler(this.btnRemoveCert_Click);
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label11.Location = new System.Drawing.Point(19, 29);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(71, 13);
            this.label11.TabIndex = 25;
            this.label11.Text = "Certificates";
            // 
            // btnAddCert
            // 
            this.btnAddCert.Location = new System.Drawing.Point(417, 50);
            this.btnAddCert.Name = "btnAddCert";
            this.btnAddCert.Size = new System.Drawing.Size(58, 23);
            this.btnAddCert.TabIndex = 38;
            this.btnAddCert.Text = "Add";
            this.btnAddCert.UseVisualStyleBackColor = true;
            this.btnAddCert.Click += new System.EventHandler(this.btnAddCert_Click);
            // 
            // pnlStep4
            // 
            this.pnlStep4.Controls.Add(this.radioButton2);
            this.pnlStep4.Controls.Add(this.radioButton1);
            this.pnlStep4.Controls.Add(this.txtSPN);
            this.pnlStep4.Controls.Add(this.lblSPN);
            this.pnlStep4.Controls.Add(this.button17);
            this.pnlStep4.Controls.Add(this.button10);
            this.pnlStep4.Controls.Add(this.button8);
            this.pnlStep4.Controls.Add(this.button9);
            this.pnlStep4.Controls.Add(this.button5);
            this.pnlStep4.Controls.Add(this.label7);
            this.pnlStep4.Controls.Add(this.txtPassword);
            this.pnlStep4.Controls.Add(this.label12);
            this.pnlStep4.Controls.Add(this.txtUsername);
            this.pnlStep4.Controls.Add(this.label13);
            this.pnlStep4.Location = new System.Drawing.Point(6, 5);
            this.pnlStep4.Name = "pnlStep4";
            this.pnlStep4.Size = new System.Drawing.Size(491, 317);
            this.pnlStep4.TabIndex = 41;
            // 
            // radioButton2
            // 
            this.radioButton2.AutoSize = true;
            this.radioButton2.Location = new System.Drawing.Point(83, 111);
            this.radioButton2.Name = "radioButton2";
            this.radioButton2.Size = new System.Drawing.Size(109, 17);
            this.radioButton2.TabIndex = 32;
            this.radioButton2.TabStop = true;
            this.radioButton2.Text = "Use user account";
            this.radioButton2.UseVisualStyleBackColor = true;
            this.radioButton2.CheckedChanged += new System.EventHandler(this.radioButton2_CheckedChanged);
            // 
            // radioButton1
            // 
            this.radioButton1.AutoSize = true;
            this.radioButton1.Location = new System.Drawing.Point(83, 87);
            this.radioButton1.Name = "radioButton1";
            this.radioButton1.Size = new System.Drawing.Size(129, 17);
            this.radioButton1.TabIndex = 29;
            this.radioButton1.TabStop = true;
            this.radioButton1.Text = "Use machine account";
            this.radioButton1.UseVisualStyleBackColor = true;
            this.radioButton1.CheckedChanged += new System.EventHandler(this.radioButton1_CheckedChanged);
            // 
            // txtSPN
            // 
            this.txtSPN.ForeColor = System.Drawing.Color.Gray;
            this.txtSPN.Location = new System.Drawing.Point(83, 138);
            this.txtSPN.Name = "txtSPN";
            this.txtSPN.Size = new System.Drawing.Size(249, 20);
            this.txtSPN.TabIndex = 31;
            this.txtSPN.Text = "sts/contoso.com";
            this.txtSPN.Enter += new System.EventHandler(this.txtSPN_Enter);
            this.txtSPN.Leave += new System.EventHandler(this.txtSPN_Leave);
            // 
            // lblSPN
            // 
            this.lblSPN.AutoSize = true;
            this.lblSPN.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblSPN.Location = new System.Drawing.Point(46, 141);
            this.lblSPN.Name = "lblSPN";
            this.lblSPN.Size = new System.Drawing.Size(32, 13);
            this.lblSPN.TabIndex = 30;
            this.lblSPN.Text = "SPN:";
            // 
            // button17
            // 
            this.button17.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("button17.BackgroundImage")));
            this.button17.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.helpProvider1.SetHelpString(this.button17, "These credentials will be used to fetch the user and group information from the d" +
        "omain");
            this.button17.Location = new System.Drawing.Point(245, 33);
            this.button17.Name = "button17";
            this.helpProvider1.SetShowHelp(this.button17, true);
            this.button17.Size = new System.Drawing.Size(24, 24);
            this.button17.TabIndex = 29;
            this.button17.TabStop = false;
            this.button17.UseVisualStyleBackColor = false;
            this.button17.Click += new System.EventHandler(this.button17_Click);
            // 
            // button10
            // 
            this.button10.Location = new System.Drawing.Point(370, 196);
            this.button10.Name = "button10";
            this.button10.Size = new System.Drawing.Size(106, 22);
            this.button10.TabIndex = 27;
            this.button10.Text = "Advanced ...";
            this.button10.UseVisualStyleBackColor = true;
            this.button10.Click += new System.EventHandler(this.button10_Click);
            // 
            // button8
            // 
            this.button8.Location = new System.Drawing.Point(169, 290);
            this.button8.Name = "button8";
            this.button8.Size = new System.Drawing.Size(75, 23);
            this.button8.TabIndex = 26;
            this.button8.Text = "Back";
            this.button8.UseVisualStyleBackColor = true;
            this.button8.Click += new System.EventHandler(this.button8_Click);
            // 
            // button9
            // 
            this.button9.Location = new System.Drawing.Point(250, 291);
            this.button9.Name = "button9";
            this.button9.Size = new System.Drawing.Size(75, 22);
            this.button9.TabIndex = 25;
            this.button9.Text = "Save";
            this.button9.UseVisualStyleBackColor = true;
            this.button9.Click += new System.EventHandler(this.button9_Click);
            // 
            // button5
            // 
            this.button5.Location = new System.Drawing.Point(370, 167);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(106, 22);
            this.button5.TabIndex = 24;
            this.button5.Text = "Test Connection";
            this.button5.UseVisualStyleBackColor = true;
            this.button5.Click += new System.EventHandler(this.button5_Click);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(19, 37);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(225, 13);
            this.label7.TabIndex = 23;
            this.label7.Text = "Credentials for user account in domain";
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(83, 198);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(249, 20);
            this.txtPassword.TabIndex = 22;
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label12.Location = new System.Drawing.Point(21, 201);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(56, 13);
            this.label12.TabIndex = 21;
            this.label12.Text = "Password:";
            // 
            // txtUsername
            // 
            this.txtUsername.ForeColor = System.Drawing.Color.Gray;
            this.txtUsername.Location = new System.Drawing.Point(83, 168);
            this.txtUsername.Name = "txtUsername";
            this.txtUsername.Size = new System.Drawing.Size(249, 20);
            this.txtUsername.TabIndex = 11;
            this.txtUsername.Text = "CN=Administrator,CN=users,DC=contoso,DC=com";
            this.txtUsername.Enter += new System.EventHandler(this.txtUsername_Enter);
            this.txtUsername.Leave += new System.EventHandler(this.txtUsername_Leave);
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label13.Location = new System.Drawing.Point(21, 171);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(58, 13);
            this.label13.TabIndex = 10;
            this.label13.Text = "Username:";
            // 
            // AddExternalDomain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(506, 331);
            this.Controls.Add(this.pnlStep2);
            this.Controls.Add(this.pnlStep3);
            this.Controls.Add(this.pnlStep1);
            this.Controls.Add(this.pnlStep4);
            this.Name = "AddExternalDomain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Add Identity Source Wizard";
            this.Load += new System.EventHandler(this.AddExternalDomain_Load);
            this.pnlStep1.ResumeLayout(false);
            this.pnlStep1.PerformLayout();
            this.pnlStep2.ResumeLayout(false);
            this.pnlStep2.PerformLayout();
            this.pnlConnectionString.ResumeLayout(false);
            this.pnlConnectionString.PerformLayout();
            this.pnlProtect.ResumeLayout(false);
            this.pnlProtect.PerformLayout();
            this.pnlStep3.ResumeLayout(false);
            this.pnlStep3.PerformLayout();
            this.pnlStep4.ResumeLayout(false);
            this.pnlStep4.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnNext;
        private System.Windows.Forms.Panel pnlStep1;
        private System.Windows.Forms.RadioButton rdoADWindowsAuth;
        private System.Windows.Forms.Label lblCaption;
        private System.Windows.Forms.RadioButton rdoopenLdap;
        private System.Windows.Forms.RadioButton rdoADLdap;
        private System.Windows.Forms.Panel pnlStep2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtDomainAlias;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtDomainName;
        private System.Windows.Forms.TextBox txtConnectionString;
        private System.Windows.Forms.CheckBox chkProtect;
        private System.Windows.Forms.RadioButton rdoSpecificDomain;
        private System.Windows.Forms.RadioButton rdoAnyDomain;
        private System.Windows.Forms.TextBox txtGroupDN;                 
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox txtUserDN;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Panel pnlStep3;
        private System.Windows.Forms.TextBox txtPrimaryConnectionString;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox txtSecondaryConnectionString;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ListView lstCertificateChain;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.Button btnRemoveCert;
        private System.Windows.Forms.Button btnAddCert;
        private System.Windows.Forms.Panel pnlStep4;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox txtUsername;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Button button6;
        private System.Windows.Forms.Button button7;
        private System.Windows.Forms.Button button10;
        private System.Windows.Forms.Button button8;
        private System.Windows.Forms.Button button9;
        private System.Windows.Forms.Button button11;
        private System.Windows.Forms.Button button12;
        private System.Windows.Forms.Button button14;
        private System.Windows.Forms.Button button13;
        private System.Windows.Forms.Button button16;
        private System.Windows.Forms.Button button17;
        private System.Windows.Forms.Panel pnlProtect;
        private System.Windows.Forms.TextBox txtSPN;
        private System.Windows.Forms.Label lblSPN;
        private System.Windows.Forms.RadioButton radioButton2;
        private System.Windows.Forms.RadioButton radioButton1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.HelpProvider helpProvider1;
        private Installer.RestSsoAdminInstaller restSsoAdminInstaller1;
        private System.Windows.Forms.Button button15;
        private System.Windows.Forms.Panel pnlConnectionString;
        private System.Windows.Forms.Button button18;
        private System.Windows.Forms.TextBox txtFriendlyName;
        private System.Windows.Forms.Label label6;
    }
}