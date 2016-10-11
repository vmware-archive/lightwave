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
    partial class NewRelyingParty
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
            this.horizontalLine = new System.Windows.Forms.GroupBox();
            this.btnCreate = new System.Windows.Forms.Button();
            this.btnClose = new System.Windows.Forms.Button();
            this.txtName = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.btnRemoveSignatureAlgorithm = new System.Windows.Forms.Button();
            this.btnAddSignatureAlgorithm = new System.Windows.Forms.Button();
            this.txtUrl = new System.Windows.Forms.TextBox();
            this.txtCertificateFilePath = new System.Windows.Forms.TextBox();
            this.btnChooseCertificate = new System.Windows.Forms.Button();
            this.btnRemoveAssertionConsumerService = new System.Windows.Forms.Button();
            this.btnAddAssertionConsumerService = new System.Windows.Forms.Button();
            this.btnRemoveAttributeConsumerService = new System.Windows.Forms.Button();
            this.btnAddAttributeConsumerService = new System.Windows.Forms.Button();
            this.btnAddSloService = new System.Windows.Forms.Button();
            this.btnRemoveSloService = new System.Windows.Forms.Button();
            this.chkSigned = new System.Windows.Forms.CheckBox();
            this.btnViewCertificate = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.label7 = new System.Windows.Forms.Label();
            this.lstSignatureAlgorithms = new System.Windows.Forms.ListView();
            this.maxKeySize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.minKeySize = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.clmPriority = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label1 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.lstAssertionConsumerServices = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader10 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label6 = new System.Windows.Forms.Label();
            this.lstAttributeConsumerServices = new System.Windows.Forms.ListView();
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader9 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.lstSloServices = new System.Windows.Forms.ListView();
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader8 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader11 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // horizontalLine
            // 
            this.horizontalLine.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.horizontalLine.Location = new System.Drawing.Point(11, 550);
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.Size = new System.Drawing.Size(365, 3);
            this.horizontalLine.TabIndex = 7;
            this.horizontalLine.TabStop = false;
            // 
            // btnCreate
            // 
            this.btnCreate.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.helpProvider1.SetHelpKeyword(this.btnCreate, "SignerIdentityCreate");
            this.helpProvider1.SetHelpString(this.btnCreate, "Add/Update relying party for the tenant");
            this.btnCreate.Location = new System.Drawing.Point(220, 558);
            this.btnCreate.Name = "btnCreate";
            this.helpProvider1.SetShowHelp(this.btnCreate, true);
            this.btnCreate.Size = new System.Drawing.Size(75, 23);
            this.btnCreate.TabIndex = 14;
            this.btnCreate.Text = "&Create";
            this.toolTip1.SetToolTip(this.btnCreate, "Add/Update relying party for the tenant");
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreateSignerIdentity_Click);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(301, 558);
            this.btnClose.Name = "btnClose";
            this.helpProvider1.SetShowHelp(this.btnClose, true);
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 15;
            this.btnClose.Text = "Cl&ose";
            this.toolTip1.SetToolTip(this.btnClose, "Click to close the window. All the changes made will be lost.");
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // txtName
            // 
            this.helpProvider1.SetHelpKeyword(this.txtName, "New Relying Party");
            this.helpProvider1.SetHelpString(this.txtName, "Displays the name of the relying party for the tenant");
            this.txtName.Location = new System.Drawing.Point(69, 11);
            this.txtName.Name = "txtName";
            this.helpProvider1.SetShowHelp(this.txtName, true);
            this.txtName.Size = new System.Drawing.Size(306, 20);
            this.txtName.TabIndex = 1;
            this.toolTip1.SetToolTip(this.txtName, "Name of Relying party");
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 14);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(38, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "Name:";
            // 
            // btnRemoveSignatureAlgorithm
            // 
            this.btnRemoveSignatureAlgorithm.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveSignatureAlgorithm, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnRemoveSignatureAlgorithm, "Select and click to remove");
            this.btnRemoveSignatureAlgorithm.Location = new System.Drawing.Point(351, 192);
            this.btnRemoveSignatureAlgorithm.Name = "btnRemoveSignatureAlgorithm";
            this.helpProvider1.SetShowHelp(this.btnRemoveSignatureAlgorithm, true);
            this.btnRemoveSignatureAlgorithm.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveSignatureAlgorithm.TabIndex = 7;
            this.btnRemoveSignatureAlgorithm.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveSignatureAlgorithm, "Select and click to remove");
            this.btnRemoveSignatureAlgorithm.UseVisualStyleBackColor = true;
            this.btnRemoveSignatureAlgorithm.Click += new System.EventHandler(this.btnRemoveSignatureAlgorithm_Click);
            // 
            // btnAddSignatureAlgorithm
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddSignatureAlgorithm, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnAddSignatureAlgorithm, "Add Signature Algorithm");
            this.btnAddSignatureAlgorithm.Location = new System.Drawing.Point(351, 134);
            this.btnAddSignatureAlgorithm.Name = "btnAddSignatureAlgorithm";
            this.helpProvider1.SetShowHelp(this.btnAddSignatureAlgorithm, true);
            this.btnAddSignatureAlgorithm.Size = new System.Drawing.Size(24, 23);
            this.btnAddSignatureAlgorithm.TabIndex = 6;
            this.btnAddSignatureAlgorithm.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddSignatureAlgorithm, "Add Signature Algorithm");
            this.btnAddSignatureAlgorithm.UseVisualStyleBackColor = true;
            this.btnAddSignatureAlgorithm.Click += new System.EventHandler(this.btnAddSignatureAlgorithm_Click);
            // 
            // txtUrl
            // 
            this.helpProvider1.SetHelpKeyword(this.txtUrl, "New Relying Party");
            this.helpProvider1.SetHelpString(this.txtUrl, "Displays the URL of the relying party for the tenant");
            this.txtUrl.Location = new System.Drawing.Point(69, 38);
            this.txtUrl.Name = "txtUrl";
            this.helpProvider1.SetShowHelp(this.txtUrl, true);
            this.txtUrl.Size = new System.Drawing.Size(306, 20);
            this.txtUrl.TabIndex = 2;
            this.toolTip1.SetToolTip(this.txtUrl, "Displays URL of Relying party");
            // 
            // txtCertificateFilePath
            // 
            this.helpProvider1.SetHelpKeyword(this.txtCertificateFilePath, "RelyingParty");
            this.helpProvider1.SetHelpString(this.txtCertificateFilePath, "Certificate Path");
            this.txtCertificateFilePath.Location = new System.Drawing.Point(69, 64);
            this.txtCertificateFilePath.Name = "txtCertificateFilePath";
            this.txtCertificateFilePath.ReadOnly = true;
            this.helpProvider1.SetShowHelp(this.txtCertificateFilePath, true);
            this.txtCertificateFilePath.Size = new System.Drawing.Size(247, 20);
            this.txtCertificateFilePath.TabIndex = 3;
            this.toolTip1.SetToolTip(this.txtCertificateFilePath, "Certificate Path");
            // 
            // btnChooseCertificate
            // 
            this.helpProvider1.SetHelpKeyword(this.btnChooseCertificate, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnChooseCertificate, "Select a certificate from the chain and click to remove");
            this.btnChooseCertificate.Location = new System.Drawing.Point(322, 64);
            this.btnChooseCertificate.Name = "btnChooseCertificate";
            this.helpProvider1.SetShowHelp(this.btnChooseCertificate, true);
            this.btnChooseCertificate.Size = new System.Drawing.Size(24, 23);
            this.btnChooseCertificate.TabIndex = 4;
            this.btnChooseCertificate.Text = "...";
            this.toolTip1.SetToolTip(this.btnChooseCertificate, "Select a certificate from the chain and click to remove");
            this.btnChooseCertificate.UseVisualStyleBackColor = true;
            this.btnChooseCertificate.Click += new System.EventHandler(this.btnChooseCertificate_Click);
            // 
            // btnRemoveAssertionConsumerService
            // 
            this.btnRemoveAssertionConsumerService.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveAssertionConsumerService, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnRemoveAssertionConsumerService, "Select and click to remove");
            this.btnRemoveAssertionConsumerService.Location = new System.Drawing.Point(351, 297);
            this.btnRemoveAssertionConsumerService.Name = "btnRemoveAssertionConsumerService";
            this.helpProvider1.SetShowHelp(this.btnRemoveAssertionConsumerService, true);
            this.btnRemoveAssertionConsumerService.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveAssertionConsumerService.TabIndex = 9;
            this.btnRemoveAssertionConsumerService.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveAssertionConsumerService, "Select and click to remove");
            this.btnRemoveAssertionConsumerService.UseVisualStyleBackColor = true;
            this.btnRemoveAssertionConsumerService.Click += new System.EventHandler(this.btnRemoveAssertionConsumerService_Click);
            // 
            // btnAddAssertionConsumerService
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddAssertionConsumerService, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnAddAssertionConsumerService, "Add Assertion Consumer Service");
            this.btnAddAssertionConsumerService.Location = new System.Drawing.Point(351, 239);
            this.btnAddAssertionConsumerService.Name = "btnAddAssertionConsumerService";
            this.helpProvider1.SetShowHelp(this.btnAddAssertionConsumerService, true);
            this.btnAddAssertionConsumerService.Size = new System.Drawing.Size(24, 23);
            this.btnAddAssertionConsumerService.TabIndex = 8;
            this.btnAddAssertionConsumerService.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddAssertionConsumerService, "Add Assertion Consumer Service");
            this.btnAddAssertionConsumerService.UseVisualStyleBackColor = true;
            this.btnAddAssertionConsumerService.Click += new System.EventHandler(this.btnAddAssertionConsumerService_Click);
            // 
            // btnRemoveAttributeConsumerService
            // 
            this.btnRemoveAttributeConsumerService.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveAttributeConsumerService, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnRemoveAttributeConsumerService, "Select and click to remove");
            this.btnRemoveAttributeConsumerService.Location = new System.Drawing.Point(351, 405);
            this.btnRemoveAttributeConsumerService.Name = "btnRemoveAttributeConsumerService";
            this.helpProvider1.SetShowHelp(this.btnRemoveAttributeConsumerService, true);
            this.btnRemoveAttributeConsumerService.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveAttributeConsumerService.TabIndex = 11;
            this.btnRemoveAttributeConsumerService.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveAttributeConsumerService, "Select and click to remove");
            this.btnRemoveAttributeConsumerService.UseVisualStyleBackColor = true;
            this.btnRemoveAttributeConsumerService.Click += new System.EventHandler(this.btnRemoveAttributeConsumerService_Click);
            // 
            // btnAddAttributeConsumerService
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddAttributeConsumerService, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnAddAttributeConsumerService, "Add attribute consumer service");
            this.btnAddAttributeConsumerService.Location = new System.Drawing.Point(351, 347);
            this.btnAddAttributeConsumerService.Name = "btnAddAttributeConsumerService";
            this.helpProvider1.SetShowHelp(this.btnAddAttributeConsumerService, true);
            this.btnAddAttributeConsumerService.Size = new System.Drawing.Size(24, 23);
            this.btnAddAttributeConsumerService.TabIndex = 10;
            this.btnAddAttributeConsumerService.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddAttributeConsumerService, "Add attribute consumer service");
            this.btnAddAttributeConsumerService.UseVisualStyleBackColor = true;
            this.btnAddAttributeConsumerService.Click += new System.EventHandler(this.btnAddAttributeConsumerService_Click);
            // 
            // btnAddSloService
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddSloService, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnAddSloService, "Add Slo Service");
            this.btnAddSloService.Location = new System.Drawing.Point(351, 457);
            this.btnAddSloService.Name = "btnAddSloService";
            this.helpProvider1.SetShowHelp(this.btnAddSloService, true);
            this.btnAddSloService.Size = new System.Drawing.Size(24, 23);
            this.btnAddSloService.TabIndex = 12;
            this.btnAddSloService.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddSloService, "Add Slo Service");
            this.btnAddSloService.UseVisualStyleBackColor = true;
            this.btnAddSloService.Click += new System.EventHandler(this.btnAddSloService_Click);
            // 
            // btnRemoveSloService
            // 
            this.btnRemoveSloService.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveSloService, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnRemoveSloService, "Select and click to remove");
            this.btnRemoveSloService.Location = new System.Drawing.Point(351, 515);
            this.btnRemoveSloService.Name = "btnRemoveSloService";
            this.helpProvider1.SetShowHelp(this.btnRemoveSloService, true);
            this.btnRemoveSloService.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveSloService.TabIndex = 13;
            this.btnRemoveSloService.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveSloService, "Select and click to remove");
            this.btnRemoveSloService.UseVisualStyleBackColor = true;
            this.btnRemoveSloService.Click += new System.EventHandler(this.btnRemoveSloService_Click);
            // 
            // chkSigned
            // 
            this.chkSigned.AutoSize = true;
            this.helpProvider1.SetHelpKeyword(this.chkSigned, "RelyingParty");
            this.helpProvider1.SetHelpString(this.chkSigned, "Sign Auth requests?");
            this.chkSigned.Location = new System.Drawing.Point(69, 90);
            this.chkSigned.Name = "chkSigned";
            this.helpProvider1.SetShowHelp(this.chkSigned, true);
            this.chkSigned.Size = new System.Drawing.Size(121, 17);
            this.chkSigned.TabIndex = 5;
            this.chkSigned.Text = "Sign Auth requests?";
            this.chkSigned.UseVisualStyleBackColor = true;
            // 
            // btnViewCertificate
            // 
            this.helpProvider1.SetHelpKeyword(this.btnViewCertificate, "RelyingParty");
            this.helpProvider1.SetHelpString(this.btnViewCertificate, "View certificate");
            this.btnViewCertificate.Location = new System.Drawing.Point(352, 64);
            this.btnViewCertificate.Name = "btnViewCertificate";
            this.helpProvider1.SetShowHelp(this.btnViewCertificate, true);
            this.btnViewCertificate.Size = new System.Drawing.Size(24, 23);
            this.btnViewCertificate.TabIndex = 53;
            this.btnViewCertificate.Text = "V";
            this.toolTip1.SetToolTip(this.btnViewCertificate, "View certificate");
            this.btnViewCertificate.UseVisualStyleBackColor = true;
            this.btnViewCertificate.Click += new System.EventHandler(this.btnViewCertificate_Click);
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 2000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.ReshowDelay = 100;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(10, 439);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(116, 13);
            this.label7.TabIndex = 24;
            this.label7.Text = "Single Logout Services";
            // 
            // lstSignatureAlgorithms
            // 
            this.lstSignatureAlgorithms.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.maxKeySize,
            this.minKeySize,
            this.clmPriority});
            this.lstSignatureAlgorithms.FullRowSelect = true;
            this.lstSignatureAlgorithms.GridLines = true;
            this.lstSignatureAlgorithms.Location = new System.Drawing.Point(9, 134);
            this.lstSignatureAlgorithms.Name = "lstSignatureAlgorithms";
            this.lstSignatureAlgorithms.Size = new System.Drawing.Size(337, 81);
            this.lstSignatureAlgorithms.TabIndex = 34;
            this.lstSignatureAlgorithms.TabStop = false;
            this.lstSignatureAlgorithms.UseCompatibleStateImageBehavior = false;
            this.lstSignatureAlgorithms.View = System.Windows.Forms.View.Details;
            this.lstSignatureAlgorithms.SelectedIndexChanged += new System.EventHandler(this.lstSignatureAlgorithms_SelectedIndexChanged);
            // 
            // maxKeySize
            // 
            this.maxKeySize.Text = "MAX Key size";
            this.maxKeySize.Width = 110;
            // 
            // minKeySize
            // 
            this.minKeySize.Text = "MIN Key size";
            this.minKeySize.Width = 110;
            // 
            // clmPriority
            // 
            this.clmPriority.Text = "Priority";
            this.clmPriority.Width = 100;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 118);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(103, 13);
            this.label1.TabIndex = 35;
            this.label1.Text = "Signature Algorithms";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(9, 41);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(32, 13);
            this.label4.TabIndex = 39;
            this.label4.Text = "URL:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(6, 67);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(57, 13);
            this.label5.TabIndex = 41;
            this.label5.Text = "Certificate:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 223);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(144, 13);
            this.label2.TabIndex = 45;
            this.label2.Text = "Assertion Consumer Services";
            // 
            // lstAssertionConsumerServices
            // 
            this.lstAssertionConsumerServices.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader7,
            this.columnHeader10,
            this.columnHeader2,
            this.columnHeader3});
            this.lstAssertionConsumerServices.FullRowSelect = true;
            this.lstAssertionConsumerServices.GridLines = true;
            this.lstAssertionConsumerServices.Location = new System.Drawing.Point(9, 239);
            this.lstAssertionConsumerServices.Name = "lstAssertionConsumerServices";
            this.lstAssertionConsumerServices.Size = new System.Drawing.Size(337, 81);
            this.lstAssertionConsumerServices.TabIndex = 44;
            this.lstAssertionConsumerServices.TabStop = false;
            this.lstAssertionConsumerServices.UseCompatibleStateImageBehavior = false;
            this.lstAssertionConsumerServices.View = System.Windows.Forms.View.Details;
            this.lstAssertionConsumerServices.SelectedIndexChanged += new System.EventHandler(this.lstAssertionConsumerServices_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Name";
            this.columnHeader1.Width = 80;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "Index";
            this.columnHeader7.Width = 40;
            // 
            // columnHeader10
            // 
            this.columnHeader10.Text = "Default";
            this.columnHeader10.Width = 50;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Endpoint";
            this.columnHeader2.Width = 80;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Binding";
            this.columnHeader3.Width = 80;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(9, 331);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(140, 13);
            this.label6.TabIndex = 49;
            this.label6.Text = "Attribute Consumer Services";
            // 
            // lstAttributeConsumerServices
            // 
            this.lstAttributeConsumerServices.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader4,
            this.columnHeader6,
            this.columnHeader9});
            this.lstAttributeConsumerServices.FullRowSelect = true;
            this.lstAttributeConsumerServices.GridLines = true;
            this.lstAttributeConsumerServices.Location = new System.Drawing.Point(9, 347);
            this.lstAttributeConsumerServices.Name = "lstAttributeConsumerServices";
            this.lstAttributeConsumerServices.Size = new System.Drawing.Size(337, 81);
            this.lstAttributeConsumerServices.TabIndex = 48;
            this.lstAttributeConsumerServices.TabStop = false;
            this.lstAttributeConsumerServices.UseCompatibleStateImageBehavior = false;
            this.lstAttributeConsumerServices.View = System.Windows.Forms.View.Details;
            this.lstAttributeConsumerServices.SelectedIndexChanged += new System.EventHandler(this.lstAttributeConsumerServices_SelectedIndexChanged);
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Name";
            this.columnHeader4.Width = 110;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Index";
            this.columnHeader6.Width = 100;
            // 
            // columnHeader9
            // 
            this.columnHeader9.Text = "Default";
            // 
            // lstSloServices
            // 
            this.lstSloServices.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5,
            this.columnHeader8,
            this.columnHeader11});
            this.lstSloServices.FullRowSelect = true;
            this.lstSloServices.GridLines = true;
            this.lstSloServices.Location = new System.Drawing.Point(9, 457);
            this.lstSloServices.Name = "lstSloServices";
            this.lstSloServices.Size = new System.Drawing.Size(337, 81);
            this.lstSloServices.TabIndex = 52;
            this.lstSloServices.TabStop = false;
            this.lstSloServices.UseCompatibleStateImageBehavior = false;
            this.lstSloServices.View = System.Windows.Forms.View.Details;
            this.lstSloServices.SelectedIndexChanged += new System.EventHandler(this.lstSloServices_SelectedIndexChanged);
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Name";
            this.columnHeader5.Width = 80;
            // 
            // columnHeader8
            // 
            this.columnHeader8.Text = "Endpoint";
            this.columnHeader8.Width = 120;
            // 
            // columnHeader11
            // 
            this.columnHeader11.Text = "Binding";
            this.columnHeader11.Width = 120;
            // 
            // NewRelyingParty
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(384, 590);
            this.Controls.Add(this.btnViewCertificate);
            this.Controls.Add(this.btnRemoveSloService);
            this.Controls.Add(this.btnAddSloService);
            this.Controls.Add(this.lstSloServices);
            this.Controls.Add(this.btnRemoveAttributeConsumerService);
            this.Controls.Add(this.btnAddAttributeConsumerService);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.lstAttributeConsumerServices);
            this.Controls.Add(this.btnRemoveAssertionConsumerService);
            this.Controls.Add(this.btnAddAssertionConsumerService);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.lstAssertionConsumerServices);
            this.Controls.Add(this.chkSigned);
            this.Controls.Add(this.btnChooseCertificate);
            this.Controls.Add(this.txtCertificateFilePath);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.txtUrl);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.btnRemoveSignatureAlgorithm);
            this.Controls.Add(this.btnAddSignatureAlgorithm);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lstSignatureAlgorithms);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.txtName);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.horizontalLine);
            this.Controls.Add(this.btnCreate);
            this.Controls.Add(this.btnClose);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.helpProvider1.SetHelpString(this, "Relying party for the server");
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "NewRelyingParty";
            this.helpProvider1.SetShowHelp(this, true);
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "New Relying Party";
            this.toolTip1.SetToolTip(this, "Relying party for the server");
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.TextBox txtName;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.HelpProvider helpProvider1;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ListView lstSignatureAlgorithms;
        private System.Windows.Forms.ColumnHeader maxKeySize;
        private System.Windows.Forms.ColumnHeader minKeySize;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnRemoveSignatureAlgorithm;
        private System.Windows.Forms.Button btnAddSignatureAlgorithm;
        private System.Windows.Forms.TextBox txtUrl;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox txtCertificateFilePath;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button btnChooseCertificate;
        private System.Windows.Forms.CheckBox chkSigned;
        private System.Windows.Forms.ColumnHeader clmPriority;
        private System.Windows.Forms.Button btnRemoveAssertionConsumerService;
        private System.Windows.Forms.Button btnAddAssertionConsumerService;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ListView lstAssertionConsumerServices;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.Button btnRemoveAttributeConsumerService;
        private System.Windows.Forms.Button btnAddAttributeConsumerService;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ListView lstAttributeConsumerServices;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.Button btnAddSloService;
        private System.Windows.Forms.ListView lstSloServices;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader8;
        private System.Windows.Forms.Button btnRemoveSloService;
        private System.Windows.Forms.ColumnHeader columnHeader9;
        private System.Windows.Forms.ColumnHeader columnHeader10;
        private System.Windows.Forms.ColumnHeader columnHeader11;
        private System.Windows.Forms.Button btnViewCertificate;


    }
}