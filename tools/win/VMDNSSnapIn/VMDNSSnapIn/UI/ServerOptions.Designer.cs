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

namespace VMDNSSnapIn.UI
{
    partial class ServerOptions
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
            this.ServerOptionsTab = new System.Windows.Forms.TabControl();
            this.Forwarders = new System.Windows.Forms.TabPage();
            this.button4 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.ForwardersList = new System.Windows.Forms.ListBox();
            this.Add = new System.Windows.Forms.Button();
            this.ForwarderIPFieldText = new System.Windows.Forms.TextBox();
            this.ForwarderIPLabel = new System.Windows.Forms.Label();
            this.OKButton = new System.Windows.Forms.Button();
            this.ServerOptionsTab.SuspendLayout();
            this.Forwarders.SuspendLayout();
            this.SuspendLayout();
            // 
            // ServerOptionsTab
            // 
            this.ServerOptionsTab.Controls.Add(this.Forwarders);
            this.ServerOptionsTab.Location = new System.Drawing.Point(12, 12);
            this.ServerOptionsTab.Name = "ServerOptionsTab";
            this.ServerOptionsTab.SelectedIndex = 0;
            this.ServerOptionsTab.Size = new System.Drawing.Size(602, 305);
            this.ServerOptionsTab.TabIndex = 0;
            // 
            // Forwarders
            // 
            this.Forwarders.Controls.Add(this.button4);
            this.Forwarders.Controls.Add(this.button3);
            this.Forwarders.Controls.Add(this.button2);
            this.Forwarders.Controls.Add(this.ForwardersList);
            this.Forwarders.Controls.Add(this.Add);
            this.Forwarders.Controls.Add(this.ForwarderIPFieldText);
            this.Forwarders.Controls.Add(this.ForwarderIPLabel);
            this.Forwarders.Location = new System.Drawing.Point(4, 25);
            this.Forwarders.Name = "Forwarders";
            this.Forwarders.Padding = new System.Windows.Forms.Padding(3);
            this.Forwarders.Size = new System.Drawing.Size(594, 276);
            this.Forwarders.TabIndex = 0;
            this.Forwarders.Text = "Forwarders";
            this.Forwarders.UseVisualStyleBackColor = true;
            // 
            // button4
            // 
            this.button4.Location = new System.Drawing.Point(432, 163);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(75, 23);
            this.button4.TabIndex = 7;
            this.button4.Text = "Delete";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.DeleteForwarder);
            // 
            // button3
            // 
            this.button3.Enabled = false;
            this.button3.Location = new System.Drawing.Point(432, 121);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(75, 23);
            this.button3.TabIndex = 6;
            this.button3.Text = "Down";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.MoveForwarderDown);
            // 
            // button2
            // 
            this.button2.Enabled = false;
            this.button2.Location = new System.Drawing.Point(432, 82);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 5;
            this.button2.Text = "Up";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.MoveForwarderUp);
            // 
            // ForwardersList
            // 
            this.ForwardersList.FormattingEnabled = true;
            this.ForwardersList.ItemHeight = 16;
            this.ForwardersList.Location = new System.Drawing.Point(30, 82);
            this.ForwardersList.Name = "ForwardersList";
            this.ForwardersList.Size = new System.Drawing.Size(376, 164);
            this.ForwardersList.TabIndex = 4;
            // 
            // Add
            // 
            this.Add.Location = new System.Drawing.Point(432, 41);
            this.Add.Name = "Add";
            this.Add.Size = new System.Drawing.Size(75, 23);
            this.Add.TabIndex = 2;
            this.Add.Text = "Add";
            this.Add.UseVisualStyleBackColor = true;
            this.Add.Click += new System.EventHandler(this.AddForwarder);
            // 
            // ForwarderIPFieldText
            // 
            this.ForwarderIPFieldText.Location = new System.Drawing.Point(129, 41);
            this.ForwarderIPFieldText.Name = "ForwarderIPFieldText";
            this.ForwarderIPFieldText.Size = new System.Drawing.Size(277, 22);
            this.ForwarderIPFieldText.TabIndex = 1;
            // 
            // ForwarderIPLabel
            // 
            this.ForwarderIPLabel.AutoSize = true;
            this.ForwarderIPLabel.Location = new System.Drawing.Point(27, 41);
            this.ForwarderIPLabel.Name = "ForwarderIPLabel";
            this.ForwarderIPLabel.Size = new System.Drawing.Size(96, 17);
            this.ForwarderIPLabel.TabIndex = 0;
            this.ForwarderIPLabel.Text = "Forwarder IP :";
            // 
            // OKButton
            // 
            this.OKButton.Location = new System.Drawing.Point(539, 323);
            this.OKButton.Name = "OKButton";
            this.OKButton.Size = new System.Drawing.Size(75, 23);
            this.OKButton.TabIndex = 8;
            this.OKButton.Text = "OK";
            this.OKButton.UseVisualStyleBackColor = true;
            this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
            // 
            // ServerOptions
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(626, 357);
            this.Controls.Add(this.OKButton);
            this.Controls.Add(this.ServerOptionsTab);
            this.Name = "ServerOptions";
            this.Text = "ServerOptions";
            this.ServerOptionsTab.ResumeLayout(false);
            this.Forwarders.ResumeLayout(false);
            this.Forwarders.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl ServerOptionsTab;
        private System.Windows.Forms.TabPage Forwarders;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.ListBox ForwardersList;
        private System.Windows.Forms.Button Add;
        private System.Windows.Forms.TextBox ForwarderIPFieldText;
        private System.Windows.Forms.Label ForwarderIPLabel;
        private System.Windows.Forms.Button OKButton;
    }
}