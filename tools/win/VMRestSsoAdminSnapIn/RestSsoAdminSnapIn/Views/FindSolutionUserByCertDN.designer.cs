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
    partial class FindSolutionUserByCertDN
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
            this.btnClose = new System.Windows.Forms.Button();
            this.btnCreate = new System.Windows.Forms.Button();
            this.btnSelectCertFile = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtCertDN = new System.Windows.Forms.TextBox();
            this.rdoUseDN = new System.Windows.Forms.RadioButton();
            this.rdoUseFile = new System.Windows.Forms.RadioButton();
            this.txtCertFile = new System.Windows.Forms.TextBox();
            this.helpProvider1 = new System.Windows.Forms.HelpProvider();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // horizontalLine
            // 
            this.horizontalLine.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.horizontalLine.Location = new System.Drawing.Point(11, 179);
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.Size = new System.Drawing.Size(364, 3);
            this.horizontalLine.TabIndex = 6;
            this.horizontalLine.TabStop = false;
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(300, 198);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 8;
            this.btnClose.Text = "Cl&ose";
            this.toolTip1.SetToolTip(this.btnClose, "Close and go back to the previous screen");
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // btnCreate
            // 
            this.btnCreate.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnCreate.Location = new System.Drawing.Point(219, 198);
            this.btnCreate.Name = "btnCreate";
            this.btnCreate.Size = new System.Drawing.Size(75, 23);
            this.btnCreate.TabIndex = 7;
            this.btnCreate.Text = "&Search";
            this.toolTip1.SetToolTip(this.btnCreate, "Click to search for solution user by certificate");
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreate_Click);
            // 
            // btnSelectCertFile
            // 
            this.btnSelectCertFile.Location = new System.Drawing.Point(317, 38);
            this.btnSelectCertFile.Name = "btnSelectCertFile";
            this.btnSelectCertFile.Size = new System.Drawing.Size(43, 23);
            this.btnSelectCertFile.TabIndex = 5;
            this.btnSelectCertFile.Text = "...";
            this.toolTip1.SetToolTip(this.btnSelectCertFile, "Choose a certificate path");
            this.btnSelectCertFile.UseVisualStyleBackColor = true;
            this.btnSelectCertFile.Click += new System.EventHandler(this.btnSelectCertFile_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.txtCertDN);
            this.groupBox1.Controls.Add(this.rdoUseDN);
            this.groupBox1.Controls.Add(this.rdoUseFile);
            this.groupBox1.Controls.Add(this.btnSelectCertFile);
            this.groupBox1.Controls.Add(this.txtCertFile);
            this.groupBox1.Location = new System.Drawing.Point(11, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(364, 150);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            // 
            // txtCertDN
            // 
            this.txtCertDN.Location = new System.Drawing.Point(31, 108);
            this.txtCertDN.Name = "txtCertDN";
            this.txtCertDN.ReadOnly = true;
            this.txtCertDN.Size = new System.Drawing.Size(327, 20);
            this.txtCertDN.TabIndex = 8;
            // 
            // rdoUseDN
            // 
            this.rdoUseDN.AutoSize = true;
            this.rdoUseDN.Location = new System.Drawing.Point(13, 85);
            this.rdoUseDN.Name = "rdoUseDN";
            this.rdoUseDN.Size = new System.Drawing.Size(118, 17);
            this.rdoUseDN.TabIndex = 7;
            this.rdoUseDN.Text = "Enter certificate DN";
            this.toolTip1.SetToolTip(this.rdoUseDN, "Enter certificate DN");
            this.rdoUseDN.UseVisualStyleBackColor = true;
            this.rdoUseDN.CheckedChanged += new System.EventHandler(this.rdoUseDN_CheckedChanged);
            // 
            // rdoUseFile
            // 
            this.rdoUseFile.AutoSize = true;
            this.rdoUseFile.Checked = true;
            this.rdoUseFile.Location = new System.Drawing.Point(13, 16);
            this.rdoUseFile.Name = "rdoUseFile";
            this.rdoUseFile.Size = new System.Drawing.Size(71, 17);
            this.rdoUseFile.TabIndex = 6;
            this.rdoUseFile.TabStop = true;
            this.rdoUseFile.Text = "Select file";
            this.toolTip1.SetToolTip(this.rdoUseFile, "Choose certificate file");
            this.rdoUseFile.UseVisualStyleBackColor = true;
            this.rdoUseFile.CheckedChanged += new System.EventHandler(this.rdoUseFile_CheckedChanged);
            // 
            // txtCertFile
            // 
            this.txtCertFile.Location = new System.Drawing.Point(31, 40);
            this.txtCertFile.Name = "txtCertFile";
            this.txtCertFile.ReadOnly = true;
            this.txtCertFile.Size = new System.Drawing.Size(279, 20);
            this.txtCertFile.TabIndex = 4;
            this.toolTip1.SetToolTip(this.txtCertFile, "Certificate path");
            // 
            // toolTip1
            // 
            this.toolTip1.AutomaticDelay = 200;
            this.toolTip1.ToolTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.toolTip1.ToolTipTitle = "Search Solution User";
            // 
            // FindSolutionUserByCertDN
            // 
            this.AcceptButton = this.btnCreate;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(383, 233);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.horizontalLine);
            this.Controls.Add(this.btnCreate);
            this.Controls.Add(this.btnClose);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(399, 271);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(399, 271);
            this.Name = "FindSolutionUserByCertDN";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Search (Certificate DN)";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TextBox txtCertFile;
        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Button btnSelectCertFile;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton rdoUseDN;
        private System.Windows.Forms.RadioButton rdoUseFile;
        private System.Windows.Forms.TextBox txtCertDN;
        private System.Windows.Forms.HelpProvider helpProvider1;
        private System.Windows.Forms.ToolTip toolTip1;



    }
}