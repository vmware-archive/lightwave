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
    partial class AddToGroup
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
            this.cnTextBox = new System.Windows.Forms.TextBox();
            this.findCnButton = new System.Windows.Forms.Button();
            this.dnLabel = new System.Windows.Forms.Label();
            this.submitButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(31, 33);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(70, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Group Name:";
            // 
            // cnTextBox
            // 
            this.cnTextBox.Location = new System.Drawing.Point(123, 26);
            this.cnTextBox.Name = "cnTextBox";
            this.cnTextBox.Size = new System.Drawing.Size(200, 20);
            this.cnTextBox.TabIndex = 1;
            // 
            // findCnButton
            // 
            this.findCnButton.Location = new System.Drawing.Point(346, 23);
            this.findCnButton.Name = "findCnButton";
            this.findCnButton.Size = new System.Drawing.Size(75, 23);
            this.findCnButton.TabIndex = 2;
            this.findCnButton.Text = "Find CN";
            this.findCnButton.UseVisualStyleBackColor = true;
            this.findCnButton.Click += new System.EventHandler(this.findCnButton_Click);
            // 
            // dnLabel
            // 
            this.dnLabel.Location = new System.Drawing.Point(34, 64);
            this.dnLabel.Name = "dnLabel";
            this.dnLabel.Size = new System.Drawing.Size(387, 32);
            this.dnLabel.TabIndex = 5;
            this.dnLabel.Text = "dnlabel";
            // 
            // submitButton
            // 
            this.submitButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.submitButton.Location = new System.Drawing.Point(248, 111);
            this.submitButton.Name = "submitButton";
            this.submitButton.Size = new System.Drawing.Size(75, 23);
            this.submitButton.TabIndex = 13;
            this.submitButton.Text = "Submit";
            this.submitButton.UseVisualStyleBackColor = true;
            this.submitButton.Click += new System.EventHandler(this.submitButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.Location = new System.Drawing.Point(167, 111);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 12;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // AddToGroup
            // 
            this.AcceptButton = this.submitButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(458, 162);
            this.Controls.Add(this.submitButton);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.dnLabel);
            this.Controls.Add(this.findCnButton);
            this.Controls.Add(this.cnTextBox);
            this.Controls.Add(this.label1);
            this.Name = "AddToGroup";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "AddToGroup";
            this.Icon = VMDirEnvironment.Instance.GetIconResource(VMDirIconIndex.AddToGroup);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox cnTextBox;
        private System.Windows.Forms.Button findCnButton;
        private System.Windows.Forms.Label dnLabel;
        private System.Windows.Forms.Button submitButton;
        private System.Windows.Forms.Button cancelButton;

    }
}