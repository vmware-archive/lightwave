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
    partial class ComputersView
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
            this.lstComputers = new System.Windows.Forms.ListView();
            this.clmHostName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.clmController = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // lstComputers
            // 
            this.lstComputers.AllowColumnReorder = true;
            this.lstComputers.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.clmHostName,
            this.clmController});
            this.lstComputers.FullRowSelect = true;
            this.lstComputers.GridLines = true;
            this.lstComputers.Location = new System.Drawing.Point(4, 3);
            this.lstComputers.MultiSelect = false;
            this.lstComputers.Name = "lstComputers";
            this.lstComputers.ShowItemToolTips = true;
            this.lstComputers.Size = new System.Drawing.Size(377, 211);
            this.lstComputers.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lstComputers.TabIndex = 38;
            this.lstComputers.UseCompatibleStateImageBehavior = false;
            this.lstComputers.View = System.Windows.Forms.View.Details;
            // 
            // clmHostName
            // 
            this.clmHostName.Text = "Hostname";
            // 
            // clmController
            // 
            this.clmController.Text = "Domain Controller";
            this.clmController.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.clmController.Width = 98;
            // 
            // ComputersView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(383, 218);
            this.Controls.Add(this.lstComputers);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.HelpButton = true;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ComputersView";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Computers";
            this.Load += new System.EventHandler(this.ComputersView_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lstComputers;
        private System.Windows.Forms.ColumnHeader clmHostName;
        private System.Windows.Forms.ColumnHeader clmController;
    }
}