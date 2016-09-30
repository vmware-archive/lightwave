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
namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    partial class ExternalDomainGeneralProperty
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.lblDomainName = new System.Windows.Forms.Label();
            this.pbIcon = new System.Windows.Forms.PictureBox();
            this.txtDomainAlias = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtDomainName = new System.Windows.Forms.TextBox();
            this.txtGroupBaseDN = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.txtUserBaseDN = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtPrimaryURL = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.label18 = new System.Windows.Forms.Label();
            this.cbDNForNestedGroups = new System.Windows.Forms.CheckBox();
            this.cbGroupSearch = new System.Windows.Forms.CheckBox();
            this.cbMatchRuleInChain = new System.Windows.Forms.CheckBox();
            this.cbSiteAffinity = new System.Windows.Forms.CheckBox();
            this.txtSecondaryUrl = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pbIcon)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Location = new System.Drawing.Point(13, 47);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(324, 3);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            // 
            // lblDomainName
            // 
            this.lblDomainName.AutoSize = true;
            this.lblDomainName.Location = new System.Drawing.Point(71, 17);
            this.lblDomainName.Name = "lblDomainName";
            this.lblDomainName.Size = new System.Drawing.Size(0, 13);
            this.lblDomainName.TabIndex = 0;
            // 
            // pbIcon
            // 
            this.pbIcon.InitialImage = null;
            this.pbIcon.Location = new System.Drawing.Point(16, 9);
            this.pbIcon.Name = "pbIcon";
            this.pbIcon.Size = new System.Drawing.Size(32, 32);
            this.pbIcon.TabIndex = 9;
            this.pbIcon.TabStop = false;
            // 
            // txtDomainAlias
            // 
            this.txtDomainAlias.Location = new System.Drawing.Point(93, 85);
            this.txtDomainAlias.Name = "txtDomainAlias";
            this.txtDomainAlias.Size = new System.Drawing.Size(244, 20);
            this.txtDomainAlias.TabIndex = 7;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(14, 88);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(70, 13);
            this.label6.TabIndex = 6;
            this.label6.Text = "Domain alias:";
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Location = new System.Drawing.Point(13, 112);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(324, 3);
            this.groupBox2.TabIndex = 8;
            this.groupBox2.TabStop = false;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(14, 62);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(75, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "Domain name:";
            // 
            // txtDomainName
            // 
            this.txtDomainName.Location = new System.Drawing.Point(93, 59);
            this.txtDomainName.Name = "txtDomainName";
            this.txtDomainName.ReadOnly = true;
            this.txtDomainName.Size = new System.Drawing.Size(244, 20);
            this.txtDomainName.TabIndex = 5;
            // 
            // txtGroupBaseDN
            // 
            this.txtGroupBaseDN.Location = new System.Drawing.Point(97, 152);
            this.txtGroupBaseDN.Name = "txtGroupBaseDN";
            this.txtGroupBaseDN.Size = new System.Drawing.Size(240, 20);
            this.txtGroupBaseDN.TabIndex = 14;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 155);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(84, 13);
            this.label1.TabIndex = 13;
            this.label1.Text = "Group base DN:";
            // 
            // txtUserBaseDN
            // 
            this.txtUserBaseDN.Location = new System.Drawing.Point(97, 126);
            this.txtUserBaseDN.Name = "txtUserBaseDN";
            this.txtUserBaseDN.Size = new System.Drawing.Size(240, 20);
            this.txtUserBaseDN.TabIndex = 12;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(17, 129);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(77, 13);
            this.label4.TabIndex = 11;
            this.label4.Text = "User base DN:";
            // 
            // txtPrimaryURL
            // 
            this.txtPrimaryURL.Location = new System.Drawing.Point(97, 178);
            this.txtPrimaryURL.Name = "txtPrimaryURL";
            this.txtPrimaryURL.Size = new System.Drawing.Size(240, 20);
            this.txtPrimaryURL.TabIndex = 18;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(23, 181);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(69, 13);
            this.label9.TabIndex = 17;
            this.label9.Text = "Primary URL:";
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(42, 262);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(48, 13);
            this.label18.TabIndex = 55;
            this.label18.Text = "Settings:";
            // 
            // cbDNForNestedGroups
            // 
            this.cbDNForNestedGroups.AutoSize = true;
            this.cbDNForNestedGroups.Location = new System.Drawing.Point(190, 263);
            this.cbDNForNestedGroups.Name = "cbDNForNestedGroups";
            this.cbDNForNestedGroups.Size = new System.Drawing.Size(158, 17);
            this.cbDNForNestedGroups.TabIndex = 54;
            this.cbDNForNestedGroups.Text = "Base DN for Nested Groups";
            this.cbDNForNestedGroups.UseVisualStyleBackColor = true;
            // 
            // cbGroupSearch
            // 
            this.cbGroupSearch.AutoSize = true;
            this.cbGroupSearch.Location = new System.Drawing.Point(96, 285);
            this.cbGroupSearch.Name = "cbGroupSearch";
            this.cbGroupSearch.Size = new System.Drawing.Size(92, 17);
            this.cbGroupSearch.TabIndex = 53;
            this.cbGroupSearch.Text = "Group Search";
            this.cbGroupSearch.UseVisualStyleBackColor = true;
            // 
            // cbMatchRuleInChain
            // 
            this.cbMatchRuleInChain.AutoSize = true;
            this.cbMatchRuleInChain.Location = new System.Drawing.Point(190, 286);
            this.cbMatchRuleInChain.Name = "cbMatchRuleInChain";
            this.cbMatchRuleInChain.Size = new System.Drawing.Size(135, 17);
            this.cbMatchRuleInChain.TabIndex = 52;
            this.cbMatchRuleInChain.Text = "Matching Rule in chain";
            this.cbMatchRuleInChain.UseVisualStyleBackColor = true;
            // 
            // cbSiteAffinity
            // 
            this.cbSiteAffinity.AutoSize = true;
            this.cbSiteAffinity.Location = new System.Drawing.Point(96, 262);
            this.cbSiteAffinity.Name = "cbSiteAffinity";
            this.cbSiteAffinity.Size = new System.Drawing.Size(78, 17);
            this.cbSiteAffinity.TabIndex = 51;
            this.cbSiteAffinity.Text = "Site Affinity";
            this.cbSiteAffinity.UseVisualStyleBackColor = true;
            // 
            // txtSecondaryUrl
            // 
            this.txtSecondaryUrl.Location = new System.Drawing.Point(97, 204);
            this.txtSecondaryUrl.Name = "txtSecondaryUrl";
            this.txtSecondaryUrl.Size = new System.Drawing.Size(240, 20);
            this.txtSecondaryUrl.TabIndex = 57;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(5, 207);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(86, 13);
            this.label2.TabIndex = 56;
            this.label2.Text = "Secondary URL:";
            // 
            // ExternalDomainGeneralProperty
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.txtSecondaryUrl);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label18);
            this.Controls.Add(this.cbDNForNestedGroups);
            this.Controls.Add(this.cbGroupSearch);
            this.Controls.Add(this.cbMatchRuleInChain);
            this.Controls.Add(this.cbSiteAffinity);
            this.Controls.Add(this.txtPrimaryURL);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.txtGroupBaseDN);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtUserBaseDN);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.txtDomainAlias);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.txtDomainName);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.pbIcon);
            this.Controls.Add(this.lblDomainName);
            this.Controls.Add(this.groupBox1);
            this.Name = "ExternalDomainGeneralProperty";
            this.Size = new System.Drawing.Size(350, 359);
            ((System.ComponentModel.ISupportInitialize)(this.pbIcon)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label lblDomainName;
        private System.Windows.Forms.PictureBox pbIcon;
        private System.Windows.Forms.TextBox txtDomainAlias;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtDomainName;
        private System.Windows.Forms.TextBox txtGroupBaseDN;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtUserBaseDN;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox txtPrimaryURL;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.CheckBox cbDNForNestedGroups;
        private System.Windows.Forms.CheckBox cbGroupSearch;
        private System.Windows.Forms.CheckBox cbMatchRuleInChain;
        private System.Windows.Forms.CheckBox cbSiteAffinity;
        private System.Windows.Forms.TextBox txtSecondaryUrl;
        private System.Windows.Forms.Label label2;

    }
}
