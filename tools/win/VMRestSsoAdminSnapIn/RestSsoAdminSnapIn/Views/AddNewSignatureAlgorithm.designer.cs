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
    partial class AddNewSignatureAlgorithm
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
            this.lblName = new System.Windows.Forms.Label();
            this.horizontalLine = new System.Windows.Forms.GroupBox();
            this.btnAdd = new System.Windows.Forms.Button();
            this.btnClose = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.nudMaxKeySize = new System.Windows.Forms.NumericUpDown();
            this.nudMinKeySize = new System.Windows.Forms.NumericUpDown();
            this.nudPriority = new System.Windows.Forms.NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)(this.nudMaxKeySize)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudMinKeySize)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPriority)).BeginInit();
            this.SuspendLayout();
            // 
            // lblName
            // 
            this.lblName.AutoSize = true;
            this.lblName.Location = new System.Drawing.Point(8, 25);
            this.lblName.Name = "lblName";
            this.lblName.Size = new System.Drawing.Size(77, 13);
            this.lblName.TabIndex = 0;
            this.lblName.Text = "MAX Key Size:";
            // 
            // horizontalLine
            // 
            this.horizontalLine.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.horizontalLine.Location = new System.Drawing.Point(7, 107);
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.Size = new System.Drawing.Size(218, 10);
            this.horizontalLine.TabIndex = 37;
            this.horizontalLine.TabStop = false;
            // 
            // btnAdd
            // 
            this.btnAdd.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnAdd.Location = new System.Drawing.Point(67, 124);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(75, 23);
            this.btnAdd.TabIndex = 4;
            this.btnAdd.Text = "&Add";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnCreate_Click);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(148, 124);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 5;
            this.btnClose.Text = "Cl&ose";
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 53);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(74, 13);
            this.label1.TabIndex = 38;
            this.label1.Text = "MIN Key Size:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 81);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(41, 13);
            this.label2.TabIndex = 40;
            this.label2.Text = "Priority:";
            // 
            // nudMaxKeySize
            // 
            this.nudMaxKeySize.Location = new System.Drawing.Point(104, 23);
            this.nudMaxKeySize.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.nudMaxKeySize.Name = "nudMaxKeySize";
            this.nudMaxKeySize.Size = new System.Drawing.Size(120, 20);
            this.nudMaxKeySize.TabIndex = 1;
            // 
            // nudMinKeySize
            // 
            this.nudMinKeySize.Location = new System.Drawing.Point(104, 49);
            this.nudMinKeySize.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.nudMinKeySize.Name = "nudMinKeySize";
            this.nudMinKeySize.Size = new System.Drawing.Size(120, 20);
            this.nudMinKeySize.TabIndex = 2;
            // 
            // nudPriority
            // 
            this.nudPriority.Location = new System.Drawing.Point(104, 81);
            this.nudPriority.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nudPriority.Name = "nudPriority";
            this.nudPriority.Size = new System.Drawing.Size(120, 20);
            this.nudPriority.TabIndex = 3;
            this.nudPriority.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // AddNewSignatureAlgorithm
            // 
            this.AcceptButton = this.btnAdd;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(232, 155);
            this.Controls.Add(this.nudPriority);
            this.Controls.Add(this.nudMinKeySize);
            this.Controls.Add(this.nudMaxKeySize);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lblName);
            this.Controls.Add(this.horizontalLine);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.btnClose);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AddNewSignatureAlgorithm";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Add Signature Algorithm";
            ((System.ComponentModel.ISupportInitialize)(this.nudMaxKeySize)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudMinKeySize)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPriority)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblName;
        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown nudMaxKeySize;
        private System.Windows.Forms.NumericUpDown nudMinKeySize;
        private System.Windows.Forms.NumericUpDown nudPriority;
    }
}