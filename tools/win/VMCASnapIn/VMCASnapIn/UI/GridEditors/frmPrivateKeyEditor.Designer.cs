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
namespace VMCASnapIn.UI.GridEditors
{
    partial class frmPrivateKeyEditor
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.numKeyLength = new System.Windows.Forms.NumericUpDown();
            this.btnCreate = new System.Windows.Forms.Button();
            this.txtFileName = new System.Windows.Forms.TextBox();
            this.btnBrowse = new System.Windows.Forms.Button();
            this.txtKeyData = new System.Windows.Forms.TextBox();
            this.rdoPaste = new System.Windows.Forms.RadioButton();
            this.rdoSelect = new System.Windows.Forms.RadioButton();
            this.rdoCreate = new System.Windows.Forms.RadioButton();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numKeyLength)).BeginInit();
            this.SuspendLayout();
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(377, 279);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 0;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnOK
            // 
            this.btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnOK.Location = new System.Drawing.Point(286, 279);
            this.btnOK.Name = "btnOK";
            this.btnOK.Size = new System.Drawing.Size(75, 23);
            this.btnOK.TabIndex = 1;
            this.btnOK.Text = "OK";
            this.btnOK.UseVisualStyleBackColor = true;
            this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.numKeyLength);
            this.groupBox1.Controls.Add(this.btnCreate);
            this.groupBox1.Controls.Add(this.txtFileName);
            this.groupBox1.Controls.Add(this.btnBrowse);
            this.groupBox1.Controls.Add(this.txtKeyData);
            this.groupBox1.Controls.Add(this.rdoPaste);
            this.groupBox1.Controls.Add(this.rdoSelect);
            this.groupBox1.Controls.Add(this.rdoCreate);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(440, 256);
            this.groupBox1.TabIndex = 2;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Select Or Create Private Key";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(61, 61);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(64, 13);
            this.label1.TabIndex = 8;
            this.label1.Text = "Key Length:";
            // 
            // numKeyLength
            // 
            this.numKeyLength.Increment = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.numKeyLength.Location = new System.Drawing.Point(131, 59);
            this.numKeyLength.Maximum = new decimal(new int[] {
            8192,
            0,
            0,
            0});
            this.numKeyLength.Minimum = new decimal(new int[] {
            2048,
            0,
            0,
            0});
            this.numKeyLength.Name = "numKeyLength";
            this.numKeyLength.Size = new System.Drawing.Size(120, 20);
            this.numKeyLength.TabIndex = 7;
            this.numKeyLength.Value = new decimal(new int[] {
            2048,
            0,
            0,
            0});
            // 
            // btnCreate
            // 
            this.btnCreate.Location = new System.Drawing.Point(257, 56);
            this.btnCreate.Name = "btnCreate";
            this.btnCreate.Size = new System.Drawing.Size(72, 23);
            this.btnCreate.TabIndex = 6;
            this.btnCreate.Text = "Create";
            this.btnCreate.UseVisualStyleBackColor = true;
            this.btnCreate.Click += new System.EventHandler(this.btnCreate_Click);
            // 
            // txtFileName
            // 
            this.txtFileName.Enabled = false;
            this.txtFileName.Location = new System.Drawing.Point(64, 116);
            this.txtFileName.Name = "txtFileName";
            this.txtFileName.Size = new System.Drawing.Size(285, 20);
            this.txtFileName.TabIndex = 5;
            // 
            // btnBrowse
            // 
            this.btnBrowse.Enabled = false;
            this.btnBrowse.Location = new System.Drawing.Point(358, 114);
            this.btnBrowse.Name = "btnBrowse";
            this.btnBrowse.Size = new System.Drawing.Size(44, 23);
            this.btnBrowse.TabIndex = 4;
            this.btnBrowse.Text = "...";
            this.btnBrowse.UseVisualStyleBackColor = true;
            this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
            // 
            // txtKeyData
            // 
            this.txtKeyData.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.txtKeyData.Enabled = false;
            this.txtKeyData.Location = new System.Drawing.Point(64, 180);
            this.txtKeyData.Multiline = true;
            this.txtKeyData.Name = "txtKeyData";
            this.txtKeyData.Size = new System.Drawing.Size(370, 70);
            this.txtKeyData.TabIndex = 3;
            // 
            // rdoPaste
            // 
            this.rdoPaste.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.rdoPaste.AutoSize = true;
            this.rdoPaste.Location = new System.Drawing.Point(42, 155);
            this.rdoPaste.Name = "rdoPaste";
            this.rdoPaste.Size = new System.Drawing.Size(121, 17);
            this.rdoPaste.TabIndex = 2;
            this.rdoPaste.Text = "Paste from clipboard";
            this.rdoPaste.UseVisualStyleBackColor = true;
            this.rdoPaste.CheckedChanged += new System.EventHandler(this.rdoPaste_CheckedChanged);
            // 
            // rdoSelect
            // 
            this.rdoSelect.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.rdoSelect.AutoSize = true;
            this.rdoSelect.Location = new System.Drawing.Point(42, 95);
            this.rdoSelect.Name = "rdoSelect";
            this.rdoSelect.Size = new System.Drawing.Size(132, 17);
            this.rdoSelect.TabIndex = 1;
            this.rdoSelect.Text = "Select from existing file";
            this.rdoSelect.UseVisualStyleBackColor = true;
            this.rdoSelect.CheckedChanged += new System.EventHandler(this.rdoSelect_CheckedChanged);
            // 
            // rdoCreate
            // 
            this.rdoCreate.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.rdoCreate.AutoSize = true;
            this.rdoCreate.Checked = true;
            this.rdoCreate.Location = new System.Drawing.Point(42, 35);
            this.rdoCreate.Name = "rdoCreate";
            this.rdoCreate.Size = new System.Drawing.Size(79, 17);
            this.rdoCreate.TabIndex = 0;
            this.rdoCreate.TabStop = true;
            this.rdoCreate.Text = "Create new";
            this.rdoCreate.UseVisualStyleBackColor = true;
            this.rdoCreate.CheckedChanged += new System.EventHandler(this.rdoCreate_CheckedChanged);
            // 
            // frmPrivateKeyEditor
            // 
            this.AcceptButton = this.btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(464, 315);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.btnOK);
            this.Controls.Add(this.btnCancel);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Icon = VMCASnapInEnvironment.Instance.GetIconResource(VMCAIconIndex.privateKey);
            this.Name = "frmPrivateKeyEditor";
            this.Text = "Select or Create Private Key";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numKeyLength)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox txtFileName;
        private System.Windows.Forms.Button btnBrowse;
        private System.Windows.Forms.TextBox txtKeyData;
        private System.Windows.Forms.RadioButton rdoPaste;
        private System.Windows.Forms.RadioButton rdoSelect;
        private System.Windows.Forms.RadioButton rdoCreate;
        private System.Windows.Forms.Button btnCreate;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown numKeyLength;
    }
}