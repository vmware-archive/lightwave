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
    partial class NewSignerIdentity
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
            this.txtKeyFile = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.horizontalLine = new System.Windows.Forms.GroupBox();
            this.btnCreate = new System.Windows.Forms.Button();
            this.btnClose = new System.Windows.Forms.Button();
            this.btnSelectKeyFile = new System.Windows.Forms.Button();
            this.btnAddCert = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.btnRemoveCert = new System.Windows.Forms.Button();
            this.txtTenantName = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.cbAlgo = new System.Windows.Forms.ComboBox();
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.txtUsername = new System.Windows.Forms.TextBox();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.listviewCertificates = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // txtKeyFile
            // 
            this.helpProvider1.SetHelpKeyword(this.txtKeyFile, "PrivateKey");
            this.helpProvider1.SetHelpString(this.txtKeyFile, "Browse and choose the private key file for the certificate chain");
            this.txtKeyFile.Location = new System.Drawing.Point(88, 91);
            this.txtKeyFile.Name = "txtKeyFile";
            this.helpProvider1.SetShowHelp(this.txtKeyFile, true);
            this.txtKeyFile.Size = new System.Drawing.Size(258, 20);
            this.txtKeyFile.TabIndex = 4;
            this.toolTip1.SetToolTip(this.txtKeyFile, "Private Key");
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 94);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(64, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Private Key:";
            // 
            // horizontalLine
            // 
            this.horizontalLine.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.horizontalLine.Location = new System.Drawing.Point(11, 416);
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.Size = new System.Drawing.Size(364, 3);
            this.horizontalLine.TabIndex = 7;
            this.horizontalLine.TabStop = false;
            // 
            // btnCreate
            // 
            this.btnCreate.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.helpProvider1.SetHelpKeyword(this.btnCreate, "SignerIdentityCreate");
            this.helpProvider1.SetHelpString(this.btnCreate, "Click to create a new signer identity for the tenant");
            this.btnCreate.Location = new System.Drawing.Point(219, 427);
            this.btnCreate.Name = "btnCreate";
            this.helpProvider1.SetShowHelp(this.btnCreate, true);
            this.btnCreate.Size = new System.Drawing.Size(75, 23);
            this.btnCreate.TabIndex = 7;
            this.btnCreate.Text = "&Create";
            this.toolTip1.SetToolTip(this.btnCreate, "Click to create a new signer identity for the tenant");
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreateSignerIdentity_Click);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(300, 427);
            this.btnClose.Name = "btnClose";
            this.helpProvider1.SetShowHelp(this.btnClose, true);
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 8;
            this.btnClose.Text = "Cl&ose";
            this.toolTip1.SetToolTip(this.btnClose, "Click to close the window. All the changes made will be lost.");
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // btnSelectKeyFile
            // 
            this.btnSelectKeyFile.Location = new System.Drawing.Point(352, 89);
            this.btnSelectKeyFile.Name = "btnSelectKeyFile";
            this.btnSelectKeyFile.Size = new System.Drawing.Size(22, 23);
            this.btnSelectKeyFile.TabIndex = 4;
            this.btnSelectKeyFile.Text = "...";
            this.toolTip1.SetToolTip(this.btnSelectKeyFile, "Browse to select private key file");
            this.btnSelectKeyFile.UseVisualStyleBackColor = true;
            this.btnSelectKeyFile.Click += new System.EventHandler(this.btnSelectKeyFile_Click);
            // 
            // btnAddCert
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddCert, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnAddCert, "Add certificate to the certificate chain");
            this.btnAddCert.Location = new System.Drawing.Point(11, 388);
            this.btnAddCert.Name = "btnAddCert";
            this.helpProvider1.SetShowHelp(this.btnAddCert, true);
            this.btnAddCert.Size = new System.Drawing.Size(132, 23);
            this.btnAddCert.TabIndex = 6;
            this.btnAddCert.Text = "Add certificate";
            this.toolTip1.SetToolTip(this.btnAddCert, "Add certificate to the certificate chain");
            this.btnAddCert.UseVisualStyleBackColor = true;
            this.btnAddCert.Click += new System.EventHandler(this.btnSelectCertFile_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(7, 155);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(158, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Certificate chain (Leaf certs first)";
            // 
            // btnRemoveCert
            // 
            this.btnRemoveCert.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveCert, "CertificateChainRemove");
            this.helpProvider1.SetHelpString(this.btnRemoveCert, "Select a certificate from the chain and click to remove");
            this.btnRemoveCert.Location = new System.Drawing.Point(241, 388);
            this.btnRemoveCert.Name = "btnRemoveCert";
            this.helpProvider1.SetShowHelp(this.btnRemoveCert, true);
            this.btnRemoveCert.Size = new System.Drawing.Size(132, 23);
            this.btnRemoveCert.TabIndex = 9;
            this.btnRemoveCert.Text = "Remove certificate";
            this.toolTip1.SetToolTip(this.btnRemoveCert, "Select a certificate from the chain and click to remove");
            this.btnRemoveCert.UseVisualStyleBackColor = true;
            this.btnRemoveCert.Click += new System.EventHandler(this.btnRemoveCert_Click);
            // 
            // txtTenantName
            // 
            this.helpProvider1.SetHelpKeyword(this.txtTenantName, "NewTenantName");
            this.helpProvider1.SetHelpString(this.txtTenantName, "Displays the name of the tenant");
            this.txtTenantName.Location = new System.Drawing.Point(88, 11);
            this.txtTenantName.Name = "txtTenantName";
            this.helpProvider1.SetShowHelp(this.txtTenantName, true);
            this.txtTenantName.Size = new System.Drawing.Size(287, 20);
            this.txtTenantName.TabIndex = 1;
            this.toolTip1.SetToolTip(this.txtTenantName, "Tenant name");
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 14);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(73, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "Tenant name:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(8, 125);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(101, 13);
            this.label4.TabIndex = 13;
            this.label4.Text = "Encrption Algorithm:";
            // 
            // cbAlgo
            // 
            this.cbAlgo.FormattingEnabled = true;
            this.helpProvider1.SetHelpKeyword(this.cbAlgo, "EncryptAlgo");
            this.helpProvider1.SetHelpString(this.cbAlgo, "Select the algorithm used by the private key for encryption");
            this.cbAlgo.Items.AddRange(new object[] {
            "RSA"});
            this.cbAlgo.Location = new System.Drawing.Point(110, 119);
            this.cbAlgo.Name = "cbAlgo";
            this.helpProvider1.SetShowHelp(this.cbAlgo, true);
            this.cbAlgo.Size = new System.Drawing.Size(264, 21);
            this.cbAlgo.TabIndex = 5;
            this.toolTip1.SetToolTip(this.cbAlgo, "Select the algorithm used by the private key for encryption");            
            // 
            // txtUsername
            // 
            this.helpProvider1.SetHelpKeyword(this.txtUsername, "NewTenantName");
            this.helpProvider1.SetHelpString(this.txtUsername, "Administrator username for the new tenant");
            this.txtUsername.Location = new System.Drawing.Point(89, 37);
            this.txtUsername.Name = "txtUsername";
            this.helpProvider1.SetShowHelp(this.txtUsername, true);
            this.txtUsername.Size = new System.Drawing.Size(286, 20);
            this.txtUsername.TabIndex = 2;
            this.toolTip1.SetToolTip(this.txtUsername, "Administrator username");
            // 
            // txtPassword
            // 
            this.helpProvider1.SetHelpKeyword(this.txtPassword, "NewTenantName");
            this.helpProvider1.SetHelpString(this.txtPassword, "Administrator password for the new tenant");
            this.txtPassword.Location = new System.Drawing.Point(89, 63);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.helpProvider1.SetShowHelp(this.txtPassword, true);
            this.txtPassword.Size = new System.Drawing.Size(286, 20);
            this.txtPassword.TabIndex = 3;
            this.toolTip1.SetToolTip(this.txtPassword, "Administrator password");
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 2000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.ReshowDelay = 100;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(10, 40);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(58, 13);
            this.label5.TabIndex = 15;
            this.label5.Text = "Username:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(10, 66);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(56, 13);
            this.label6.TabIndex = 17;
            this.label6.Text = "Password:";
            // 
            // listviewCertificates
            // 
            this.listviewCertificates.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.listviewCertificates.Location = new System.Drawing.Point(9, 174);
            this.listviewCertificates.Name = "listviewCertificates";
            this.listviewCertificates.Size = new System.Drawing.Size(366, 201);
            this.listviewCertificates.TabIndex = 18;
            this.listviewCertificates.UseCompatibleStateImageBehavior = false;
            this.listviewCertificates.View = System.Windows.Forms.View.Details;
            this.listviewCertificates.SelectedIndexChanged += new System.EventHandler(this.listviewCertificates_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Certificate Path";
            this.columnHeader1.Width = 355;
            // 
            // NewSignerIdentity
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(383, 459);
            this.Controls.Add(this.listviewCertificates);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.txtUsername);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.cbAlgo);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.txtTenantName);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.btnRemoveCert);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnAddCert);
            this.Controls.Add(this.btnSelectKeyFile);
            this.Controls.Add(this.horizontalLine);
            this.Controls.Add(this.btnCreate);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.txtKeyFile);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.helpProvider1.SetHelpString(this, "Create a new signer identity for the tenant");
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "NewSignerIdentity";
            this.helpProvider1.SetShowHelp(this, true);
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "New Signer Identity";
            this.toolTip1.SetToolTip(this, "Create a new signer identity for the tenant");
            this.Load += new System.EventHandler(this.NewSignerIdentity_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtKeyFile;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Button btnSelectKeyFile;
        private System.Windows.Forms.Button btnAddCert;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnRemoveCert;
        private System.Windows.Forms.TextBox txtTenantName;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox cbAlgo;
        private System.Windows.Forms.HelpProvider helpProvider1;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.TextBox txtUsername;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ListView listviewCertificates;
        private System.Windows.Forms.ColumnHeader columnHeader1;


    }
}