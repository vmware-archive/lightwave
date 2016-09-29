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
namespace VMCertStoreSnapIn.UI
{
    partial class FormAddPrivateKey
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
            this.label1 = new System.Windows.Forms.Label();
            this.txtAlias = new System.Windows.Forms.TextBox();
            this.txtPrivateKey = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.btnBrowsePrivateKey = new System.Windows.Forms.Button();
            this.btnBrowseCertificate = new System.Windows.Forms.Button();
            this.txtCertificate = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(40, 27);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(29, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Alias";
            // 
            // txtAlias
            // 
            this.txtAlias.Location = new System.Drawing.Point(76, 25);
            this.txtAlias.Name = "txtAlias";
            this.txtAlias.Size = new System.Drawing.Size(304, 20);
            this.txtAlias.TabIndex = 1;
            // 
            // txtPrivateKey
            // 
            this.txtPrivateKey.Location = new System.Drawing.Point(76, 56);
            this.txtPrivateKey.Name = "txtPrivateKey";
            this.txtPrivateKey.ReadOnly = true;
            this.txtPrivateKey.Size = new System.Drawing.Size(264, 20);
            this.txtPrivateKey.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 59);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(61, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Private Key";
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(305, 157);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 11;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnAdd
            // 
            this.btnAdd.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnAdd.Location = new System.Drawing.Point(212, 157);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(75, 23);
            this.btnAdd.TabIndex = 10;
            this.btnAdd.Text = "Add";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // btnBrowsePrivateKey
            // 
            this.btnBrowsePrivateKey.Location = new System.Drawing.Point(347, 54);
            this.btnBrowsePrivateKey.Name = "btnBrowsePrivateKey";
            this.btnBrowsePrivateKey.Size = new System.Drawing.Size(33, 23);
            this.btnBrowsePrivateKey.TabIndex = 4;
            this.btnBrowsePrivateKey.Text = "...";
            this.btnBrowsePrivateKey.UseVisualStyleBackColor = true;
            this.btnBrowsePrivateKey.Click += new System.EventHandler(this.btnBrowsePrivateKey_Click);
            // 
            // btnBrowseCertificate
            // 
            this.btnBrowseCertificate.Location = new System.Drawing.Point(347, 83);
            this.btnBrowseCertificate.Name = "btnBrowseCertificate";
            this.btnBrowseCertificate.Size = new System.Drawing.Size(33, 23);
            this.btnBrowseCertificate.TabIndex = 7;
            this.btnBrowseCertificate.Text = "...";
            this.btnBrowseCertificate.UseVisualStyleBackColor = true;
            this.btnBrowseCertificate.Click += new System.EventHandler(this.btnBrowseCertificate_Click);
            // 
            // txtCertificate
            // 
            this.txtCertificate.Location = new System.Drawing.Point(76, 86);
            this.txtCertificate.Name = "txtCertificate";
            this.txtCertificate.ReadOnly = true;
            this.txtCertificate.Size = new System.Drawing.Size(264, 20);
            this.txtCertificate.TabIndex = 6;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(15, 89);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(54, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Certificate";
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(76, 112);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(304, 20);
            this.txtPassword.TabIndex = 8;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(17, 114);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(53, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "Password";
            // 
            // FormAddPrivateKey
            // 
            this.AcceptButton = this.btnAdd;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(455, 218);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.btnBrowseCertificate);
            this.Controls.Add(this.txtCertificate);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.btnBrowsePrivateKey);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.txtPrivateKey);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtAlias);
            this.Controls.Add(this.label1);
            this.Icon = global::VMCertStoreSnapIn.Resources.privateEntity;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "FormAddPrivateKey";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Add Private key";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtAlias;
        private System.Windows.Forms.TextBox txtPrivateKey;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Button btnBrowsePrivateKey;
        private System.Windows.Forms.Button btnBrowseCertificate;
        private System.Windows.Forms.TextBox txtCertificate;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.Label label4;
    }
}
