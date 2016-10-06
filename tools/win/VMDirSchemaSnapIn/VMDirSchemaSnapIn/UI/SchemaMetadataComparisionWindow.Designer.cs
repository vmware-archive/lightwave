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

    partial class SchemaMetadataComparisionWindow

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

            this.label2 = new System.Windows.Forms.Label();

            this.SchemaButton = new System.Windows.Forms.RadioButton();

            this.MetaDataButton = new System.Windows.Forms.RadioButton();

            this.NodesList = new System.Windows.Forms.ListBox();

            this.label3 = new System.Windows.Forms.Label();

            this.CompareButton = new System.Windows.Forms.Button();

            this.ViewAttributeTypeDiffButton = new System.Windows.Forms.Button();

            this.ViewObjectClassDiffButton = new System.Windows.Forms.Button();

            this.CompareOptionsGroup = new System.Windows.Forms.GroupBox();

            this.CompareOptionsGroup.SuspendLayout();

            this.SuspendLayout();

            // 

            // label1

            // 

            this.label1.AutoSize = true;

            this.label1.Location = new System.Drawing.Point(38, 39);

            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);

            this.label1.Name = "label1";

            this.label1.Size = new System.Drawing.Size(502, 13);

            this.label1.TabIndex = 0;

            this.label1.Text = "Compare Schema and Replication Metadata across  Federation nodes with the current" +

    "ly connected node";

            // 

            // label2

            // 

            this.label2.AutoSize = true;

            this.label2.Location = new System.Drawing.Point(54, 82);

            this.label2.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);

            this.label2.Name = "label2";

            this.label2.Size = new System.Drawing.Size(90, 13);

            this.label2.TabIndex = 1;

            this.label2.Text = "Select an option :";

            // 

            // SchemaButton

            // 

            this.SchemaButton.AutoSize = true;

            this.SchemaButton.Location = new System.Drawing.Point(18, 17);

            this.SchemaButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);

            this.SchemaButton.Name = "SchemaButton";

            this.SchemaButton.Size = new System.Drawing.Size(64, 17);

            this.SchemaButton.TabIndex = 2;

            this.SchemaButton.TabStop = true;

            this.SchemaButton.Text = "Schema";

            this.SchemaButton.UseVisualStyleBackColor = true;

            // 

            // MetaDataButton

            // 

            this.MetaDataButton.AutoSize = true;

            this.MetaDataButton.Location = new System.Drawing.Point(18, 42);

            this.MetaDataButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);

            this.MetaDataButton.Name = "MetaDataButton";

            this.MetaDataButton.Size = new System.Drawing.Size(72, 17);

            this.MetaDataButton.TabIndex = 3;

            this.MetaDataButton.TabStop = true;

            this.MetaDataButton.Text = "MetaData";

            this.MetaDataButton.UseVisualStyleBackColor = true;

            // 

            // NodesList

            // 

            this.NodesList.FormattingEnabled = true;

            this.NodesList.Location = new System.Drawing.Point(56, 175);

            this.NodesList.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);

            this.NodesList.Name = "NodesList";

            this.NodesList.Size = new System.Drawing.Size(176, 108);

            this.NodesList.TabIndex = 4;

            // 

            // label3

            // 

            this.label3.AutoSize = true;

            this.label3.Location = new System.Drawing.Point(53, 148);

            this.label3.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);

            this.label3.Name = "label3";

            this.label3.Size = new System.Drawing.Size(324, 13);

            this.label3.TabIndex = 5;

            this.label3.Text = "Following nodes are out of sync from the currently connected node ";

            // 

            // CompareButton

            // 

            this.CompareButton.Location = new System.Drawing.Point(285, 76);

            this.CompareButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);

            this.CompareButton.Name = "CompareButton";

            this.CompareButton.Size = new System.Drawing.Size(76, 19);

            this.CompareButton.TabIndex = 6;

            this.CompareButton.Text = "Compare";

            this.CompareButton.UseVisualStyleBackColor = true;

            this.CompareButton.Click += new System.EventHandler(this.CompareButton_Click);

            // 

            // ViewAttributeTypeDiffButton

            // 

            this.ViewAttributeTypeDiffButton.Location = new System.Drawing.Point(56, 303);

            this.ViewAttributeTypeDiffButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);

            this.ViewAttributeTypeDiffButton.Name = "ViewAttributeTypeDiffButton";

            this.ViewAttributeTypeDiffButton.Size = new System.Drawing.Size(176, 19);

            this.ViewAttributeTypeDiffButton.TabIndex = 7;

            this.ViewAttributeTypeDiffButton.Text = "View AttributeType Diff";

            this.ViewAttributeTypeDiffButton.UseVisualStyleBackColor = true;

            this.ViewAttributeTypeDiffButton.Click += new System.EventHandler(this.ViewDiffButtonClicked);

            // 

            // ViewObjectClassDiffButton

            // 

            this.ViewObjectClassDiffButton.Location = new System.Drawing.Point(244, 303);

            this.ViewObjectClassDiffButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);

            this.ViewObjectClassDiffButton.Name = "ViewObjectClassDiffButton";

            this.ViewObjectClassDiffButton.Size = new System.Drawing.Size(152, 19);

            this.ViewObjectClassDiffButton.TabIndex = 8;

            this.ViewObjectClassDiffButton.Text = "View ObjectClass Diff";

            this.ViewObjectClassDiffButton.UseVisualStyleBackColor = true;

            this.ViewObjectClassDiffButton.Click += new System.EventHandler(this.ViewDiffButtonClicked);

            // 

            // CompareOptionsGroup

            // 

            this.CompareOptionsGroup.Controls.Add(this.SchemaButton);

            this.CompareOptionsGroup.Controls.Add(this.MetaDataButton);

            this.CompareOptionsGroup.Location = new System.Drawing.Point(149, 65);

            this.CompareOptionsGroup.Name = "CompareOptionsGroup";

            this.CompareOptionsGroup.Size = new System.Drawing.Size(131, 68);

            this.CompareOptionsGroup.TabIndex = 9;

            this.CompareOptionsGroup.TabStop = false;

            // 

            // SchemaMetadataComparisionWindow

            // 

            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);

            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;

            this.ClientSize = new System.Drawing.Size(548, 404);

            this.Controls.Add(this.CompareOptionsGroup);

            this.Controls.Add(this.ViewObjectClassDiffButton);

            this.Controls.Add(this.ViewAttributeTypeDiffButton);

            this.Controls.Add(this.CompareButton);

            this.Controls.Add(this.label3);

            this.Controls.Add(this.NodesList);

            this.Controls.Add(this.label2);

            this.Controls.Add(this.label1);

            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);

            this.MaximizeBox = false;

            this.Name = "SchemaMetadataComparisionWindow";

            this.Text = "SchemaMetadataComparisionWindow";

            this.CompareOptionsGroup.ResumeLayout(false);

            this.CompareOptionsGroup.PerformLayout();

            this.ResumeLayout(false);

            this.PerformLayout();



        }



        #endregion



        private System.Windows.Forms.Label label1;

        private System.Windows.Forms.Label label2;

        private System.Windows.Forms.RadioButton SchemaButton;

        private System.Windows.Forms.RadioButton MetaDataButton;

        private System.Windows.Forms.ListBox NodesList;

        private System.Windows.Forms.Label label3;

        private System.Windows.Forms.Button CompareButton;

        private System.Windows.Forms.Button ViewAttributeTypeDiffButton;

        private System.Windows.Forms.Button ViewObjectClassDiffButton;

        private System.Windows.Forms.GroupBox CompareOptionsGroup;

    }

}