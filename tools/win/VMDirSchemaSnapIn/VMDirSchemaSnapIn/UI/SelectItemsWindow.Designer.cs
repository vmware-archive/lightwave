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

namespace VMDirSchemaSnapIn.UI

{

    partial class SelectItemsWindow

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

            this.FromList = new System.Windows.Forms.ListBox();

            this.ToList = new System.Windows.Forms.ListBox();

            this.AddButton = new System.Windows.Forms.Button();

            this.RemoveButton = new System.Windows.Forms.Button();

            this.ApplyButton = new System.Windows.Forms.Button();

            this.label1 = new System.Windows.Forms.Label();

            this.SearchTextBox = new System.Windows.Forms.TextBox();

            this.CancelButton = new System.Windows.Forms.Button();

            this.SearchButton = new System.Windows.Forms.Button();

            this.ResetButton = new System.Windows.Forms.Button();

            this.SuspendLayout();

            // 

            // FromList

            // 

            this.FromList.FormattingEnabled = true;

            this.FromList.ItemHeight = 16;

            this.FromList.Location = new System.Drawing.Point(41, 129);

            this.FromList.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.FromList.Name = "FromList";

            this.FromList.Size = new System.Drawing.Size(164, 132);

            this.FromList.TabIndex = 0;

            // 

            // ToList

            // 

            this.ToList.FormattingEnabled = true;

            this.ToList.ItemHeight = 16;

            this.ToList.Location = new System.Drawing.Point(283, 92);

            this.ToList.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.ToList.Name = "ToList";

            this.ToList.Size = new System.Drawing.Size(160, 164);

            this.ToList.TabIndex = 1;

            // 

            // AddButton

            // 

            this.AddButton.Location = new System.Drawing.Point(222, 172);

            this.AddButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.AddButton.Name = "AddButton";

            this.AddButton.Size = new System.Drawing.Size(29, 23);

            this.AddButton.TabIndex = 2;

            this.AddButton.Text = ">";

            this.AddButton.UseVisualStyleBackColor = true;

            this.AddButton.Click += new System.EventHandler(this.AddButton_Click);

            // 

            // RemoveButton

            // 

            this.RemoveButton.Location = new System.Drawing.Point(222, 199);

            this.RemoveButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.RemoveButton.Name = "RemoveButton";

            this.RemoveButton.Size = new System.Drawing.Size(29, 23);

            this.RemoveButton.TabIndex = 3;

            this.RemoveButton.Text = "<";

            this.RemoveButton.UseVisualStyleBackColor = true;

            this.RemoveButton.Click += new System.EventHandler(this.RemoveButton_Click);

            // 

            // ApplyButton

            // 

            this.ApplyButton.Location = new System.Drawing.Point(243, 295);

            this.ApplyButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.ApplyButton.Name = "ApplyButton";

            this.ApplyButton.Size = new System.Drawing.Size(75, 23);

            this.ApplyButton.TabIndex = 4;

            this.ApplyButton.Text = "Apply";

            this.ApplyButton.UseVisualStyleBackColor = true;

            this.ApplyButton.Click += new System.EventHandler(this.ApplyButton_Click);

            // 

            // label1

            // 

            this.label1.AutoSize = true;

            this.label1.Location = new System.Drawing.Point(37, 53);

            this.label1.Name = "label1";

            this.label1.Size = new System.Drawing.Size(214, 17);

            this.label1.TabIndex = 5;

            this.label1.Text = "Select items from the box below :";

            // 

            // SearchTextBox

            // 

            this.SearchTextBox.Location = new System.Drawing.Point(41, 77);

            this.SearchTextBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.SearchTextBox.Name = "SearchTextBox";

            this.SearchTextBox.Size = new System.Drawing.Size(164, 22);

            this.SearchTextBox.TabIndex = 6;

            // 

            // CancelButton

            // 

            this.CancelButton.Location = new System.Drawing.Point(368, 295);

            this.CancelButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.CancelButton.Name = "CancelButton";

            this.CancelButton.Size = new System.Drawing.Size(75, 23);

            this.CancelButton.TabIndex = 7;

            this.CancelButton.Text = "Close";

            this.CancelButton.UseVisualStyleBackColor = true;

            this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);

            // 

            // SearchButton

            // 

            this.SearchButton.Location = new System.Drawing.Point(213, 74);

            this.SearchButton.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);

            this.SearchButton.Name = "SearchButton";

            this.SearchButton.Size = new System.Drawing.Size(63, 25);

            this.SearchButton.TabIndex = 8;

            this.SearchButton.Text = "Search";

            this.SearchButton.UseVisualStyleBackColor = true;

            this.SearchButton.Click += new System.EventHandler(this.SearchButton_Click);

            // 

            // ResetButton

            // 

            this.ResetButton.Location = new System.Drawing.Point(213, 107);

            this.ResetButton.Margin = new System.Windows.Forms.Padding(4);

            this.ResetButton.Name = "ResetButton";

            this.ResetButton.Size = new System.Drawing.Size(63, 25);

            this.ResetButton.TabIndex = 9;

            this.ResetButton.Text = "Reset";

            this.ResetButton.UseVisualStyleBackColor = true;

            this.ResetButton.Click += new System.EventHandler(this.ResetButton_Click);

            // 

            // SelectItemsWindow

            // 

            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);

            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;

            this.ClientSize = new System.Drawing.Size(477, 359);

            this.Controls.Add(this.ResetButton);

            this.Controls.Add(this.SearchButton);

            this.Controls.Add(this.CancelButton);

            this.Controls.Add(this.SearchTextBox);

            this.Controls.Add(this.label1);

            this.Controls.Add(this.ApplyButton);

            this.Controls.Add(this.RemoveButton);

            this.Controls.Add(this.AddButton);

            this.Controls.Add(this.ToList);

            this.Controls.Add(this.FromList);

            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.MaximizeBox = false;

            this.Name = "SelectItemsWindow";

            this.Text = "Select Items";

            this.ResumeLayout(false);

            this.PerformLayout();



        }



        #endregion



        private System.Windows.Forms.ListBox FromList;

        private System.Windows.Forms.ListBox ToList;

        private System.Windows.Forms.Button AddButton;

        private System.Windows.Forms.Button RemoveButton;

        private System.Windows.Forms.Button ApplyButton;

        private System.Windows.Forms.Label label1;

        private System.Windows.Forms.TextBox SearchTextBox;

        private System.Windows.Forms.Button CancelButton;

        private System.Windows.Forms.Button SearchButton;

        private System.Windows.Forms.Button ResetButton;

    }

}