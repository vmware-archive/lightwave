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
namespace VMCASnapIn.UI
{
    partial class FormAddRootCert
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
            this.txtPrivateKey = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.btnBrowsePrivateKey = new System.Windows.Forms.Button();
            this.btnBrowseCertificate = new System.Windows.Forms.Button();
            this.txtCertificate = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // txtPrivateKey
            // 
            this.txtPrivateKey.Location = new System.Drawing.Point(76, 56);
            this.txtPrivateKey.Name = "txtPrivateKey";
            this.txtPrivateKey.ReadOnly = true;
            this.txtPrivateKey.Size = new System.Drawing.Size(264, 20);
            this.txtPrivateKey.TabIndex = 2;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 59);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(61, 13);
            this.label2.TabIndex = 7;
            this.label2.Text = "Private Key";
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(305, 110);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnAdd
            // 
            this.btnAdd.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnAdd.Location = new System.Drawing.Point(212, 110);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(75, 23);
            this.btnAdd.TabIndex = 4;
            this.btnAdd.Text = "Add";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // btnBrowsePrivateKey
            // 
            this.btnBrowsePrivateKey.Location = new System.Drawing.Point(347, 54);
            this.btnBrowsePrivateKey.Name = "btnBrowsePrivateKey";
            this.btnBrowsePrivateKey.Size = new System.Drawing.Size(33, 23);
            this.btnBrowsePrivateKey.TabIndex = 3;
            this.btnBrowsePrivateKey.Text = "...";
            this.btnBrowsePrivateKey.UseVisualStyleBackColor = true;
            this.btnBrowsePrivateKey.Click += new System.EventHandler(this.btnBrowsePrivateKey_Click);
            // 
            // btnBrowseCertificate
            // 
            this.btnBrowseCertificate.Location = new System.Drawing.Point(347, 25);
            this.btnBrowseCertificate.Name = "btnBrowseCertificate";
            this.btnBrowseCertificate.Size = new System.Drawing.Size(33, 23);
            this.btnBrowseCertificate.TabIndex = 1;
            this.btnBrowseCertificate.Text = "...";
            this.btnBrowseCertificate.UseVisualStyleBackColor = true;
            this.btnBrowseCertificate.Click += new System.EventHandler(this.btnBrowseCertificate_Click);
            // 
            // txtCertificate
            // 
            this.txtCertificate.Location = new System.Drawing.Point(76, 28);
            this.txtCertificate.Name = "txtCertificate";
            this.txtCertificate.ReadOnly = true;
            this.txtCertificate.Size = new System.Drawing.Size(264, 20);
            this.txtCertificate.TabIndex = 0;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(15, 31);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(54, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Certificate";
            // 
            // FormAddRootCert
            // 
            this.AcceptButton = this.btnAdd;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(455, 166);
            this.Controls.Add(this.btnBrowseCertificate);
            this.Controls.Add(this.txtCertificate);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.btnBrowsePrivateKey);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.txtPrivateKey);
            this.Controls.Add(this.label2);
            this.Icon = VMCASnapInEnvironment.Instance.GetIconResource(VMCAIconIndex.rootCert);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "FormAddRootCert";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Add Root Certificate";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtPrivateKey;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Button btnBrowsePrivateKey;
        private System.Windows.Forms.Button btnBrowseCertificate;
        private System.Windows.Forms.TextBox txtCertificate;
        private System.Windows.Forms.Label label3;
    }
}
