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
namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    partial class ExternalDomainGeneralPropertyIntegratedAuth
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.lblDomainName = new System.Windows.Forms.Label();
            this.pbIcon = new System.Windows.Forms.PictureBox();
            this.pnlAD = new System.Windows.Forms.Panel();
            this.rdoSpn = new System.Windows.Forms.RadioButton();
            this.rdoMachineAccount = new System.Windows.Forms.RadioButton();
            this.txtADPassword = new System.Windows.Forms.TextBox();
            this.label14 = new System.Windows.Forms.Label();
            this.txtADSpn = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.txtADUsername = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pbIcon)).BeginInit();
            this.pnlAD.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Location = new System.Drawing.Point(13, 47);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(324, 3);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            // 
            // lblDomainName
            // 
            this.lblDomainName.AutoSize = true;
            this.lblDomainName.Location = new System.Drawing.Point(71, 17);
            this.lblDomainName.Name = "lblDomainName";
            this.lblDomainName.Size = new System.Drawing.Size(0, 13);
            this.lblDomainName.TabIndex = 0;
            // 
            // pbIcon
            // 
            this.pbIcon.InitialImage = null;
            this.pbIcon.Location = new System.Drawing.Point(16, 9);
            this.pbIcon.Name = "pbIcon";
            this.pbIcon.Size = new System.Drawing.Size(32, 32);
            this.pbIcon.TabIndex = 9;
            this.pbIcon.TabStop = false;
            // 
            // pnlAD
            // 
            this.pnlAD.Controls.Add(this.rdoSpn);
            this.pnlAD.Controls.Add(this.rdoMachineAccount);
            this.pnlAD.Controls.Add(this.txtADPassword);
            this.pnlAD.Controls.Add(this.label14);
            this.pnlAD.Controls.Add(this.txtADSpn);
            this.pnlAD.Controls.Add(this.label13);
            this.pnlAD.Controls.Add(this.txtADUsername);
            this.pnlAD.Controls.Add(this.label4);
            this.pnlAD.Location = new System.Drawing.Point(2, 66);
            this.pnlAD.Name = "pnlAD";
            this.pnlAD.Size = new System.Drawing.Size(345, 263);
            this.pnlAD.TabIndex = 34;
            // 
            // rdoSpn
            // 
            this.rdoSpn.AutoSize = true;
            this.rdoSpn.Location = new System.Drawing.Point(11, 47);
            this.rdoSpn.Name = "rdoSpn";
            this.rdoSpn.Size = new System.Drawing.Size(188, 17);
            this.rdoSpn.TabIndex = 5;
            this.rdoSpn.TabStop = true;
            this.rdoSpn.Text = "Use Service Principal Name (SPN)";
            this.rdoSpn.UseVisualStyleBackColor = true;
            this.rdoSpn.CheckedChanged += new System.EventHandler(this.rdoSpn_CheckedChanged);
            // 
            // rdoMachineAccount
            // 
            this.rdoMachineAccount.AutoSize = true;
            this.rdoMachineAccount.Location = new System.Drawing.Point(12, 19);
            this.rdoMachineAccount.Name = "rdoMachineAccount";
            this.rdoMachineAccount.Size = new System.Drawing.Size(131, 17);
            this.rdoMachineAccount.TabIndex = 4;
            this.rdoMachineAccount.TabStop = true;
            this.rdoMachineAccount.Text = "Use Machine Account";
            this.rdoMachineAccount.UseVisualStyleBackColor = true;
            this.rdoMachineAccount.CheckedChanged += new System.EventHandler(this.rdoMachineAccount_CheckedChanged);
            // 
            // txtADPassword
            // 
            this.txtADPassword.Location = new System.Drawing.Point(133, 144);
            this.txtADPassword.Name = "txtADPassword";
            this.txtADPassword.PasswordChar = '*';
            this.txtADPassword.Size = new System.Drawing.Size(202, 20);
            this.txtADPassword.TabIndex = 8;
            this.txtADPassword.TextChanged += new System.EventHandler(this.txtADPassword_TextChanged);
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(7, 146);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(56, 13);
            this.label14.TabIndex = 38;
            this.label14.Text = "Password:";
            // 
            // txtADSpn
            // 
            this.txtADSpn.Location = new System.Drawing.Point(134, 80);
            this.txtADSpn.Name = "txtADSpn";
            this.txtADSpn.Size = new System.Drawing.Size(201, 20);
            this.txtADSpn.TabIndex = 6;
            this.txtADSpn.TextChanged += new System.EventHandler(this.txtADSpn_TextChanged);
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(7, 82);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(120, 13);
            this.label13.TabIndex = 36;
            this.label13.Text = "Service Principal Name:";
            // 
            // txtADUsername
            // 
            this.txtADUsername.Location = new System.Drawing.Point(133, 113);
            this.txtADUsername.Name = "txtADUsername";
            this.txtADUsername.Size = new System.Drawing.Size(202, 20);
            this.txtADUsername.TabIndex = 7;
            this.txtADUsername.TextChanged += new System.EventHandler(this.txtADUsername_TextChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(8, 115);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(106, 13);
            this.label4.TabIndex = 34;
            this.label4.Text = "User Principal Name:";
            // 
            // ExternalDomainGeneralPropertyIntegratedAuth
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.pnlAD);
            this.Controls.Add(this.pbIcon);
            this.Controls.Add(this.lblDomainName);
            this.Controls.Add(this.groupBox1);
            this.Name = "ExternalDomainGeneralPropertyIntegratedAuth";
            this.Size = new System.Drawing.Size(350, 359);
            ((System.ComponentModel.ISupportInitialize)(this.pbIcon)).EndInit();
            this.pnlAD.ResumeLayout(false);
            this.pnlAD.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label lblDomainName;
        private System.Windows.Forms.PictureBox pbIcon;
        private System.Windows.Forms.Panel pnlAD;
        private System.Windows.Forms.RadioButton rdoSpn;
        private System.Windows.Forms.RadioButton rdoMachineAccount;
        private System.Windows.Forms.TextBox txtADPassword;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox txtADSpn;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.TextBox txtADUsername;
        private System.Windows.Forms.Label label4;

    }
}
