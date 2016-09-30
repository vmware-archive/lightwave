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
    partial class NewOidcClient
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
            this.txtClientId = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.btnRemoveRedirecturi = new System.Windows.Forms.Button();
            this.btnAddRedirectUri = new System.Windows.Forms.Button();
            this.txtSubjectDN = new System.Windows.Forms.TextBox();
            this.txtLogoutUri = new System.Windows.Forms.TextBox();
            this.btnRemovePostlogoutUri = new System.Windows.Forms.Button();
            this.btnAddPostLogoutRedirectUri = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.lstRedirectUris = new System.Windows.Forms.ListView();
            this.clmSubjectFormatItem = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.lstPostLogoutRedirectUris = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.cbTokenAuth = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // horizontalLine
            // 
            this.horizontalLine.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.horizontalLine.Location = new System.Drawing.Point(11, 350);
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.Size = new System.Drawing.Size(365, 3);
            this.horizontalLine.TabIndex = 7;
            this.horizontalLine.TabStop = false;
            // 
            // btnCreate
            // 
            this.btnCreate.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.helpProvider1.SetHelpKeyword(this.btnCreate, "SignerIdentityCreate");
            this.helpProvider1.SetHelpString(this.btnCreate, "Click to create a new OIDC CLient for the tenant");
            this.btnCreate.Location = new System.Drawing.Point(220, 358);
            this.btnCreate.Name = "btnCreate";
            this.helpProvider1.SetShowHelp(this.btnCreate, true);
            this.btnCreate.Size = new System.Drawing.Size(75, 23);
            this.btnCreate.TabIndex = 9;
            this.btnCreate.Text = "&Create";
            this.toolTip1.SetToolTip(this.btnCreate, "Click to create a new OIDC CLient for the tenant");
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreateSignerIdentity_Click);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(301, 358);
            this.btnClose.Name = "btnClose";
            this.helpProvider1.SetShowHelp(this.btnClose, true);
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 10;
            this.btnClose.Text = "Cl&ose";
            this.toolTip1.SetToolTip(this.btnClose, "Click to close the window. All the changes made will be lost.");
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // txtClientId
            // 
            this.helpProvider1.SetHelpKeyword(this.txtClientId, "NewTenantName");
            this.helpProvider1.SetHelpString(this.txtClientId, "Displays the client id");
            this.txtClientId.Location = new System.Drawing.Point(128, 11);
            this.txtClientId.Name = "txtClientId";
            this.txtClientId.ReadOnly = true;
            this.helpProvider1.SetShowHelp(this.txtClientId, true);
            this.txtClientId.Size = new System.Drawing.Size(244, 20);
            this.txtClientId.TabIndex = 100;
            this.txtClientId.TabStop = false;
            this.toolTip1.SetToolTip(this.txtClientId, "Displays the client id");
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(67, 14);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(48, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "Client Id:";
            // 
            // btnRemoveRedirecturi
            // 
            this.btnRemoveRedirecturi.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemoveRedirecturi, "CertificateChainRemove");
            this.helpProvider1.SetHelpString(this.btnRemoveRedirecturi, "Select and click to remove");
            this.btnRemoveRedirecturi.Location = new System.Drawing.Point(351, 205);
            this.btnRemoveRedirecturi.Name = "btnRemoveRedirecturi";
            this.helpProvider1.SetShowHelp(this.btnRemoveRedirecturi, true);
            this.btnRemoveRedirecturi.Size = new System.Drawing.Size(24, 23);
            this.btnRemoveRedirecturi.TabIndex = 5;
            this.btnRemoveRedirecturi.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemoveRedirecturi, "Select and click to remove");
            this.btnRemoveRedirecturi.UseVisualStyleBackColor = true;
            this.btnRemoveRedirecturi.Click += new System.EventHandler(this.btnRemoveRedirectUri_Click);
            // 
            // btnAddRedirectUri
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddRedirectUri, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnAddRedirectUri, "Add redirect uri");
            this.btnAddRedirectUri.Location = new System.Drawing.Point(351, 147);
            this.btnAddRedirectUri.Name = "btnAddRedirectUri";
            this.helpProvider1.SetShowHelp(this.btnAddRedirectUri, true);
            this.btnAddRedirectUri.Size = new System.Drawing.Size(24, 23);
            this.btnAddRedirectUri.TabIndex = 5;
            this.btnAddRedirectUri.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddRedirectUri, "Add redirect uri");
            this.btnAddRedirectUri.UseVisualStyleBackColor = true;
            this.btnAddRedirectUri.Click += new System.EventHandler(this.btnAddRedirectUri_Click);
            // 
            // txtSubjectDN
            // 
            this.helpProvider1.SetHelpKeyword(this.txtSubjectDN, "NewOIDCClient");
            this.helpProvider1.SetHelpString(this.txtSubjectDN, "Displays the Certificate Subject DN");
            this.txtSubjectDN.Location = new System.Drawing.Point(128, 37);
            this.txtSubjectDN.Name = "txtSubjectDN";
            this.helpProvider1.SetShowHelp(this.txtSubjectDN, true);
            this.txtSubjectDN.Size = new System.Drawing.Size(245, 20);
            this.txtSubjectDN.TabIndex = 1;
            this.toolTip1.SetToolTip(this.txtSubjectDN, "Displays the Certificate Subject DN");
            // 
            // txtLogoutUri
            // 
            this.helpProvider1.SetHelpKeyword(this.txtLogoutUri, "NewOidcClient");
            this.helpProvider1.SetHelpString(this.txtLogoutUri, "Displays the logout Uri");
            this.txtLogoutUri.Location = new System.Drawing.Point(127, 90);
            this.txtLogoutUri.Name = "txtLogoutUri";
            this.helpProvider1.SetShowHelp(this.txtLogoutUri, true);
            this.txtLogoutUri.Size = new System.Drawing.Size(245, 20);
            this.txtLogoutUri.TabIndex = 3;
            this.toolTip1.SetToolTip(this.txtLogoutUri, "Displays the logout Uri");
            // 
            // btnRemovePostlogoutUri
            // 
            this.btnRemovePostlogoutUri.Enabled = false;
            this.helpProvider1.SetHelpKeyword(this.btnRemovePostlogoutUri, "");
            this.helpProvider1.SetHelpString(this.btnRemovePostlogoutUri, "Select and click to remove");
            this.btnRemovePostlogoutUri.Location = new System.Drawing.Point(351, 317);
            this.btnRemovePostlogoutUri.Name = "btnRemovePostlogoutUri";
            this.helpProvider1.SetShowHelp(this.btnRemovePostlogoutUri, true);
            this.btnRemovePostlogoutUri.Size = new System.Drawing.Size(24, 23);
            this.btnRemovePostlogoutUri.TabIndex = 8;
            this.btnRemovePostlogoutUri.Text = "-";
            this.toolTip1.SetToolTip(this.btnRemovePostlogoutUri, "Select and click to remove");
            this.btnRemovePostlogoutUri.UseVisualStyleBackColor = true;
            this.btnRemovePostlogoutUri.Click += new System.EventHandler(this.btnRemovePostlogoutUri_Click);
            // 
            // btnAddPostLogoutRedirectUri
            // 
            this.helpProvider1.SetHelpKeyword(this.btnAddPostLogoutRedirectUri, "CertificateChainAdd");
            this.helpProvider1.SetHelpString(this.btnAddPostLogoutRedirectUri, "Add post logout redirect uri");
            this.btnAddPostLogoutRedirectUri.Location = new System.Drawing.Point(351, 259);
            this.btnAddPostLogoutRedirectUri.Name = "btnAddPostLogoutRedirectUri";
            this.helpProvider1.SetShowHelp(this.btnAddPostLogoutRedirectUri, true);
            this.btnAddPostLogoutRedirectUri.Size = new System.Drawing.Size(24, 23);
            this.btnAddPostLogoutRedirectUri.TabIndex = 7;
            this.btnAddPostLogoutRedirectUri.Text = "+";
            this.toolTip1.SetToolTip(this.btnAddPostLogoutRedirectUri, "Add post logout redirect uri");
            this.btnAddPostLogoutRedirectUri.UseVisualStyleBackColor = true;
            this.btnAddPostLogoutRedirectUri.Click += new System.EventHandler(this.btnAddPostLogoutRedirectUri_Click);
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 2000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.ReshowDelay = 100;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            // 
            // lstRedirectUris
            // 
            this.lstRedirectUris.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.clmSubjectFormatItem});
            this.lstRedirectUris.FullRowSelect = true;
            this.lstRedirectUris.GridLines = true;
            this.lstRedirectUris.Location = new System.Drawing.Point(9, 147);
            this.lstRedirectUris.Name = "lstRedirectUris";
            this.lstRedirectUris.Size = new System.Drawing.Size(337, 81);
            this.lstRedirectUris.TabIndex = 4;
            this.lstRedirectUris.UseCompatibleStateImageBehavior = false;
            this.lstRedirectUris.View = System.Windows.Forms.View.Details;
            this.lstRedirectUris.SelectedIndexChanged += new System.EventHandler(this.lstRedirectUris_SelectedIndexChanged);
            // 
            // clmSubjectFormatItem
            // 
            this.clmSubjectFormatItem.Text = "URI";
            this.clmSubjectFormatItem.Width = 330;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 131);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(77, 13);
            this.label1.TabIndex = 35;
            this.label1.Text = "Redirect Uri(s):";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(10, 40);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(115, 13);
            this.label2.TabIndex = 37;
            this.label2.Text = "Certificate Subject DN:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(9, 66);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(105, 13);
            this.label4.TabIndex = 39;
            this.label4.Text = "Token Auth Method:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(61, 93);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(59, 13);
            this.label5.TabIndex = 45;
            this.label5.Text = "Logout Uri:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(9, 243);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(137, 13);
            this.label6.TabIndex = 43;
            this.label6.Text = "Post Logout Redirect Uri(s):";
            // 
            // lstPostLogoutRedirectUris
            // 
            this.lstPostLogoutRedirectUris.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.lstPostLogoutRedirectUris.FullRowSelect = true;
            this.lstPostLogoutRedirectUris.GridLines = true;
            this.lstPostLogoutRedirectUris.Location = new System.Drawing.Point(9, 259);
            this.lstPostLogoutRedirectUris.Name = "lstPostLogoutRedirectUris";
            this.lstPostLogoutRedirectUris.Size = new System.Drawing.Size(337, 81);
            this.lstPostLogoutRedirectUris.TabIndex = 6;
            this.lstPostLogoutRedirectUris.UseCompatibleStateImageBehavior = false;
            this.lstPostLogoutRedirectUris.View = System.Windows.Forms.View.Details;
            this.lstPostLogoutRedirectUris.SelectedIndexChanged += new System.EventHandler(this.lstPostLogoutRedirectUris_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "URI";
            this.columnHeader1.Width = 330;
            // 
            // cbTokenAuth
            // 
            this.cbTokenAuth.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbTokenAuth.FormattingEnabled = true;
            this.cbTokenAuth.Items.AddRange(new object[] {
            "none",
            "private_key_jwt"});
            this.cbTokenAuth.Location = new System.Drawing.Point(128, 63);
            this.cbTokenAuth.Name = "cbTokenAuth";
            this.cbTokenAuth.Size = new System.Drawing.Size(244, 21);
            this.cbTokenAuth.TabIndex = 101;
            // 
            // NewOidcClient
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(384, 390);
            this.Controls.Add(this.cbTokenAuth);
            this.Controls.Add(this.txtLogoutUri);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.btnRemovePostlogoutUri);
            this.Controls.Add(this.btnAddPostLogoutRedirectUri);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.lstPostLogoutRedirectUris);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.txtSubjectDN);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnRemoveRedirecturi);
            this.Controls.Add(this.btnAddRedirectUri);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lstRedirectUris);
            this.Controls.Add(this.txtClientId);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.horizontalLine);
            this.Controls.Add(this.btnCreate);
            this.Controls.Add(this.btnClose);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.helpProvider1.SetHelpString(this, "OIDC Client for the server");
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "NewOidcClient";
            this.helpProvider1.SetShowHelp(this, true);
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "New OIDC Client";
            this.toolTip1.SetToolTip(this, "OIDC Client for the server");
            this.Load += new System.EventHandler(this.NewOidcClient_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.TextBox txtClientId;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.HelpProvider helpProvider1;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.ListView lstRedirectUris;
        private System.Windows.Forms.ColumnHeader clmSubjectFormatItem;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnRemoveRedirecturi;
        private System.Windows.Forms.Button btnAddRedirectUri;
        private System.Windows.Forms.TextBox txtSubjectDN;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox txtLogoutUri;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button btnRemovePostlogoutUri;
        private System.Windows.Forms.Button btnAddPostLogoutRedirectUri;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ListView lstPostLogoutRedirectUris;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ComboBox cbTokenAuth;


    }
}