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
    partial class SolutionUserCertProperty
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
            this.txtDN = new System.Windows.Forms.TextBox();
            this.btnShowCertificate = new System.Windows.Forms.Button();
            this.btnSelectCert = new System.Windows.Forms.Button();
            this.txtValidTo = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtValidFrom = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.txtIssuedBy = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            //
            // txtDN
            //
            this.txtDN.Location = new System.Drawing.Point(17, 164);
            this.txtDN.Multiline = true;
            this.txtDN.Name = "txtDN";
            this.txtDN.ReadOnly = true;
            this.txtDN.Size = new System.Drawing.Size(310, 53);
            this.txtDN.TabIndex = 6;
            //
            // btnShowCertificate
            //
            this.btnShowCertificate.Location = new System.Drawing.Point(17, 233);
            this.btnShowCertificate.Name = "btnShowCertificate";
            this.btnShowCertificate.Size = new System.Drawing.Size(106, 23);
            this.btnShowCertificate.TabIndex = 7;
            this.btnShowCertificate.Text = "Show certificate";
            this.btnShowCertificate.UseVisualStyleBackColor = true;
            this.btnShowCertificate.Click += new System.EventHandler(this.btnShowCertificate_Click);
            //
            // btnSelectCert
            //
            this.btnSelectCert.Location = new System.Drawing.Point(221, 233);
            this.btnSelectCert.Name = "btnSelectCert";
            this.btnSelectCert.Size = new System.Drawing.Size(106, 23);
            this.btnSelectCert.TabIndex = 8;
            this.btnSelectCert.Text = "Change certificate";
            this.btnSelectCert.UseVisualStyleBackColor = true;
            this.btnSelectCert.Click += new System.EventHandler(this.btnSelectCert_Click);
            //
            // txtValidTo
            //
            this.txtValidTo.Location = new System.Drawing.Point(93, 113);
            this.txtValidTo.Name = "txtValidTo";
            this.txtValidTo.ReadOnly = true;
            this.txtValidTo.Size = new System.Drawing.Size(234, 20);
            this.txtValidTo.TabIndex = 5;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(14, 116);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(45, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Valid to:";
            //
            // txtValidFrom
            //
            this.txtValidFrom.Location = new System.Drawing.Point(93, 87);
            this.txtValidFrom.Name = "txtValidFrom";
            this.txtValidFrom.ReadOnly = true;
            this.txtValidFrom.Size = new System.Drawing.Size(234, 20);
            this.txtValidFrom.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(14, 90);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(56, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Valid from:";
            // 
            // txtIssuedBy
            // 
            this.txtIssuedBy.Location = new System.Drawing.Point(93, 59);
            this.txtIssuedBy.Name = "txtIssuedBy";
            this.txtIssuedBy.ReadOnly = true;
            this.txtIssuedBy.Size = new System.Drawing.Size(234, 20);
            this.txtIssuedBy.TabIndex = 1;
            //
            // label1
            //
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(14, 62);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(55, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Issued by:";
            //
            // SolutionUserCertProperty
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.txtIssuedBy);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtValidTo);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.txtValidFrom);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnSelectCert);
            this.Controls.Add(this.btnShowCertificate);
            this.Controls.Add(this.txtDN);
            this.Name = "SolutionUserCertProperty";
            this.Size = new System.Drawing.Size(350, 359);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtDN;
        private System.Windows.Forms.Button btnShowCertificate;
        private System.Windows.Forms.Button btnSelectCert;
        private System.Windows.Forms.TextBox txtValidTo;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtValidFrom;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtIssuedBy;
        private System.Windows.Forms.Label label1;


    }
}
