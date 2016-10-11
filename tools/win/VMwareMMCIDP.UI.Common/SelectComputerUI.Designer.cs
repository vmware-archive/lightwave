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
namespace VMwareMMCIDP.UI.Common
{
    partial class SelectComputerUI
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SelectComputerUI));
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.groupBoxSelect = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanelComputers = new System.Windows.Forms.TableLayoutPanel();
            this.radioButtonLocal = new System.Windows.Forms.RadioButton();
            this.radioButtonRemote = new System.Windows.Forms.RadioButton();
            this.textBoxRemote = new System.Windows.Forms.TextBox();
            this.horizontalLine = new System.Windows.Forms.GroupBox();
            this.labelSelect = new System.Windows.Forms.Label();
            this.tableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            this.tableLayoutPanel1.SuspendLayout();
            this.groupBoxSelect.SuspendLayout();
            this.tableLayoutPanelComputers.SuspendLayout();
            this.tableLayoutPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // tableLayoutPanel1
            // 
            resources.ApplyResources(this.tableLayoutPanel1, "tableLayoutPanel1");
            this.tableLayoutPanel1.Controls.Add(this.buttonOK, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.buttonCancel, 1, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            // 
            // buttonOK
            // 
            resources.ApplyResources(this.buttonOK, "buttonOK");
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            resources.ApplyResources(this.buttonCancel, "buttonCancel");
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // groupBoxSelect
            // 
            resources.ApplyResources(this.groupBoxSelect, "groupBoxSelect");
            this.groupBoxSelect.Controls.Add(this.tableLayoutPanelComputers);
            this.groupBoxSelect.Name = "groupBoxSelect";
            this.groupBoxSelect.TabStop = false;
            // 
            // tableLayoutPanelComputers
            // 
            resources.ApplyResources(this.tableLayoutPanelComputers, "tableLayoutPanelComputers");
            this.tableLayoutPanelComputers.Controls.Add(this.radioButtonLocal, 0, 0);
            this.tableLayoutPanelComputers.Controls.Add(this.radioButtonRemote, 0, 1);
            this.tableLayoutPanelComputers.Controls.Add(this.textBoxRemote, 1, 1);
            this.tableLayoutPanelComputers.Name = "tableLayoutPanelComputers";
            // 
            // radioButtonLocal
            // 
            resources.ApplyResources(this.radioButtonLocal, "radioButtonLocal");
            this.tableLayoutPanelComputers.SetColumnSpan(this.radioButtonLocal, 2);
            this.radioButtonLocal.Name = "radioButtonLocal";
            this.radioButtonLocal.UseVisualStyleBackColor = true;
            // 
            // radioButtonRemote
            // 
            resources.ApplyResources(this.radioButtonRemote, "radioButtonRemote");
            this.radioButtonRemote.Checked = true;
            this.radioButtonRemote.Name = "radioButtonRemote";
            this.radioButtonRemote.TabStop = true;
            this.radioButtonRemote.UseVisualStyleBackColor = true;
            this.radioButtonRemote.CheckedChanged += new System.EventHandler(this.radioButtonRemote_CheckedChanged);
            // 
            // textBoxRemote
            // 
            resources.ApplyResources(this.textBoxRemote, "textBoxRemote");
            this.textBoxRemote.Name = "textBoxRemote";
            // 
            // horizontalLine
            // 
            resources.ApplyResources(this.horizontalLine, "horizontalLine");
            this.horizontalLine.Name = "horizontalLine";
            this.horizontalLine.TabStop = false;
            // 
            // labelSelect
            // 
            resources.ApplyResources(this.labelSelect, "labelSelect");
            this.labelSelect.Name = "labelSelect";
            // 
            // tableLayoutPanel
            // 
            resources.ApplyResources(this.tableLayoutPanel, "tableLayoutPanel");
            this.tableLayoutPanel.Controls.Add(this.labelSelect, 0, 0);
            this.tableLayoutPanel.Controls.Add(this.horizontalLine, 0, 2);
            this.tableLayoutPanel.Controls.Add(this.groupBoxSelect, 0, 1);
            this.tableLayoutPanel.Controls.Add(this.tableLayoutPanel1, 0, 3);
            this.tableLayoutPanel.Name = "tableLayoutPanel";
            // 
            // SelectComputerUI
            // 
            this.AcceptButton = this.buttonOK;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.Controls.Add(this.tableLayoutPanel);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SelectComputerUI";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.InitializeSnapin_FormClosing);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.groupBoxSelect.ResumeLayout(false);
            this.tableLayoutPanelComputers.ResumeLayout(false);
            this.tableLayoutPanelComputers.PerformLayout();
            this.tableLayoutPanel.ResumeLayout(false);
            this.tableLayoutPanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.GroupBox groupBoxSelect;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelComputers;
        private System.Windows.Forms.RadioButton radioButtonLocal;
        private System.Windows.Forms.RadioButton radioButtonRemote;
        private System.Windows.Forms.TextBox textBoxRemote;
        private System.Windows.Forms.GroupBox horizontalLine;
        private System.Windows.Forms.Label labelSelect;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel;

    }
}