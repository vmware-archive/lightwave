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
    partial class PropertiesControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.buttonSubmit = new System.Windows.Forms.Button();
            this.buttonReset = new System.Windows.Forms.Button();
            this.listViewProp = new System.Windows.Forms.ListView();
            this.contextMenuStripProp = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.addAnotherToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textBoxEdit = new System.Windows.Forms.MaskedTextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.contextMenuStripProp.SuspendLayout();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonSubmit
            // 
            this.buttonSubmit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.buttonSubmit.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.buttonSubmit.Location = new System.Drawing.Point(167, 4);
            this.buttonSubmit.Name = "buttonSubmit";
            this.buttonSubmit.Size = new System.Drawing.Size(75, 23);
            this.buttonSubmit.TabIndex = 1;
            this.buttonSubmit.Text = "Submit";
            this.buttonSubmit.UseVisualStyleBackColor = true;
            this.buttonSubmit.Click += new System.EventHandler(this.buttonSubmit_Click);
            // 
            // buttonReset
            // 
            this.buttonReset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.buttonReset.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.buttonReset.Location = new System.Drawing.Point(257, 4);
            this.buttonReset.Name = "buttonReset";
            this.buttonReset.Size = new System.Drawing.Size(75, 23);
            this.buttonReset.TabIndex = 2;
            this.buttonReset.Text = "Reset";
            this.buttonReset.UseVisualStyleBackColor = true;
            this.buttonReset.Click += new System.EventHandler(this.buttonReset_Click);
            // 
            // listViewProp
            // 
            this.listViewProp.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewProp.ContextMenuStrip = this.contextMenuStripProp;
            this.listViewProp.FullRowSelect = true;
            this.listViewProp.GridLines = true;
            this.listViewProp.HideSelection = false;
            this.listViewProp.Location = new System.Drawing.Point(0, 0);
            this.listViewProp.Name = "listViewProp";
            this.listViewProp.Size = new System.Drawing.Size(480, 446);
            this.listViewProp.TabIndex = 4;
            this.listViewProp.UseCompatibleStateImageBehavior = false;
            this.listViewProp.View = System.Windows.Forms.View.Details;
            this.listViewProp.DoubleClick += new System.EventHandler(this.listViewProp_DoubleClick);
            // 
            // contextMenuStripProp
            // 
            this.contextMenuStripProp.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addAnotherToolStripMenuItem});
            this.contextMenuStripProp.Name = "contextMenuStripProp";
            this.contextMenuStripProp.Size = new System.Drawing.Size(143, 26);
            this.contextMenuStripProp.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStripProp_Opening);
            // 
            // addAnotherToolStripMenuItem
            // 
            this.addAnotherToolStripMenuItem.Name = "addAnotherToolStripMenuItem";
            this.addAnotherToolStripMenuItem.Size = new System.Drawing.Size(142, 22);
            this.addAnotherToolStripMenuItem.Text = "Add Another";
            this.addAnotherToolStripMenuItem.Click += new System.EventHandler(this.addAnotherToolStripMenuItem_Click);
            // 
            // textBoxEdit
            // 
            this.textBoxEdit.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.textBoxEdit.Location = new System.Drawing.Point(23, 426);
            this.textBoxEdit.Name = "textBoxEdit";
            this.textBoxEdit.Size = new System.Drawing.Size(36, 20);
            this.textBoxEdit.TabIndex = 5;
            this.textBoxEdit.Visible = false;
            this.textBoxEdit.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textBoxEdit_KeyPress);
            this.textBoxEdit.LostFocus += new System.EventHandler(this.textBoxEdit_LostFocus);
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel1.Controls.Add(this.buttonReset);
            this.panel1.Controls.Add(this.buttonSubmit);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 452);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(480, 35);
            this.panel1.TabIndex = 6;
            // 
            // PropertiesControl
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Inherit;
            this.AutoScroll = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.textBoxEdit);
            this.Controls.Add(this.listViewProp);
            this.DoubleBuffered = true;
            this.Name = "PropertiesControl";
            this.Size = new System.Drawing.Size(480, 487);
            this.contextMenuStripProp.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonSubmit;
        private System.Windows.Forms.Button buttonReset;
        private System.Windows.Forms.ListView listViewProp;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripProp;
        private System.Windows.Forms.ToolStripMenuItem addAnotherToolStripMenuItem;
        private System.Windows.Forms.MaskedTextBox textBoxEdit;
        private System.Windows.Forms.Panel panel1;
    }
}
