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

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDir.Common.Schema;
using VMDir.Common.VMDirUtilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaSnapIn.UI
{
    public partial class AttributeTypeWindow : Form
    {
        private bool isAddMode = false;

        public AttributeTypeDTO AttributeDTO { get; set; }

        public AttributeTypeDTO AttributeModDTO { get; set; }

        private string[] syntaxList;

        public AttributeTypeWindow()
        {
            InitializeComponent();
            isAddMode = true;
            AttributeDTO = new AttributeTypeDTO();
            Initialise();
        }

        public AttributeTypeWindow(AttributeTypeDTO attrType)
        {
            InitializeComponent();
            isAddMode = false;
            AttributeDTO = attrType;
            AttributeModDTO = new AttributeTypeDTO();
            Initialise();
        }

        private void FillControlsWithAttributeData()
        {
            this.NameTextBox.Text = AttributeDTO.Name;
            this.DescriptionTextBox.Text = AttributeDTO.Description;
            this.AttributeIdentifierTextbox.Text = AttributeDTO.AttributeID;
            if (!AttributeDTO.SingleValue)
                MultiValuedCheckBox.Checked = true;
            AttributeSyntaxCombo.SelectedIndex = Array.IndexOf(syntaxList, AttributeDTO.Type);
        }

        private void Initialise()
        {
            syntaxList = VMDirCommonEnvironment.Instance.SyntaxDefs.SyntaxList.Select(x => x.Name).ToArray();
            this.AttributeSyntaxCombo.Items.AddRange(syntaxList);
            if (!isAddMode)
            {
                FillControlsWithAttributeData();
                EnableUIControls(false);
                this.AddButton.Text = VMwareMMCIDP.UI.Common.Utilities.MMCUIConstants.EDIT;
            }

        }


        private void EnableUIControls(bool state)
        {
            this.NameTextBox.ReadOnly = !state;
            this.AttributeIdentifierTextbox.ReadOnly = !state;
            this.AttributeSyntaxCombo.Enabled = state;
            this.DescriptionTextBox.ReadOnly = !state;
            this.MultiValuedCheckBox.Enabled = state;
        }

        private void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(this.NameTextBox.Text) || string.IsNullOrWhiteSpace(this.AttributeIdentifierTextbox.Text))
                throw new Exception(VMwareMMCIDP.UI.Common.Utilities.MMCUIConstants.VALUES_EMPTY);
        }


        private void FillDTOWithUIControls()
        {
            if (isAddMode)
            {
                AttributeDTO.Description = this.DescriptionTextBox.Text;
                AttributeDTO.Name = this.NameTextBox.Text;
                AttributeDTO.LDAPDisplayName = this.NameTextBox.Text;
                AttributeDTO.AttributeID = this.AttributeIdentifierTextbox.Text;
                var syntax = VMDirCommonEnvironment.Instance.SyntaxDefs.LookupSyntaxByName(this.AttributeSyntaxCombo.SelectedItem.ToString());
                if (syntax != null)
                    AttributeDTO.AttributeSyntax = syntax.Value;
                AttributeDTO.SingleValue = (MultiValuedCheckBox.Checked != true);
            }
            else
            {
                AttributeModDTO.Name = AttributeDTO.Name;
                if (!this.DescriptionTextBox.Text.Equals(AttributeDTO.Description))
                    AttributeModDTO.Description = this.DescriptionTextBox.Text;
                if (AttributeDTO.SingleValue == true && MultiValuedCheckBox.Checked == true)
                    AttributeModDTO.SingleValue = false;
                else
                    AttributeModDTO.SingleValue = AttributeDTO.SingleValue;
            }
        }

        private void SetUIToolsEditability()
        {
            this.DescriptionTextBox.ReadOnly = false;
            if (MultiValuedCheckBox.Checked != true)
                MultiValuedCheckBox.Enabled = true;
        }

        private void AddButton_Click_1(object sender, EventArgs e)
        {

            UIErrorHelper.CheckedExecNonModal(delegate()
                {
                    if (this.AddButton.Text == VMwareMMCIDP.UI.Common.Utilities.MMCUIConstants.EDIT)
                    {
                        SetUIToolsEditability();
                        this.AddButton.Text = VMwareMMCIDP.UI.Common.Utilities.MMCUIConstants.UPDATE;
                    }

                    else if (UIErrorHelper.ShowConfirm(VMwareMMCIDP.UI.Common.Utilities.MMCUIConstants.CONFIRM) == DialogResult.Yes)
                    {
                        DoValidateControls();
                        FillDTOWithUIControls();
                        this.Close();
                        this.DialogResult = DialogResult.OK;

                    }
                });
        }

        private void CancelButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }
    }
}
