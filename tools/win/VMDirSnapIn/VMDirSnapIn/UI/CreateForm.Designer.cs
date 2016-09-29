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
namespace VMDirSnapIn.UI
{

    partial class CreateForm
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
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnOK = new System.Windows.Forms.Button();
            this.listViewProp = new System.Windows.Forms.ListView();
            this.textBoxEdit = new System.Windows.Forms.MaskedTextBox();
            this.textBoxParentDn = new System.Windows.Forms.TextBox();
            this.textBoxRdn = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(201, 411);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 2;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnOK
            // 
            this.btnOK.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnOK.Location = new System.Drawing.Point(119, 411);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 3;
            this.btnOK.Text = "Create";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // listViewProp
            // 
            this.listViewProp.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewProp.FullRowSelect = true;
            this.listViewProp.GridLines = true;
            this.listViewProp.HideSelection = false;
            this.listViewProp.Location = new System.Drawing.Point(12, 70);
            this.listViewProp.Name = "listViewProp";
            this.listViewProp.Size = new System.Drawing.Size(402, 330);
            this.listViewProp.TabIndex = 4;
            this.listViewProp.UseCompatibleStateImageBehavior = false;
            this.listViewProp.View = System.Windows.Forms.View.Details;
            this.listViewProp.DoubleClick += new System.EventHandler(this.listViewProp_DoubleClick);
            // 
            // textBoxEdit
            // 
            this.textBoxEdit.Location = new System.Drawing.Point(12, 414);
            this.textBoxEdit.Name = "textBoxEdit";
            this.textBoxEdit.Size = new System.Drawing.Size(36, 20);
            this.textBoxEdit.TabIndex = 6;
            this.textBoxEdit.Visible = false;
            this.textBoxEdit.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textBoxEdit_KeyPress);
            this.textBoxEdit.LostFocus += new System.EventHandler(this.textBoxEdit_LostFocus);
            // 
            // textBoxParentDn
            // 
            this.textBoxParentDn.Enabled = false;
            this.textBoxParentDn.Location = new System.Drawing.Point(75, 13);
            this.textBoxParentDn.Name = "textBoxParentDn";
            this.textBoxParentDn.Size = new System.Drawing.Size(339, 20);
            this.textBoxParentDn.TabIndex = 7;
            // 
            // textBoxRdn
            // 
            this.textBoxRdn.Location = new System.Drawing.Point(77, 39);
            this.textBoxRdn.Name = "textBoxRdn";
            this.textBoxRdn.Size = new System.Drawing.Size(337, 20);
            this.textBoxRdn.TabIndex = 8;
            this.textBoxRdn.Text = "cn=";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 20);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 9;
            this.label1.Text = "Parent DN:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 46);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(62, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Enter RDN:";
            // 
            // CreateForm
            // 
            this.AcceptButton = this.btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(426, 446);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBoxRdn);
            this.Controls.Add(this.textBoxParentDn);
            this.Controls.Add(this.textBoxEdit);
            this.Controls.Add(this.listViewProp);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.btnCancel);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "CreateForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "CreateForm";
            this.Icon = VMDirEnvironment.Instance.GetIconResource(VMDirIconIndex.Object);
            this.ResumeLayout(false);
            this.PerformLayout();

        }



        #endregion


        private System.Windows.Forms.Button btnCancel;

        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.ListView listViewProp;
        private System.Windows.Forms.MaskedTextBox textBoxEdit;
        private System.Windows.Forms.TextBox textBoxParentDn;
        private System.Windows.Forms.TextBox textBoxRdn;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;

    }

}