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
    partial class UserRolesProperty
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
            this.label2 = new System.Windows.Forms.Label();
            this.cboRole = new System.Windows.Forms.ComboBox();
            this.chkActAsUser = new System.Windows.Forms.CheckBox();
            this.chkIDPAdmin = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 39);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(32, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "Role:";
            // 
            // cboRole
            // 
            this.cboRole.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cboRole.FormattingEnabled = true;
            this.cboRole.Items.AddRange(new object[] {
            "GuestUser",
            "RegularUser",
            "Administrator"});
            this.cboRole.Location = new System.Drawing.Point(86, 36);
            this.cboRole.Name = "cboRole";
            this.cboRole.Size = new System.Drawing.Size(229, 21);
            this.cboRole.TabIndex = 1;
            // 
            // chkActAsUser
            // 
            this.chkActAsUser.AutoSize = true;
            this.chkActAsUser.Location = new System.Drawing.Point(16, 80);
            this.chkActAsUser.Name = "chkActAsUser";
            this.chkActAsUser.Size = new System.Drawing.Size(79, 17);
            this.chkActAsUser.TabIndex = 2;
            this.chkActAsUser.Text = "Act as user";
            this.chkActAsUser.UseVisualStyleBackColor = true;
            // 
            // chkIDPAdmin
            // 
            this.chkIDPAdmin.AutoSize = true;
            this.chkIDPAdmin.Location = new System.Drawing.Point(16, 114);
            this.chkIDPAdmin.Name = "chkIDPAdmin";
            this.chkIDPAdmin.Size = new System.Drawing.Size(106, 17);
            this.chkIDPAdmin.TabIndex = 3;
            this.chkIDPAdmin.Text = "IDP administrator";
            this.chkIDPAdmin.UseVisualStyleBackColor = true;
            this.chkIDPAdmin.Visible = false;
            // 
            // UserRolesProperty
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.chkIDPAdmin);
            this.Controls.Add(this.chkActAsUser);
            this.Controls.Add(this.cboRole);
            this.Controls.Add(this.label2);
            this.Name = "UserRolesProperty";
            this.Size = new System.Drawing.Size(350, 359);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cboRole;
        private System.Windows.Forms.CheckBox chkActAsUser;
        private System.Windows.Forms.CheckBox chkIDPAdmin;


    }
}
