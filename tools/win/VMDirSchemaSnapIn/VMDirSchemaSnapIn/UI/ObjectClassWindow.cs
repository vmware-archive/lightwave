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

using Microsoft.ManagementConsole;
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
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaSnapIn.UI
{
    public partial class ObjectClassWindow : Form
    {
        public ObjectClassDTO ObjectDTO { get; set; }

        public ObjectClassDTO ObjectClassModDTO { get; set; }

        public AttributeTypeManager attributeManager { get; set; }

        public ObjectClassManager objectManager { get; set; }

        public SchemaManager schemaManager { get; set; }

        private bool isAddMode = false;
        private List<string> allAttributesList = null;
        private List<string> allClassesList = null;
        private List<string> auxiliaryClassesList = new List<string>();
        private List<string> mandatoryAttributesList = new List<string>();
        private List<string> optionalAttributesList = new List<string>();
        //private List<string> parentMandatoryAttributes = new List<string>();
        //private List<string> parentOptionalAttributes = new List<string>();

        //object class types
        private List<string> structuralClasses = null;
        private List<string> abstractClasses = null;
        private List<string> auxiliaryClasses = null;
        private List<string> parentClassList = new List<string>();

        public ObjectClassWindow(SchemaManager schemaManager)
        {
            InitializeComponent();
            isAddMode = true;
            this.schemaManager = schemaManager;
            this.attributeManager = schemaManager.GetAttributeTypeManager();
            this.objectManager = schemaManager.GetObjectClassManager();
            Initialise();
        }

        public ObjectClassWindow(ObjectClassDTO dto, SchemaManager schemaManager)
        {
            InitializeComponent();
            this.ObjectDTO = dto;
            isAddMode = false;
            this.schemaManager = schemaManager;
            this.attributeManager = schemaManager.GetAttributeTypeManager();
            this.objectManager = schemaManager.GetObjectClassManager();
            Initialise();
            
        }

        public void Initialise()
        {
            InitialiseAttributesList();
            InitialiseClassesList();
            //this.RemoveAuxiliaryAttribtueButton.Enabled = false;
            // this.AddAuxiliaryAttributeButton.Enabled = false;
            if (!isAddMode)
            {
                ObjectClassModDTO = new ObjectClassDTO();
                InitialiseControlsWithDTO();
                this.AddButton.Text = "Edit";
                SetUIFieldsEditability(false);
            }
            else
            {
                ObjectDTO = new ObjectClassDTO();
                parentClassList.AddRange(abstractClasses);
            }
        }

        public void InitialiseAttributesList()
        {
            var data = attributeManager.Data;
            allAttributesList = data.Select(x => x.Key).ToList();
        }

        public void InitialiseClassesList()
        {
            var data = objectManager.Data;
            allClassesList = data.Select(x => x.Key).ToList();
            structuralClasses = data.Where(x => x.Value.ClassType == ObjectClassDTO.ObjectClassType.Structural).Select(x => x.Value.Name).ToList();
            abstractClasses = data.Where(x => x.Value.ClassType == ObjectClassDTO.ObjectClassType.Abstract).Select(x => x.Value.Name).ToList();
            auxiliaryClasses = data.Where(x => x.Value.ClassType == ObjectClassDTO.ObjectClassType.Auxiliary).Select(x => x.Value.Name).ToList();
        }

        private void SetUIFieldsEditability(bool status)
        {
            this.ObjectClassNameText.ReadOnly = !status;
            this.ObjectClassIdentifierText.ReadOnly = !status;
            this.DescriptionText.ReadOnly = !status;
            this.ClassTypeCombo.Enabled = status;
            this.GovernsIDTextField.ReadOnly = !status;
            this.ParentClassText.ReadOnly = true;
            this.AddParentClassButton.Enabled = status;
            this.AddAuxiliaryAttributeButton.Enabled = status;
            this.RemoveAuxiliaryAttribtueButton.Enabled = status;
            this.AddMandatoryAttributeButton.Enabled = status;
            this.RemoveMandatoryAttributeButton.Enabled = status;
            this.AddOptionalAttributeButton.Enabled = status;
            this.RemoveOptionalAttributeButton.Enabled = status;

        }

        private void InitialiseControlsWithDTO()
        {
            this.ObjectClassNameText.Text = ObjectDTO.Name;
            this.DescriptionText.Text = ObjectDTO.Description ?? string.Empty;
            this.ObjectClassIdentifierText.Text = ObjectDTO.Name;
            this.ParentClassText.Text = ObjectDTO.SuperClass;
            this.ClassTypeCombo.SelectedIndex = ((int)ObjectDTO.ClassType - 1);
            if (ObjectDTO.Must != null)
                mandatoryAttributesList = ObjectDTO.Must;
            this.MandatoryList.DataSource = mandatoryAttributesList;
            if (ObjectDTO.May != null)
                optionalAttributesList = ObjectDTO.May;
            this.OptionalList.DataSource = optionalAttributesList;
            if (ObjectDTO.Aux != null)
                auxiliaryClassesList = ObjectDTO.Aux;
            this.AuxiliaryList.DataSource = auxiliaryClassesList;

            this.GovernsIDTextField.Text = ObjectDTO.GovernsID;
        }

        private void DoValidateControls()
        {
            if (String.IsNullOrWhiteSpace(this.ObjectClassNameText.Text) || String.IsNullOrWhiteSpace(this.ClassTypeCombo.SelectedItem.ToString()) || String.IsNullOrWhiteSpace(this.ObjectClassIdentifierText.Text)
                || String.IsNullOrWhiteSpace(this.ParentClassText.Text) || String.IsNullOrWhiteSpace(this.GovernsIDTextField.Text))
                throw new Exception("One or more fields empty");

            if (!isAddMode)
            {
                if (ObjectClassModDTO == null)
                    throw new Exception("No changes");
            }
        }

        private void AddMandatoryAttributeButton_Click(object sender, EventArgs e)
        {
            if (isAddMode)
            {
                var frm = new SelectItemsWindow(allAttributesList, mandatoryAttributesList, null);

                if (frm.ShowDialog() == DialogResult.OK)
                {
                    mandatoryAttributesList.AddRange(frm.SelectedItemsList);
                    this.MandatoryList.DataSource = null;
                    this.MandatoryList.DataSource = mandatoryAttributesList;
                    this.MandatoryList.Refresh();
                }
            }
        }

        private void RemoveMandatoryAttributeButton_Click(object sender, EventArgs e)
        {
            if (isAddMode)
            {
                int row = (int)this.MandatoryList.SelectedIndex;
                if (row >= 0)
                {
                    mandatoryAttributesList.RemoveAt(row);
                    this.MandatoryList.DataSource = null;
                    this.MandatoryList.DataSource = mandatoryAttributesList;
                    this.MandatoryList.Refresh();
                }
            }
        }

        private void AddOptionalAttributeButton_Click(object sender, EventArgs e)
        {
            var frm = new SelectItemsWindow(allAttributesList, optionalAttributesList, null);
            if (frm.ShowDialog() == DialogResult.OK)
            {
                optionalAttributesList.AddRange(frm.SelectedItemsList);
                this.OptionalList.DataSource = null;
                this.OptionalList.DataSource = optionalAttributesList;
                this.OptionalList.Refresh();
            }
        }

        private void RemoveOptionalAttributeButton_Click(object sender, EventArgs e)
        {
            if (isAddMode)
            {
                int row = (int)this.OptionalList.SelectedIndex;
                if (row >= 0)
                {
                    optionalAttributesList.RemoveAt(row);
                    this.OptionalList.DataSource = null;
                    this.OptionalList.DataSource = optionalAttributesList;
                    this.OptionalList.Refresh();
                }  
            }
        }

        private void AddAuxiliaryAttributeButton_Click(object sender, EventArgs e)
        {
            var frm = new SelectItemsWindow(auxiliaryClasses, auxiliaryClassesList, null);
            if (frm.ShowDialog() == DialogResult.OK)
            {
                auxiliaryClassesList.AddRange(frm.SelectedItemsList);
                this.AuxiliaryList.DataSource = null;
                this.AuxiliaryList.DataSource = auxiliaryClassesList;
                this.AuxiliaryList.Refresh();
            }
        }

        private void RemoveAuxiliaryAttribtueButton_Click(object sender, EventArgs e)
        {
            if (isAddMode)
            {
                int row = (int)this.AuxiliaryList.SelectedIndex;
                if (row >= 0)
                {
                    auxiliaryClassesList.RemoveAt(row);
                    this.AuxiliaryList.DataSource = null;
                    this.AuxiliaryList.DataSource = auxiliaryClassesList;
                    this.AuxiliaryList.Refresh();
                }
            }
        }

        private void UpdateObjectClassDTOWithUIValues()
        {
            if (isAddMode)
            {
                ObjectDTO.Name = this.ObjectClassNameText.Text;
                ObjectDTO.Description = this.DescriptionText.Text;
                ObjectDTO.SuperClass = this.ParentClassText.Text;
                //optionalAttributesList.AddRange(parentOptionalAttributes);
                ObjectDTO.GovernsID = this.GovernsIDTextField.Text;
                ObjectDTO.May = optionalAttributesList;
                ObjectDTO.Must = mandatoryAttributesList;
                ObjectDTO.Aux = auxiliaryClassesList;
                ObjectDTO.ClassType = (ObjectClassDTO.ObjectClassType)((int)this.ClassTypeCombo.SelectedIndex) + 1;
            }
            else
            {
                ObjectClassModDTO.Name = this.ObjectClassNameText.Text;
                if (String.Equals(ObjectDTO.Description, this.DescriptionText.Text) != true)
                    ObjectClassModDTO.Description = this.DescriptionText.Text;
                ObjectClassModDTO.May = optionalAttributesList;
                ObjectClassModDTO.Aux = auxiliaryClassesList;

            }
        }

        //set only editable object class fields to true.
        private void SetEditableFields()
        {
            this.DescriptionText.ReadOnly = false;
            this.AddAuxiliaryAttributeButton.Enabled = true;
            this.AddOptionalAttributeButton.Enabled = true;
            this.AddMandatoryAttributeButton.Visible = false;
            this.AddParentClassButton.Visible = false;
            this.RemoveAuxiliaryAttribtueButton.Visible = false;
            this.RemoveMandatoryAttributeButton.Visible = false;
            this.RemoveOptionalAttributeButton.Visible = false;
        }

        private void AddButton_Click(object sender, EventArgs e)
        {
            if (this.AddButton.Text == "Edit")
            {
                this.AddButton.Text = "Update";
                SetEditableFields();
            }
            else if (UIErrorHelper.ShowConfirm(VMwareMMCIDP.UI.Common.Utilities.MMCUIConstants.CONFIRM) == DialogResult.Yes)
            {

                UIErrorHelper.CheckedExecNonModal(delegate()
                    {
                        DoValidateControls();
                        UpdateObjectClassDTOWithUIValues();
                        this.DialogResult = DialogResult.OK;
                        this.Close();
                    });
            }
        }

        private void CloseButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void AddParentClassButton_Click(object sender, EventArgs e)
        {
            var frm = new SelectObjectClassWindow(parentClassList);
            if (frm.ShowDialog() == DialogResult.OK)
            {
                this.ParentClassText.Text = frm.SelectedItem;

                //mandatoryAttributesList.RemoveAll(item => parentMandatoryAttributes.Contains(item));

                // parentMandatoryAttributes.Clear();
                // parentOptionalAttributes.Clear();

                // parentMandatoryAttributes.AddRange(this.schemaManager.GetRequiredAttributes(this.ParentClassText.Text).Select(item => item.Name).ToList());
                // parentOptionalAttributes.AddRange(this.schemaManager.GetOptionalAttributes(this.ParentClassText.Text).Select(item => item.Name).ToList());


                // mandatoryAttributesList.AddRange(parentMandatoryAttributes);
                this.MandatoryList.DataSource = mandatoryAttributesList;
                this.MandatoryList.Refresh();
            }
        }

        private void OnSelectionChanged(object sender, EventArgs e)
        {
            parentClassList.Clear();
            if (String.Equals(this.ClassTypeCombo.SelectedItem.ToString(), VMDirSchemaConstants.VMDIRSCHEMA_STRUCTURAL) == true)
            {
                //this.AddAuxiliaryAttributeButton.Enabled = true;
                //this.RemoveAuxiliaryAttribtueButton.Enabled = true;
                this.ParentClassText.Enabled = true;
                parentClassList.AddRange(structuralClasses);
                parentClassList.AddRange(abstractClasses);
            }
            else
            {
                if (String.Equals(this.ClassTypeCombo.SelectedItem.ToString(), VMDirSchemaConstants.VMDIRSCHEMA_AUXILIARY) == true)
                {
                    this.ParentClassText.Enabled = false;
                    parentClassList.AddRange(abstractClasses);
                    parentClassList.AddRange(auxiliaryClasses);
                }
                else
                {
                    this.ParentClassText.Enabled = true;
                    parentClassList.AddRange(abstractClasses);
                }
                auxiliaryClassesList.Clear();
                this.AuxiliaryList.Refresh();
                // this.AddAuxiliaryAttributeButton.Enabled = false;
                // this.RemoveAuxiliaryAttribtueButton.Enabled = false;

            }
        }
    }
}
