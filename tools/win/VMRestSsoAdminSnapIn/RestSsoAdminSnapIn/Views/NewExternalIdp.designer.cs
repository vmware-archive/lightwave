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
    partial class NewExternalIdp
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
            this.btnAddCert = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.btnRemoveCert = new System.Windows.Forms.Button();
            this.txtEntityId = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.btnRemoveSloService = new System.Windows.Forms.Button();
            this.btnSloService = new System.Windows.Forms.Button();
            this.lstSloServices = new System.Windows.Forms.ListBox();
            this.btnRemoveSsoService = new System.Windows.Forms.Button();
            this.btnAddSsoService = new System.Windows.Forms.Button();
            this.lstSsoServices = new System.Windows.Forms.ListBox();
            this.btnRemoveNameIdFormat = new System.Windows.Forms.Button();
            this.btnAddNameIdFormat = new System.Windows.Forms.Button();
            this.lstNameIdFormats = new System.Windows.Forms.ListBox();
            this.btnRemoveSubjectFormat = new System.Windows.Forms.Button();
            this.btnAddSubjectFormat = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.txtAlias = new System.Windows.Forms.TextBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.label7 = new System.Windows.Forms.Label();
            this.lblSsoServices = new System.Windows.Forms.Label();
            this.lblNameIdFormats = new System.Windows.Forms.Label();
            this.chkJit = new System.Windows.Forms.CheckBox();
            this.lstSubjectFormat = new System.Windows.Forms.ListView();
            this.clmSubjectFormatItem = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.clmSubjectFormatValue = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label1 = new System.Windows.Forms.Label();
            this.lstCertificateChain = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label4 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // horizontalLine
            // 
            this.horizontalLine.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.horizontalLine.Location = new System.Drawing.Point(11, 569);
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.Size = new System.Drawing.Size(365, 3);
            this.horizontalLine.TabIndex = 7;
            this.horizontalLine.TabStop = false;
            // 
            // btnCreate
            // 
            this.btnCreate.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.helpProvider1.SetHelpKeyword(this.btnCreate, "SignerIdentityCreate");
            this.helpProvider1.SetHelpString(this.btnCreate, "Click to create a new signer identity for the tenant");
            this.btnCreate.Location = new System.Drawing.Point(220, 577);
            this.btnCreate.Name = "btnCreate";
            this.helpProvider1.SetShowHelp(this.btnCreate, true);
            this.btnCreate.Size = new System.Drawing.Size(75, 23);
            this.btnCreate.TabIndex = 13;
            this.btnCreate.Text = "&Create";
            this.toolTip1.SetToolTip(this.btnCreate, "Click to create a new signer identity for the tenant");
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreateSignerIdentity_Click);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(301, 577);
            this.btnClose.Name = "btnClose";
            this.helpProvider1.SetShowHelp(this.btnClose, true);
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 14;
            this.btnClose.Text = "Cl&ose";
            this.toolTip1.SetToolTip(this.btnClose, "Click to close the window. All the changes made will be lost.");
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // btnAddCert
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddCert, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnAddCert, "Add certificate to the certificate chain");
            this.btnAddCert.Location = new System.Drawing.Point(354, 468);
            this.btnAddCert.Name = "btnAddCert";
            this.helpProvider1.SetShowHelp(this.btnAddCert, true);
            this.btnAddCert.Size = new System.Drawing.Size(24, 23);
            this.btnAddCert.TabIndex = 11;
            this.btnAddCert.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddCert, "Add certificate to the certificate chain");
            this.btnAddCert.UseVisualStyleBackColor = true;
            this.btnAddCert.Click += new System.EventHandler(this.btnSelectCertFile_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 449);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(196, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Signing Certificate chain (Leaf certs first)";
            // 
            // btnRemoveCert
            // 
            this.btnRemoveCert.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveCert, "CertificateChainRemove");
            this.helpProvider1.SetHelpString(this.btnRemoveCert, "Select a certificate from the chain and click to remove");
            this.btnRemoveCert.Location = new System.Drawing.Point(354, 540);
            this.btnRemoveCert.Name = "btnRemoveCert";
            this.helpProvider1.SetShowHelp(this.btnRemoveCert, true);
            this.btnRemoveCert.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveCert.TabIndex = 12;
            this.btnRemoveCert.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveCert, "Select a certificate from the chain and click to remove");
            this.btnRemoveCert.UseVisualStyleBackColor = true;
            this.btnRemoveCert.Click += new System.EventHandler(this.btnRemoveCert_Click);
            // 
            // txtEntityId
            // 
            this.helpProvider1.SetHelpKeyword(this.txtEntityId, "NewTenantName");
            this.helpProvider1.SetHelpString(this.txtEntityId, "Displays the name of the tenant");
            this.txtEntityId.Location = new System.Drawing.Point(73, 11);
            this.txtEntityId.Name = "txtEntityId";
            this.helpProvider1.SetShowHelp(this.txtEntityId, true);
            this.txtEntityId.Size = new System.Drawing.Size(302, 20);
            this.txtEntityId.TabIndex = 1;
            this.toolTip1.SetToolTip(this.txtEntityId, "Entity Id");
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 14);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(58, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "Unique ID:";
            // 
            // btnRemoveSloService
            // 
            this.btnRemoveSloService.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveSloService, "CertificateChainRemove");
            this.helpProvider1.SetHelpString(this.btnRemoveSloService, "Select a certificate from the chain and click to remove");
            this.btnRemoveSloService.Location = new System.Drawing.Point(352, 421);
            this.btnRemoveSloService.Name = "btnRemoveSloService";
            this.helpProvider1.SetShowHelp(this.btnRemoveSloService, true);
            this.btnRemoveSloService.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveSloService.TabIndex = 10;
            this.btnRemoveSloService.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveSloService, "Select a certificate from the chain and click to remove");
            this.btnRemoveSloService.UseVisualStyleBackColor = true;
            this.btnRemoveSloService.Click += new System.EventHandler(this.btnRemoveSloService_Click);
            // 
            // btnSloService
            // 
            this.helpProvider1.SetHelpKeyword(this.btnSloService, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnSloService, "Add certificate to the certificate chain");
            this.btnSloService.Location = new System.Drawing.Point(352, 375);
            this.btnSloService.Name = "btnSloService";
            this.helpProvider1.SetShowHelp(this.btnSloService, true);
            this.btnSloService.Size = new System.Drawing.Size(24, 23);
            this.btnSloService.TabIndex = 9;
            this.btnSloService.Text = "+";
            this.toolTip1.SetToolTip(this.btnSloService, "Add certificate to the certificate chain");
            this.btnSloService.UseVisualStyleBackColor = true;
            this.btnSloService.Click += new System.EventHandler(this.btnSloService_Click);
            // 
            // lstSloServices
            // 
            this.lstSloServices.FormattingEnabled = true;
            this.helpProvider1.SetHelpString(this.lstSloServices, "Displays the logout endpoints provided by the external identity provider");
            this.lstSloServices.Location = new System.Drawing.Point(9, 375);
            this.lstSloServices.Name = "lstSloServices";
            this.helpProvider1.SetShowHelp(this.lstSloServices, true);
            this.lstSloServices.Size = new System.Drawing.Size(337, 69);
            this.lstSloServices.TabIndex = 25;
            this.lstSloServices.TabStop = false;
            this.lstSloServices.SelectedIndexChanged += new System.EventHandler(this.lstSloServices_SelectedIndexChanged);
            // 
            // btnRemoveSsoService
            // 
            this.btnRemoveSsoService.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveSsoService, "CertificateChainRemove");
            this.helpProvider1.SetHelpString(this.btnRemoveSsoService, "Select a certificate from the chain and click to remove");
            this.btnRemoveSsoService.Location = new System.Drawing.Point(352, 333);
            this.btnRemoveSsoService.Name = "btnRemoveSsoService";
            this.helpProvider1.SetShowHelp(this.btnRemoveSsoService, true);
            this.btnRemoveSsoService.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveSsoService.TabIndex = 8;
            this.btnRemoveSsoService.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveSsoService, "Select a certificate from the chain and click to remove");
            this.btnRemoveSsoService.UseVisualStyleBackColor = true;
            this.btnRemoveSsoService.Click += new System.EventHandler(this.btnRemoveSsoService_Click);
            // 
            // btnAddSsoService
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddSsoService, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnAddSsoService, "Add certificate to the certificate chain");
            this.btnAddSsoService.Location = new System.Drawing.Point(352, 287);
            this.btnAddSsoService.Name = "btnAddSsoService";
            this.helpProvider1.SetShowHelp(this.btnAddSsoService, true);
            this.btnAddSsoService.Size = new System.Drawing.Size(24, 23);
            this.btnAddSsoService.TabIndex = 7;
            this.btnAddSsoService.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddSsoService, "Add certificate to the certificate chain");
            this.btnAddSsoService.UseVisualStyleBackColor = true;
            this.btnAddSsoService.Click += new System.EventHandler(this.btnAddSsoService_Click);
            // 
            // lstSsoServices
            // 
            this.lstSsoServices.FormattingEnabled = true;
            this.helpProvider1.SetHelpString(this.lstSsoServices, "Displays the login endpoints provided by the external identity provider");
            this.lstSsoServices.Location = new System.Drawing.Point(9, 287);
            this.lstSsoServices.Name = "lstSsoServices";
            this.helpProvider1.SetShowHelp(this.lstSsoServices, true);
            this.lstSsoServices.Size = new System.Drawing.Size(337, 69);
            this.lstSsoServices.TabIndex = 29;
            this.lstSsoServices.TabStop = false;
            this.lstSsoServices.SelectedIndexChanged += new System.EventHandler(this.lstSsoServices_SelectedIndexChanged);
            // 
            // btnRemoveNameIdFormat
            // 
            this.btnRemoveNameIdFormat.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveNameIdFormat, "CertificateChainRemove");
            this.helpProvider1.SetHelpString(this.btnRemoveNameIdFormat, "Select a certificate from the chain and click to remove");
            this.btnRemoveNameIdFormat.Location = new System.Drawing.Point(352, 243);
            this.btnRemoveNameIdFormat.Name = "btnRemoveNameIdFormat";
            this.helpProvider1.SetShowHelp(this.btnRemoveNameIdFormat, true);
            this.btnRemoveNameIdFormat.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveNameIdFormat.TabIndex = 6;
            this.btnRemoveNameIdFormat.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveNameIdFormat, "Select a certificate from the chain and click to remove");
            this.btnRemoveNameIdFormat.UseVisualStyleBackColor = true;
            this.btnRemoveNameIdFormat.Click += new System.EventHandler(this.btnRemoveNameIdFormat_Click);
            // 
            // btnAddNameIdFormat
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddNameIdFormat, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnAddNameIdFormat, "Add certificate to the certificate chain");
            this.btnAddNameIdFormat.Location = new System.Drawing.Point(352, 197);
            this.btnAddNameIdFormat.Name = "btnAddNameIdFormat";
            this.helpProvider1.SetShowHelp(this.btnAddNameIdFormat, true);
            this.btnAddNameIdFormat.Size = new System.Drawing.Size(24, 23);
            this.btnAddNameIdFormat.TabIndex = 5;
            this.btnAddNameIdFormat.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddNameIdFormat, "Add certificate to the certificate chain");
            this.btnAddNameIdFormat.UseVisualStyleBackColor = true;
            this.btnAddNameIdFormat.Click += new System.EventHandler(this.btnAddNameIdFormat_Click);
            // 
            // lstNameIdFormats
            // 
            this.lstNameIdFormats.FormattingEnabled = true;
            this.helpProvider1.SetHelpString(this.lstNameIdFormats, "Displays the name id formats for the external identity provider");
            this.lstNameIdFormats.Location = new System.Drawing.Point(9, 197);
            this.lstNameIdFormats.Name = "lstNameIdFormats";
            this.helpProvider1.SetShowHelp(this.lstNameIdFormats, true);
            this.lstNameIdFormats.Size = new System.Drawing.Size(337, 69);
            this.lstNameIdFormats.TabIndex = 33;
            this.lstNameIdFormats.TabStop = false;
            this.lstNameIdFormats.SelectedIndexChanged += new System.EventHandler(this.lstNameIdFormats_SelectedIndexChanged);
            // 
            // btnRemoveSubjectFormat
            // 
            this.btnRemoveSubjectFormat.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveSubjectFormat, "CertificateChainRemove");
            this.helpProvider1.SetHelpString(this.btnRemoveSubjectFormat, "Select a certificate from the chain and click to remove");
            this.btnRemoveSubjectFormat.Location = new System.Drawing.Point(351, 152);
            this.btnRemoveSubjectFormat.Name = "btnRemoveSubjectFormat";
            this.helpProvider1.SetShowHelp(this.btnRemoveSubjectFormat, true);
            this.btnRemoveSubjectFormat.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveSubjectFormat.TabIndex = 4;
            this.btnRemoveSubjectFormat.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveSubjectFormat, "Select a certificate from the chain and click to remove");
            this.btnRemoveSubjectFormat.UseVisualStyleBackColor = true;
            this.btnRemoveSubjectFormat.Click += new System.EventHandler(this.btnRemoveSubjectFormat_Click);
            // 
            // btnAddSubjectFormat
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddSubjectFormat, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnAddSubjectFormat, "Add certificate to the certificate chain");
            this.btnAddSubjectFormat.Location = new System.Drawing.Point(351, 94);
            this.btnAddSubjectFormat.Name = "btnAddSubjectFormat";
            this.helpProvider1.SetShowHelp(this.btnAddSubjectFormat, true);
            this.btnAddSubjectFormat.Size = new System.Drawing.Size(24, 23);
            this.btnAddSubjectFormat.TabIndex = 3;
            this.btnAddSubjectFormat.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddSubjectFormat, "Add certificate to the certificate chain");
            this.btnAddSubjectFormat.UseVisualStyleBackColor = true;
            this.btnAddSubjectFormat.Click += new System.EventHandler(this.btnAddSubjectFormat_Click);
            // 
            // button1
            // 
            this.helpProvider1.SetHelpKeyword(this.button1, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.button1, "View certificate");
            this.button1.Location = new System.Drawing.Point(354, 506);
            this.button1.Name = "button1";
            this.helpProvider1.SetShowHelp(this.button1, true);
            this.button1.Size = new System.Drawing.Size(24, 23);
            this.button1.TabIndex = 38;
            this.button1.Text = "v";
            this.toolTip1.SetToolTip(this.button1, "View certificate");
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // txtAlias
            // 
            this.helpProvider1.SetHelpKeyword(this.txtAlias, "NewTenantName");
            this.helpProvider1.SetHelpString(this.txtAlias, "Displays the alias of the external identity provider");
            this.txtAlias.Location = new System.Drawing.Point(73, 37);
            this.txtAlias.Name = "txtAlias";
            this.helpProvider1.SetShowHelp(this.txtAlias, true);
            this.txtAlias.Size = new System.Drawing.Size(302, 20);
            this.txtAlias.TabIndex = 2;
            this.toolTip1.SetToolTip(this.txtAlias, "Alias");
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
            this.label7.Location = new System.Drawing.Point(10, 359);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(66, 13);
            this.label7.TabIndex = 24;
            this.label7.Text = "Slo Services";
            // 
            // lblSsoServices
            // 
            this.lblSsoServices.AutoSize = true;
            this.lblSsoServices.Location = new System.Drawing.Point(10, 271);
            this.lblSsoServices.Name = "lblSsoServices";
            this.lblSsoServices.Size = new System.Drawing.Size(69, 13);
            this.lblSsoServices.TabIndex = 28;
            this.lblSsoServices.Text = "Sso Services";
            // 
            // lblNameIdFormats
            // 
            this.lblNameIdFormats.AutoSize = true;
            this.lblNameIdFormats.Location = new System.Drawing.Point(10, 181);
            this.lblNameIdFormats.Name = "lblNameIdFormats";
            this.lblNameIdFormats.Size = new System.Drawing.Size(87, 13);
            this.lblNameIdFormats.TabIndex = 32;
            this.lblNameIdFormats.Text = "Name Id Formats";
            // 
            // chkJit
            // 
            this.chkJit.AutoSize = true;
            this.chkJit.Location = new System.Drawing.Point(331, 63);
            this.chkJit.Name = "chkJit";
            this.chkJit.Size = new System.Drawing.Size(41, 17);
            this.chkJit.TabIndex = 3;
            this.chkJit.Text = "JIT";
            this.chkJit.UseVisualStyleBackColor = true;
            // 
            // lstSubjectFormat
            // 
            this.lstSubjectFormat.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.clmSubjectFormatItem,
            this.clmSubjectFormatValue});
            this.lstSubjectFormat.FullRowSelect = true;
            this.lstSubjectFormat.GridLines = true;
            this.lstSubjectFormat.Location = new System.Drawing.Point(9, 94);
            this.lstSubjectFormat.Name = "lstSubjectFormat";
            this.lstSubjectFormat.Size = new System.Drawing.Size(337, 81);
            this.lstSubjectFormat.TabIndex = 34;
            this.lstSubjectFormat.TabStop = false;
            this.lstSubjectFormat.UseCompatibleStateImageBehavior = false;
            this.lstSubjectFormat.View = System.Windows.Forms.View.Details;
            this.lstSubjectFormat.SelectedIndexChanged += new System.EventHandler(this.lstSubjectFormat_SelectedIndexChanged);
            // 
            // clmSubjectFormatItem
            // 
            this.clmSubjectFormatItem.Text = "Name";
            this.clmSubjectFormatItem.Width = 106;
            // 
            // clmSubjectFormatValue
            // 
            this.clmSubjectFormatValue.Text = "Value";
            this.clmSubjectFormatValue.Width = 226;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 78);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(83, 13);
            this.label1.TabIndex = 35;
            this.label1.Text = "Subject Formats";
            // 
            // lstCertificateChain
            // 
            this.lstCertificateChain.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.lstCertificateChain.FullRowSelect = true;
            this.lstCertificateChain.GridLines = true;
            this.lstCertificateChain.HideSelection = false;
            this.lstCertificateChain.Location = new System.Drawing.Point(9, 468);
            this.lstCertificateChain.MultiSelect = false;
            this.lstCertificateChain.Name = "lstCertificateChain";
            this.lstCertificateChain.Size = new System.Drawing.Size(337, 95);
            this.lstCertificateChain.TabIndex = 37;
            this.lstCertificateChain.TabStop = false;
            this.lstCertificateChain.UseCompatibleStateImageBehavior = false;
            this.lstCertificateChain.View = System.Windows.Forms.View.Details;
            this.lstCertificateChain.SelectedIndexChanged += new System.EventHandler(this.lstCertificateChain_SelectedIndexChanged);
            this.lstCertificateChain.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lstCertificateChain_MouseDoubleClick);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Certificate";
            this.columnHeader1.Width = 320;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(35, 40);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(32, 13);
            this.label4.TabIndex = 40;
            this.label4.Text = "Alias:";
            // 
            // NewExternalIdp
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(384, 609);
            this.Controls.Add(this.txtAlias);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.lstCertificateChain);
            this.Controls.Add(this.btnRemoveSubjectFormat);
            this.Controls.Add(this.btnAddSubjectFormat);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lstSubjectFormat);
            this.Controls.Add(this.chkJit);
            this.Controls.Add(this.btnRemoveNameIdFormat);
            this.Controls.Add(this.lblNameIdFormats);
            this.Controls.Add(this.btnAddNameIdFormat);
            this.Controls.Add(this.lstNameIdFormats);
            this.Controls.Add(this.btnRemoveSsoService);
            this.Controls.Add(this.lblSsoServices);
            this.Controls.Add(this.btnAddSsoService);
            this.Controls.Add(this.lstSsoServices);
            this.Controls.Add(this.btnRemoveSloService);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.btnSloService);
            this.Controls.Add(this.lstSloServices);
            this.Controls.Add(this.txtEntityId);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.btnRemoveCert);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnAddCert);
            this.Controls.Add(this.horizontalLine);
            this.Controls.Add(this.btnCreate);
            this.Controls.Add(this.btnClose);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.helpProvider1.SetHelpString(this, "Create a new external identity provider for the server");
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "NewExternalIdp";
            this.helpProvider1.SetShowHelp(this, true);
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "New External Identity Provider";
            this.toolTip1.SetToolTip(this, "Create a new external identity provider for the server");
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Button btnAddCert;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnRemoveCert;
        private System.Windows.Forms.TextBox txtEntityId;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.HelpProvider helpProvider1;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.Button btnRemoveSloService;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button btnSloService;
        private System.Windows.Forms.ListBox lstSloServices;
        private System.Windows.Forms.Button btnRemoveSsoService;
        private System.Windows.Forms.Label lblSsoServices;
        private System.Windows.Forms.Button btnAddSsoService;
        private System.Windows.Forms.ListBox lstSsoServices;
        private System.Windows.Forms.Button btnRemoveNameIdFormat;
        private System.Windows.Forms.Label lblNameIdFormats;
        private System.Windows.Forms.Button btnAddNameIdFormat;
        private System.Windows.Forms.CheckBox chkJit;
        private System.Windows.Forms.ListBox lstNameIdFormats;
        private System.Windows.Forms.ListView lstSubjectFormat;
        private System.Windows.Forms.ColumnHeader clmSubjectFormatItem;
        private System.Windows.Forms.ColumnHeader clmSubjectFormatValue;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnRemoveSubjectFormat;
        private System.Windows.Forms.Button btnAddSubjectFormat;
        private System.Windows.Forms.ListView lstCertificateChain;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox txtAlias;
        private System.Windows.Forms.Label label4;


    }
}