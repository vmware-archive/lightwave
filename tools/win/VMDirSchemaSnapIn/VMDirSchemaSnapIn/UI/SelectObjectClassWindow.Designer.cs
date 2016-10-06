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

    partial class SelectObjectClassWindow

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

            this.SearchTextBox = new System.Windows.Forms.TextBox();

            this.FromItemsList = new System.Windows.Forms.ListBox();

            this.SelectButton = new System.Windows.Forms.Button();

            this.SearchButton = new System.Windows.Forms.Button();

            this.ResetButton = new System.Windows.Forms.Button();

            this.SuspendLayout();

            // 

            // label1

            // 

            this.label1.AutoSize = true;

            this.label1.Location = new System.Drawing.Point(21, 41);

            this.label1.Name = "label1";

            this.label1.Size = new System.Drawing.Size(214, 17);

            this.label1.TabIndex = 0;

            this.label1.Text = "Select Items from the box below :";

            // 

            // SearchTextBox

            // 

            this.SearchTextBox.Location = new System.Drawing.Point(24, 78);

            this.SearchTextBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.SearchTextBox.Name = "SearchTextBox";

            this.SearchTextBox.Size = new System.Drawing.Size(178, 22);

            this.SearchTextBox.TabIndex = 1;

            // 

            // FromItemsList

            // 

            this.FromItemsList.FormattingEnabled = true;

            this.FromItemsList.ItemHeight = 16;

            this.FromItemsList.Location = new System.Drawing.Point(24, 119);

            this.FromItemsList.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.FromItemsList.Name = "FromItemsList";

            this.FromItemsList.Size = new System.Drawing.Size(211, 180);

            this.FromItemsList.TabIndex = 2;

            // 

            // SelectButton

            // 

            this.SelectButton.Location = new System.Drawing.Point(241, 277);

            this.SelectButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.SelectButton.Name = "SelectButton";

            this.SelectButton.Size = new System.Drawing.Size(92, 23);

            this.SelectButton.TabIndex = 3;

            this.SelectButton.Text = "Select";

            this.SelectButton.UseVisualStyleBackColor = true;

            this.SelectButton.Click += new System.EventHandler(this.SelectButton_Click);

            // 

            // SearchButton

            // 

            this.SearchButton.Location = new System.Drawing.Point(208, 78);

            this.SearchButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.SearchButton.Name = "SearchButton";

            this.SearchButton.Size = new System.Drawing.Size(62, 23);

            this.SearchButton.TabIndex = 4;

            this.SearchButton.Text = "Search";

            this.SearchButton.UseVisualStyleBackColor = true;

            this.SearchButton.Click += new System.EventHandler(this.SearchButton_Click);

            // 

            // ResetButton

            // 

            this.ResetButton.Location = new System.Drawing.Point(276, 78);

            this.ResetButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.ResetButton.Name = "ResetButton";

            this.ResetButton.Size = new System.Drawing.Size(57, 23);

            this.ResetButton.TabIndex = 5;

            this.ResetButton.Text = "Reset";

            this.ResetButton.UseVisualStyleBackColor = true;

            this.ResetButton.Click += new System.EventHandler(this.Reset_Click);

            // 

            // SelectObjectClassWindow

            // 

            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);

            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;

            this.ClientSize = new System.Drawing.Size(362, 338);

            this.Controls.Add(this.ResetButton);

            this.Controls.Add(this.SearchButton);

            this.Controls.Add(this.SelectButton);

            this.Controls.Add(this.FromItemsList);

            this.Controls.Add(this.SearchTextBox);

            this.Controls.Add(this.label1);

            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);

            this.MaximizeBox = false;

            this.Name = "SelectObjectClassWindow";

            this.Text = "SelectObjectClassWindow";

            this.ResumeLayout(false);

            this.PerformLayout();



        }



        #endregion



        private System.Windows.Forms.Label label1;

        private System.Windows.Forms.TextBox SearchTextBox;

        private System.Windows.Forms.ListBox FromItemsList;

        private System.Windows.Forms.Button SelectButton;

        private System.Windows.Forms.Button SearchButton;

        private System.Windows.Forms.Button ResetButton;

    }

}