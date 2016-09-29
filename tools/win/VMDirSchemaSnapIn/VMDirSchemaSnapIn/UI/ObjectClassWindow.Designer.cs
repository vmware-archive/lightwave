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
    partial class ObjectClassWindow
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
        {            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.ObjectClassNameText = new System.Windows.Forms.TextBox();
            this.DescriptionText = new System.Windows.Forms.TextBox();
            this.ObjectClassIdentifierText = new System.Windows.Forms.TextBox();
            this.ParentClassText = new System.Windows.Forms.TextBox();
            this.ClassTypeCombo = new System.Windows.Forms.ComboBox();
            this.AddParentClassButton = new System.Windows.Forms.Button();
            this.MandatoryList = new System.Windows.Forms.ListBox();
            this.OptionalList = new System.Windows.Forms.ListBox();
            this.AuxiliaryList = new System.Windows.Forms.ListBox();
            this.AddButton = new System.Windows.Forms.Button();
            this.CloseButton = new System.Windows.Forms.Button();
            this.AddMandatoryAttributeButton = new System.Windows.Forms.Button();
            this.RemoveMandatoryAttributeButton = new System.Windows.Forms.Button();
            this.RemoveOptionalAttributeButton = new System.Windows.Forms.Button();
            this.AddOptionalAttributeButton = new System.Windows.Forms.Button();
            this.AddAuxiliaryAttributeButton = new System.Windows.Forms.Button();
            this.RemoveAuxiliaryAttribtueButton = new System.Windows.Forms.Button();
            this.GovernsIDTextField = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(68, 9);
            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(41, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Name :";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(43, 39);
            this.label2.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(66, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Description :";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(56, 70);
            this.label3.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(54, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "X00.OID :";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(44, 110);
            this.label4.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(65, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Class Type :";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(34, 137);
            this.label5.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(75, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "Parent Class : ";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(42, 210);
            this.label6.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(113, 13);
            this.label6.TabIndex = 5;
            this.label6.Text = "Mandatory Attributes : ";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(197, 210);
            this.label7.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(99, 13);
            this.label7.TabIndex = 6;
            this.label7.Text = "Optional Attributes :";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(347, 210);
            this.label8.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(93, 13);
            this.label8.TabIndex = 7;
            this.label8.Text = "Auxiliary Classes : ";
            // 
            // ObjectClassNameText
            // 
            this.ObjectClassNameText.Location = new System.Drawing.Point(152, 9);
            this.ObjectClassNameText.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.ObjectClassNameText.Name = "ObjectClassNameText";
            this.ObjectClassNameText.Size = new System.Drawing.Size(290, 20);
            this.ObjectClassNameText.TabIndex = 8;
            // 
            // DescriptionText
            // 
            this.DescriptionText.Location = new System.Drawing.Point(152, 37);
            this.DescriptionText.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.DescriptionText.Name = "DescriptionText";
            this.DescriptionText.Size = new System.Drawing.Size(290, 20);
            this.DescriptionText.TabIndex = 9;
            // 
            // ObjectClassIdentifierText
            // 
            this.ObjectClassIdentifierText.Location = new System.Drawing.Point(152, 70);
            this.ObjectClassIdentifierText.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.ObjectClassIdentifierText.Name = "ObjectClassIdentifierText";
            this.ObjectClassIdentifierText.Size = new System.Drawing.Size(290, 20);
            this.ObjectClassIdentifierText.TabIndex = 10;
            // 
            // ParentClassText
            // 
            this.ParentClassText.Location = new System.Drawing.Point(152, 137);
            this.ParentClassText.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.ParentClassText.Name = "ParentClassText";
            this.ParentClassText.ReadOnly = true;
            this.ParentClassText.Size = new System.Drawing.Size(250, 20);
            this.ParentClassText.TabIndex = 11;
            // 
            // ClassTypeCombo
            // 
            this.ClassTypeCombo.FormattingEnabled = true;
            this.ClassTypeCombo.Items.AddRange(new object[] {
            "Structural",
            "Abstract",
            "Auxiliary"});
            this.ClassTypeCombo.Location = new System.Drawing.Point(152, 105);
            this.ClassTypeCombo.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.ClassTypeCombo.Name = "ClassTypeCombo";
            this.ClassTypeCombo.Size = new System.Drawing.Size(290, 21);
            this.ClassTypeCombo.TabIndex = 12;
            this.ClassTypeCombo.SelectedValueChanged += new System.EventHandler(this.OnSelectionChanged);
            // 
            // AddParentClassButton
            // 
            this.AddParentClassButton.Location = new System.Drawing.Point(417, 137);
            this.AddParentClassButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.AddParentClassButton.Name = "AddParentClassButton";
            this.AddParentClassButton.Size = new System.Drawing.Size(24, 19);
            this.AddParentClassButton.TabIndex = 13;
            this.AddParentClassButton.Text = "+";
            this.AddParentClassButton.UseVisualStyleBackColor = true;
            this.AddParentClassButton.Click += new System.EventHandler(this.AddParentClassButton_Click);
            // 
            // MandatoryList
            // 
            this.MandatoryList.FormattingEnabled = true;
            this.MandatoryList.Location = new System.Drawing.Point(36, 235);
            this.MandatoryList.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.MandatoryList.Name = "MandatoryList";
            this.MandatoryList.Size = new System.Drawing.Size(123, 95);
            this.MandatoryList.TabIndex = 14;
            // 
            // OptionalList
            // 
            this.OptionalList.FormattingEnabled = true;
            this.OptionalList.Location = new System.Drawing.Point(191, 235);
            this.OptionalList.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.OptionalList.Name = "OptionalList";
            this.OptionalList.Size = new System.Drawing.Size(127, 95);
            this.OptionalList.TabIndex = 15;
            // 
            // AuxiliaryList
            // 
            this.AuxiliaryList.FormattingEnabled = true;
            this.AuxiliaryList.Location = new System.Drawing.Point(350, 235);
            this.AuxiliaryList.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.AuxiliaryList.Name = "AuxiliaryList";
            this.AuxiliaryList.Size = new System.Drawing.Size(129, 95);
            this.AuxiliaryList.TabIndex = 16;
            // 
            // AddButton
            // 
            this.AddButton.Location = new System.Drawing.Point(338, 347);
            this.AddButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.AddButton.Name = "AddButton";
            this.AddButton.Size = new System.Drawing.Size(56, 19);
            this.AddButton.TabIndex = 17;
            this.AddButton.Text = "Add";
            this.AddButton.UseVisualStyleBackColor = true;
            this.AddButton.Click += new System.EventHandler(this.AddButton_Click);
            // 
            // CloseButton
            // 
            this.CloseButton.Location = new System.Drawing.Point(412, 347);
            this.CloseButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.Size = new System.Drawing.Size(56, 19);
            this.CloseButton.TabIndex = 18;
            this.CloseButton.Text = "Close";
            this.CloseButton.UseVisualStyleBackColor = true;
            this.CloseButton.Click += new System.EventHandler(this.CloseButton_Click);
            // 
            // AddMandatoryAttributeButton
            // 
            this.AddMandatoryAttributeButton.Location = new System.Drawing.Point(163, 235);
            this.AddMandatoryAttributeButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.AddMandatoryAttributeButton.Name = "AddMandatoryAttributeButton";
            this.AddMandatoryAttributeButton.Size = new System.Drawing.Size(14, 19);
            this.AddMandatoryAttributeButton.TabIndex = 19;
            this.AddMandatoryAttributeButton.Text = "+";
            this.AddMandatoryAttributeButton.UseVisualStyleBackColor = true;
            this.AddMandatoryAttributeButton.Click += new System.EventHandler(this.AddMandatoryAttributeButton_Click);
            // 
            // RemoveMandatoryAttributeButton
            // 
            this.RemoveMandatoryAttributeButton.Location = new System.Drawing.Point(163, 258);
            this.RemoveMandatoryAttributeButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.RemoveMandatoryAttributeButton.Name = "RemoveMandatoryAttributeButton";
            this.RemoveMandatoryAttributeButton.Size = new System.Drawing.Size(14, 19);
            this.RemoveMandatoryAttributeButton.TabIndex = 20;
            this.RemoveMandatoryAttributeButton.Text = "-";
            this.RemoveMandatoryAttributeButton.UseVisualStyleBackColor = true;
            this.RemoveMandatoryAttributeButton.Click += new System.EventHandler(this.RemoveMandatoryAttributeButton_Click);
            // 
            // RemoveOptionalAttributeButton
            // 
            this.RemoveOptionalAttributeButton.Location = new System.Drawing.Point(322, 258);
            this.RemoveOptionalAttributeButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.RemoveOptionalAttributeButton.Name = "RemoveOptionalAttributeButton";
            this.RemoveOptionalAttributeButton.Size = new System.Drawing.Size(15, 19);
            this.RemoveOptionalAttributeButton.TabIndex = 21;
            this.RemoveOptionalAttributeButton.Text = "-";
            this.RemoveOptionalAttributeButton.UseVisualStyleBackColor = true;
            this.RemoveOptionalAttributeButton.Click += new System.EventHandler(this.RemoveOptionalAttributeButton_Click);
            // 
            // AddOptionalAttributeButton
            // 
            this.AddOptionalAttributeButton.Location = new System.Drawing.Point(322, 235);
            this.AddOptionalAttributeButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.AddOptionalAttributeButton.Name = "AddOptionalAttributeButton";
            this.AddOptionalAttributeButton.Size = new System.Drawing.Size(15, 19);
            this.AddOptionalAttributeButton.TabIndex = 22;
            this.AddOptionalAttributeButton.Text = "+";
            this.AddOptionalAttributeButton.UseVisualStyleBackColor = true;
            this.AddOptionalAttributeButton.Click += new System.EventHandler(this.AddOptionalAttributeButton_Click);
            // 
            // AddAuxiliaryAttributeButton
            // 
            this.AddAuxiliaryAttributeButton.Location = new System.Drawing.Point(483, 235);
            this.AddAuxiliaryAttributeButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.AddAuxiliaryAttributeButton.Name = "AddAuxiliaryAttributeButton";
            this.AddAuxiliaryAttributeButton.Size = new System.Drawing.Size(13, 19);
            this.AddAuxiliaryAttributeButton.TabIndex = 23;
            this.AddAuxiliaryAttributeButton.Text = "+";
            this.AddAuxiliaryAttributeButton.UseVisualStyleBackColor = true;
            this.AddAuxiliaryAttributeButton.Click += new System.EventHandler(this.AddAuxiliaryAttributeButton_Click);
            // 
            // RemoveAuxiliaryAttribtueButton
            // 
            this.RemoveAuxiliaryAttribtueButton.Location = new System.Drawing.Point(483, 258);
            this.RemoveAuxiliaryAttribtueButton.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.RemoveAuxiliaryAttribtueButton.Name = "RemoveAuxiliaryAttribtueButton";
            this.RemoveAuxiliaryAttribtueButton.Size = new System.Drawing.Size(13, 19);
            this.RemoveAuxiliaryAttribtueButton.TabIndex = 24;
            this.RemoveAuxiliaryAttribtueButton.Text = "-";
            this.RemoveAuxiliaryAttribtueButton.UseVisualStyleBackColor = true;
            this.RemoveAuxiliaryAttribtueButton.Click += new System.EventHandler(this.RemoveAuxiliaryAttribtueButton_Click);
            // 
            // GovernsIDTextField
            // 
            this.GovernsIDTextField.Location = new System.Drawing.Point(152, 172);
            this.GovernsIDTextField.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.GovernsIDTextField.Name = "GovernsIDTextField";
            this.GovernsIDTextField.Size = new System.Drawing.Size(290, 20);
            this.GovernsIDTextField.TabIndex = 25;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(34, 172);
            this.label9.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(70, 13);
            this.label9.TabIndex = 26;
            this.label9.Text = "Governs ID : ";
            // 
            // ObjectClassWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(507, 392);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.GovernsIDTextField);
            this.Controls.Add(this.RemoveAuxiliaryAttribtueButton);
            this.Controls.Add(this.AddAuxiliaryAttributeButton);
            this.Controls.Add(this.AddOptionalAttributeButton);
            this.Controls.Add(this.RemoveOptionalAttributeButton);
            this.Controls.Add(this.RemoveMandatoryAttributeButton);
            this.Controls.Add(this.AddMandatoryAttributeButton);
            this.Controls.Add(this.CloseButton);
            this.Controls.Add(this.AddButton);
            this.Controls.Add(this.AuxiliaryList);
            this.Controls.Add(this.OptionalList);
            this.Controls.Add(this.MandatoryList);
            this.Controls.Add(this.AddParentClassButton);
            this.Controls.Add(this.ClassTypeCombo);
            this.Controls.Add(this.ParentClassText);
            this.Controls.Add(this.ObjectClassIdentifierText);
            this.Controls.Add(this.DescriptionText);
            this.Controls.Add(this.ObjectClassNameText);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.MaximizeBox = false;
            this.Name = "ObjectClassWindow";
            this.Text = "ObjectClassWindow";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox ObjectClassNameText;
        private System.Windows.Forms.TextBox DescriptionText;
        private System.Windows.Forms.TextBox ObjectClassIdentifierText;
        private System.Windows.Forms.TextBox ParentClassText;
        private System.Windows.Forms.ComboBox ClassTypeCombo;
        private System.Windows.Forms.Button AddParentClassButton;
        private System.Windows.Forms.ListBox MandatoryList;
        private System.Windows.Forms.ListBox OptionalList;
        private System.Windows.Forms.ListBox AuxiliaryList;
        private System.Windows.Forms.Button AddButton;
        private System.Windows.Forms.Button CloseButton;
        private System.Windows.Forms.Button AddMandatoryAttributeButton;
        private System.Windows.Forms.Button RemoveMandatoryAttributeButton;
        private System.Windows.Forms.Button RemoveOptionalAttributeButton;
        private System.Windows.Forms.Button AddOptionalAttributeButton;
        private System.Windows.Forms.Button AddAuxiliaryAttributeButton;
        private System.Windows.Forms.Button RemoveAuxiliaryAttribtueButton;
        private System.Windows.Forms.TextBox GovernsIDTextField;
        private System.Windows.Forms.Label label9;
    }
}