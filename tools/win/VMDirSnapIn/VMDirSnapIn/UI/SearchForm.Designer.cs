namespace VMDirSnapIn.UI
{
    partial class SearchForm
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SearchForm));
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.openToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.saveToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonShowHide = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonShowHideOperAttr = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator6 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonSetPage = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.searchQueryControl1 = new UI.SearchQueryControl();
            this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.panel1 = new System.Windows.Forms.Panel();
            this.panel3 = new System.Windows.Forms.Panel();
            this.resultStatusLabel = new System.Windows.Forms.Label();
            this.resultTreeView = new System.Windows.Forms.TreeView();
            this.panel2 = new System.Windows.Forms.Panel();
            this.label1 = new System.Windows.Forms.Label();
            this.PrevButton = new System.Windows.Forms.Button();
            this.NextButton = new System.Windows.Forms.Button();
            this.currPageTextBox = new System.Windows.Forms.TextBox();
            this.propertiesControl1 = new UI.PropertiesControl();
            this.cmuResultTreeView = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmiAddToGroup = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiResetUserPassword = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmiVerifyUserPassword = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStrip1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.tableLayoutPanel2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.panel3.SuspendLayout();
            this.panel2.SuspendLayout();
            this.cmuResultTreeView.SuspendLayout();
            this.SuspendLayout();
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripSeparator1,
            this.openToolStripButton,
            this.toolStripSeparator2,
            this.saveToolStripButton,
            this.toolStripSeparator3,
            this.toolStripButtonShowHide,
            this.toolStripSeparator6,
            this.toolStripButtonSetPage,
            this.toolStripSeparator4,
            this.toolStripButtonShowHideOperAttr});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(1008, 25);
            this.toolStrip1.TabIndex = 11;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
            // 
            // openToolStripButton
            // 
            this.openToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.openToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("openToolStripButton.Image")));
            this.openToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.openToolStripButton.Name = "openToolStripButton";
            this.openToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.openToolStripButton.Text = "&Open";
            this.openToolStripButton.Click += new System.EventHandler(this.openToolStripButton_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 25);
            // 
            // saveToolStripButton
            // 
            this.saveToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.saveToolStripButton.Image = ((System.Drawing.Image)(resources.GetObject("saveToolStripButton.Image")));
            this.saveToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.saveToolStripButton.Name = "saveToolStripButton";
            this.saveToolStripButton.Size = new System.Drawing.Size(23, 22);
            this.saveToolStripButton.Text = "&Save";
            this.saveToolStripButton.Click += new System.EventHandler(this.saveToolStripButton_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonShowHide
            // 
            this.toolStripButtonShowHide.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonShowHide.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonShowHide.Image")));
            this.toolStripButtonShowHide.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonShowHide.Name = "toolStripButtonShowHide";
            this.toolStripButtonShowHide.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonShowHide.Text = "Hide/Show Search Box";
            this.toolStripButtonShowHide.Click += new System.EventHandler(this.toolStripButtonShowHide_Click);
            // 
            // toolStripSeparator6
            // 
            this.toolStripSeparator6.Name = "toolStripSeparator6";
            this.toolStripSeparator6.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonSetPage
            // 
            this.toolStripButtonSetPage.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonSetPage.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonSetPage.Image")));
            this.toolStripButtonSetPage.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonSetPage.Name = "toolStripButtonSetPage";
            this.toolStripButtonSetPage.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonSetPage.Text = "Set Page Size";
            this.toolStripButtonSetPage.Click += new System.EventHandler(this.toolStripButtonSetPage_Click);
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            this.toolStripSeparator4.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonShowHideOperAttr
            // 
            this.toolStripButtonShowHideOperAttr.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonShowHideOperAttr.Image = VMDirEnvironment.Instance.GetImageResource(VMDirIconIndex.ShowOperationalAttr);
            this.toolStripButtonShowHideOperAttr.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonShowHideOperAttr.Name = "toolStripButtonShowHideOperAttr";
            this.toolStripButtonShowHideOperAttr.Size = new System.Drawing.Size(23, 17);
            this.toolStripButtonShowHideOperAttr.Text = "Show/Hide Operational Attributes";
            this.toolStripButtonShowHideOperAttr.Click += new System.EventHandler(this.toolStripButtonShowHideOperAttr_Click);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.AutoSize = true;
            this.tableLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.Controls.Add(this.searchQueryControl1, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 0, 1);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 25);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.Size = new System.Drawing.Size(1008, 553);
            this.tableLayoutPanel1.TabIndex = 12;
            // 
            // searchQueryControl1
            // 
            this.searchQueryControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.searchQueryControl1.Location = new System.Drawing.Point(3, 3);
            this.searchQueryControl1.Name = "searchQueryControl1";
            this.searchQueryControl1.Size = new System.Drawing.Size(1002, 271);
            this.searchQueryControl1.TabIndex = 0;
            this.searchQueryControl1.Load += new System.EventHandler(this.searchQueryControl1_Load);
            // 
            // tableLayoutPanel2
            // 
            this.tableLayoutPanel2.ColumnCount = 2;
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 75F));
            this.tableLayoutPanel2.Controls.Add(this.panel1, 0, 0);
            this.tableLayoutPanel2.Controls.Add(this.propertiesControl1, 1, 0);
            this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 280);
            this.tableLayoutPanel2.Name = "tableLayoutPanel2";
            this.tableLayoutPanel2.RowCount = 1;
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel2.Size = new System.Drawing.Size(1002, 270);
            this.tableLayoutPanel2.TabIndex = 1;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.panel3);
            this.panel1.Controls.Add(this.resultTreeView);
            this.panel1.Controls.Add(this.panel2);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 3);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(244, 264);
            this.panel1.TabIndex = 0;
            // 
            // panel3
            // 
            this.panel3.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.panel3.Controls.Add(this.resultStatusLabel);
            this.panel3.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel3.Location = new System.Drawing.Point(0, 0);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(244, 20);
            this.panel3.TabIndex = 3;
            // 
            // resultStatusLabel
            // 
            this.resultStatusLabel.AutoSize = true;
            this.resultStatusLabel.Location = new System.Drawing.Point(22, 4);
            this.resultStatusLabel.Name = "resultStatusLabel";
            this.resultStatusLabel.Size = new System.Drawing.Size(35, 13);
            this.resultStatusLabel.TabIndex = 0;
            this.resultStatusLabel.Text = "label2";
            // 
            // resultTreeView
            // 
            this.resultTreeView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.resultTreeView.Location = new System.Drawing.Point(0, 20);
            this.resultTreeView.Name = "resultTreeView";
            this.resultTreeView.ShowLines = false;
            this.resultTreeView.Size = new System.Drawing.Size(244, 206);
            this.resultTreeView.TabIndex = 2;
            this.resultTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.resultTreeView_AfterSelect);
            this.resultTreeView.MouseUp +=new System.Windows.Forms.MouseEventHandler(resultTreeView_MouseUp);
            // 
            // panel2
            // 
            this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel2.Controls.Add(this.label1);
            this.panel2.Controls.Add(this.PrevButton);
            this.panel2.Controls.Add(this.NextButton);
            this.panel2.Controls.Add(this.currPageTextBox);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel2.Location = new System.Drawing.Point(0, 229);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(244, 35);
            this.panel2.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(54, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(32, 13);
            this.label1.TabIndex = 7;
            this.label1.Text = "Page";
            // 
            // PrevButton
            // 
            this.PrevButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.PrevButton.Location = new System.Drawing.Point(8, 2);
            this.PrevButton.Name = "PrevButton";
            this.PrevButton.Size = new System.Drawing.Size(40, 23);
            this.PrevButton.TabIndex = 6;
            this.PrevButton.Text = "<";
            this.PrevButton.UseVisualStyleBackColor = true;
            this.PrevButton.Click += new System.EventHandler(this.PrevButton_Click);
            // 
            // NextButton
            // 
            this.NextButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.NextButton.Location = new System.Drawing.Point(190, 2);
            this.NextButton.Name = "NextButton";
            this.NextButton.Size = new System.Drawing.Size(40, 23);
            this.NextButton.TabIndex = 5;
            this.NextButton.Text = ">";
            this.NextButton.UseVisualStyleBackColor = true;
            this.NextButton.Click += new System.EventHandler(this.NextButton_Click);
            // 
            // currPageTextBox
            // 
            this.currPageTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)));
            this.currPageTextBox.Location = new System.Drawing.Point(90, 4);
            this.currPageTextBox.Name = "currPageTextBox";
            this.currPageTextBox.Size = new System.Drawing.Size(80, 20);
            this.currPageTextBox.TabIndex = 4;
            // 
            // propertiesControl1
            // 
            this.propertiesControl1.AutoScroll = true;
            this.propertiesControl1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.propertiesControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propertiesControl1.Location = new System.Drawing.Point(253, 3);
            this.propertiesControl1.Name = "propertiesControl1";
            this.propertiesControl1.Size = new System.Drawing.Size(746, 264);
            this.propertiesControl1.TabIndex = 1;
            // 
            // cmuResultTreeView
            // 
            this.cmuResultTreeView.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiAddToGroup,
            this.tsmiResetUserPassword,
            this.tsmiVerifyUserPassword});
            this.cmuResultTreeView.Name = "cmuResultTreeView";
            this.cmuResultTreeView.Size = new System.Drawing.Size(184, 92);
            // 
            // tsmiAddToGroup
            // 
            this.tsmiAddToGroup.Name = "tsmiAddToGroup";
            this.tsmiAddToGroup.Size = new System.Drawing.Size(183, 22);
            this.tsmiAddToGroup.Text = "Add To Group";
            this.tsmiAddToGroup.Click += new System.EventHandler(this.tsmiAddToGroup_Click);
            // 
            // tsmiResetUserPassword
            // 
            this.tsmiResetUserPassword.Name = "tsmiResetUserPassword";
            this.tsmiResetUserPassword.Size = new System.Drawing.Size(183, 22);
            this.tsmiResetUserPassword.Text = "Reset User Password";
            this.tsmiResetUserPassword.Click += new System.EventHandler(this.tsmiResetUserPassword_Click);
            // 
            // tsmiVerifyUserPassword
            // 
            this.tsmiVerifyUserPassword.Name = "tsmiVerifyUserPassword";
            this.tsmiVerifyUserPassword.Size = new System.Drawing.Size(183, 22);
            this.tsmiVerifyUserPassword.Text = "Verify User Password";
            this.tsmiVerifyUserPassword.Click += new System.EventHandler(this.tsmiVerifyUserPassword_Click);
            // 
            // SearchForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(1008, 578);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.toolStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "SearchForm";
            this.Text = "SearchForm";
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel2.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.panel3.ResumeLayout(false);
            this.panel3.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.cmuResultTreeView.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripButton openToolStripButton;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripButton saveToolStripButton;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private UI.SearchQueryControl searchQueryControl1;
        private UI.PropertiesControl propertiesControl1;
        private System.Windows.Forms.ToolStripButton toolStripButtonShowHide;
        private System.Windows.Forms.ToolStripButton toolStripButtonShowHideOperAttr;
        private System.Windows.Forms.ToolStripButton toolStripButtonSetPage;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator6;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.TreeView resultTreeView;
        private System.Windows.Forms.Panel panel3;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button PrevButton;
        private System.Windows.Forms.Button NextButton;
        private System.Windows.Forms.TextBox currPageTextBox;
        private System.Windows.Forms.Label resultStatusLabel;
        private System.Windows.Forms.ContextMenuStrip cmuResultTreeView;
        private System.Windows.Forms.ToolStripMenuItem tsmiAddToGroup;
        private System.Windows.Forms.ToolStripMenuItem tsmiResetUserPassword;
        private System.Windows.Forms.ToolStripMenuItem tsmiVerifyUserPassword;
    }
}