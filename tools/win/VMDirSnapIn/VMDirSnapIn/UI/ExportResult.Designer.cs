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
    partial class ExportResult
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
            this.comboBoxFileFormat = new System.Windows.Forms.ComboBox();
            this.comboBoxScope = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.buttonExport = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.comboBoxAttrToReturn = new System.Windows.Forms.ComboBox();
            this.buttonAttrRemove = new System.Windows.Forms.Button();
            this.listViewAttrToExport = new System.Windows.Forms.ListView();
            this.Attribute = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.buttonAttrRemoveAll = new System.Windows.Forms.Button();
            this.buttonAttrAdd = new System.Windows.Forms.Button();
            this.checkBoxAttToExport = new System.Windows.Forms.CheckBox();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // comboBoxFileFormat
            // 
            this.comboBoxFileFormat.FormattingEnabled = true;
            this.comboBoxFileFormat.Location = new System.Drawing.Point(85, 34);
            this.comboBoxFileFormat.Name = "comboBoxFileFormat";
            this.comboBoxFileFormat.Size = new System.Drawing.Size(200, 21);
            this.comboBoxFileFormat.TabIndex = 0;
            // 
            // comboBoxScope
            // 
            this.comboBoxScope.FormattingEnabled = true;
            this.comboBoxScope.Location = new System.Drawing.Point(85, 74);
            this.comboBoxScope.Name = "comboBoxScope";
            this.comboBoxScope.Size = new System.Drawing.Size(200, 21);
            this.comboBoxScope.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(25, 42);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(42, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Format:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(25, 82);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(41, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Scope:";
            // 
            // buttonExport
            // 
            this.buttonExport.Location = new System.Drawing.Point(228, 340);
            this.buttonExport.Name = "buttonExport";
            this.buttonExport.Size = new System.Drawing.Size(75, 23);
            this.buttonExport.TabIndex = 4;
            this.buttonExport.Text = "Export";
            this.buttonExport.UseVisualStyleBackColor = true;
            this.buttonExport.Click += new System.EventHandler(this.buttonExport_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(147, 340);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 5;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.checkBoxAttToExport);
            this.groupBox2.Controls.Add(this.comboBoxAttrToReturn);
            this.groupBox2.Controls.Add(this.buttonAttrRemove);
            this.groupBox2.Controls.Add(this.listViewAttrToExport);
            this.groupBox2.Controls.Add(this.buttonAttrRemoveAll);
            this.groupBox2.Controls.Add(this.buttonAttrAdd);
            this.groupBox2.Location = new System.Drawing.Point(28, 116);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(390, 203);
            this.groupBox2.TabIndex = 27;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Attributes To Export:";
            // 
            // comboBoxAttrToReturn
            // 
            this.comboBoxAttrToReturn.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxAttrToReturn.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxAttrToReturn.FormattingEnabled = true;
            this.comboBoxAttrToReturn.Location = new System.Drawing.Point(24, 70);
            this.comboBoxAttrToReturn.Name = "comboBoxAttrToReturn";
            this.comboBoxAttrToReturn.Size = new System.Drawing.Size(251, 21);
            this.comboBoxAttrToReturn.TabIndex = 9;
            // 
            // buttonAttrRemove
            // 
            this.buttonAttrRemove.Location = new System.Drawing.Point(295, 98);
            this.buttonAttrRemove.Name = "buttonAttrRemove";
            this.buttonAttrRemove.Size = new System.Drawing.Size(75, 23);
            this.buttonAttrRemove.TabIndex = 12;
            this.buttonAttrRemove.Text = "Remove";
            this.buttonAttrRemove.UseVisualStyleBackColor = true;
            this.buttonAttrRemove.Click += new System.EventHandler(this.buttonAttrRemove_Click);
            // 
            // listViewAttrToExport
            // 
            this.listViewAttrToExport.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.Attribute});
            this.listViewAttrToExport.GridLines = true;
            this.listViewAttrToExport.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.listViewAttrToExport.Location = new System.Drawing.Point(24, 99);
            this.listViewAttrToExport.Name = "listViewAttrToExport";
            this.listViewAttrToExport.Size = new System.Drawing.Size(251, 88);
            this.listViewAttrToExport.TabIndex = 11;
            this.listViewAttrToExport.UseCompatibleStateImageBehavior = false;
            this.listViewAttrToExport.View = System.Windows.Forms.View.Details;
            // 
            // Attribute
            // 
            this.Attribute.Width = 185;
            // 
            // buttonAttrRemoveAll
            // 
            this.buttonAttrRemoveAll.Location = new System.Drawing.Point(295, 128);
            this.buttonAttrRemoveAll.Name = "buttonAttrRemoveAll";
            this.buttonAttrRemoveAll.Size = new System.Drawing.Size(75, 23);
            this.buttonAttrRemoveAll.TabIndex = 13;
            this.buttonAttrRemoveAll.Text = "Remove All";
            this.buttonAttrRemoveAll.UseVisualStyleBackColor = true;
            this.buttonAttrRemoveAll.Click += new System.EventHandler(this.buttonAttrRemoveAll_Click);
            // 
            // buttonAttrAdd
            // 
            this.buttonAttrAdd.Location = new System.Drawing.Point(295, 68);
            this.buttonAttrAdd.Name = "buttonAttrAdd";
            this.buttonAttrAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonAttrAdd.TabIndex = 10;
            this.buttonAttrAdd.Text = "Add";
            this.buttonAttrAdd.UseVisualStyleBackColor = true;
            this.buttonAttrAdd.Click += new System.EventHandler(this.buttonAttrAdd_Click);
            // 
            // checkBoxAttToExport
            // 
            this.checkBoxAttToExport.AutoSize = true;
            this.checkBoxAttToExport.Location = new System.Drawing.Point(24, 38);
            this.checkBoxAttToExport.Name = "checkBoxAttToExport";
            this.checkBoxAttToExport.Size = new System.Drawing.Size(131, 17);
            this.checkBoxAttToExport.TabIndex = 14;
            this.checkBoxAttToExport.Text = "All Returned Attributes";
            this.checkBoxAttToExport.UseVisualStyleBackColor = true;
            this.checkBoxAttToExport.CheckedChanged += new System.EventHandler(this.checkBoxAttToExport_CheckedChanged);
            // 
            // ExportResult
            // 
            this.AcceptButton = this.buttonExport;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(460, 384);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonExport);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.comboBoxScope);
            this.Controls.Add(this.comboBoxFileFormat);
            this.Name = "ExportResult";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Export Result";
            this.Icon = VMDirEnvironment.Instance.GetIconResource(VMDirIconIndex.Export);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBoxFileFormat;
        private System.Windows.Forms.ComboBox comboBoxScope;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button buttonExport;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.ComboBox comboBoxAttrToReturn;
        private System.Windows.Forms.Button buttonAttrRemove;
        private System.Windows.Forms.ListView listViewAttrToExport;
        private System.Windows.Forms.ColumnHeader Attribute;
        private System.Windows.Forms.Button buttonAttrRemoveAll;
        private System.Windows.Forms.Button buttonAttrAdd;
        private System.Windows.Forms.CheckBox checkBoxAttToExport;
    }
}