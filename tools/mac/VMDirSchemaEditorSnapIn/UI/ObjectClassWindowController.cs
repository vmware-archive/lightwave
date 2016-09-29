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

using Foundation;
using AppKit;
using VMDir.Common.Schema;
using VmIdentity.UI.Common;
using System.Collections.Generic;
using VMDirSchemaEditorSnapIn.ListViews;
using System.Linq;
using VmIdentity.UI.Common.Utilities;
using VMDirSchema;

namespace VMDirSchemaEditorSnapIn
{
    public partial class ObjectClassWindowController : NSWindowController
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
        // private List<string> parentMandatoryAttributes = new List<string>();
        // private List<string> parentOptionalAttributes = new List<string>();

        //object class types
        private List<string> structuralClasses = null;
        private List<string> abstractClasses = null;
        private List<string> auxiliaryClasses = null;
        private List<string> parentClassList = new List<string>();

        public ObjectClassWindowController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public ObjectClassWindowController(NSCoder coder)
            : base(coder)
        {
        }

        public ObjectClassWindowController(SchemaManager schemaManager)
            : base("ObjectClassWindow")
        {
            isAddMode = true;
            this.schemaManager = schemaManager;
            this.attributeManager = schemaManager.GetAttributeTypeManager();
            this.objectManager = schemaManager.GetObjectClassManager();
            InitialiseAttributesList();
            InitialiseClassesList();
        }

        public ObjectClassWindowController(ObjectClassDTO dto, SchemaManager schemaManager)
            : this(schemaManager)
        {
            this.ObjectDTO = dto;
            isAddMode = false;
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

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            //AuxiliaryClassesListView.Enabled = false;
            // RemoveAuxiliaryClassesButton.Enabled = false;
            //AddAuxiliaryClassesButton.Enabled = false;
            if (!isAddMode)
            {
                ObjectClassModDTO = new ObjectClassDTO();
                InitialiseControlsWithDTO();
                ActionButton.Title = VMIdentityConstants.EDIT;
                SetUIFieldsEditability(false);
            }
            else
            {
                ObjectDTO = new ObjectClassDTO();
                parentClassList.AddRange(abstractClasses);
            }
        }

        partial void OnClassTypePopupChanged(Foundation.NSObject sender)
        {
            parentClassList.Clear();
            if (String.Equals(ClassTypePopup.TitleOfSelectedItem, VMDirSchemaConstants.VMDIRSCHEMA_STRUCTURAL) == true)
            {
                // AuxiliaryClassesListView.Enabled = true;
                // AddAuxiliaryClassesButton.Enabled = true;
                //RemoveAuxiliaryClassesButton.Enabled = true;
                ParentClass.Enabled = true;
                parentClassList.AddRange(structuralClasses);
                parentClassList.AddRange(abstractClasses);
            }
            else
            {
                if (String.Equals(ClassTypePopup.TitleOfSelectedItem, VMDirSchemaConstants.VMDIRSCHEMA_AUXILIARY) == true)
                {
                    ParentClass.Enabled = false;
                    parentClassList.AddRange(abstractClasses);
                    parentClassList.AddRange(auxiliaryClasses);
                }
                else
                {
                    ParentClass.Enabled = true;
                    parentClassList.AddRange(abstractClasses);
                }
                auxiliaryClassesList.Clear();
                AuxiliaryClassesListView.ReloadData();
                // AuxiliaryClassesListView.Enabled = false;
                // AddAuxiliaryClassesButton.Enabled = false;
                // RemoveAuxiliaryClassesButton.Enabled = false;

            }
        }

        private void SetUIFieldsEditability(bool status)
        {
            ObjectClassName.Enabled = status;
            ObjectClassID.Enabled = status;
            ObjectClassDescription.Enabled = status;
            ClassTypePopup.Enabled = status;
            ParentClass.Enabled = status;
            GovernsIDField.Enabled = status;
            AddObjectClassButton.Enabled = status;
            AuxiliaryClassesListView.Enabled = status;
            MandatoryAttributesListView.Enabled = status;
            OptionalAttributesListView.Enabled = status;
            AddAuxiliaryClassesButton.Enabled = status;
            RemoveAuxiliaryClassesButton.Enabled = status;
            AddMandatoryAttributesButton.Enabled = status;
            RemoveMandatoryAttributesButton.Enabled = status;
            AddOptionalAttributesButton.Enabled = status;
            RemoveOptionalAttributesButton.Enabled = status;

        }

        private void InitialiseControlsWithDTO()
        {
            ObjectClassName.StringValue = ObjectDTO.Name;
            ObjectClassDescription.StringValue = ObjectDTO.Description ?? string.Empty;
            ObjectClassID.StringValue = ObjectDTO.Name;
            ParentClass.StringValue = ObjectDTO.SuperClass;
            GovernsIDField.StringValue = ObjectDTO.GovernsID;
            ClassTypePopup.SelectItem((int)ObjectDTO.ClassType - 1);
            if (ObjectDTO.Must != null)
                mandatoryAttributesList = ObjectDTO.Must;
            MandatoryAttributesListView.DataSource = new StringItemsListView(mandatoryAttributesList);
            if (ObjectDTO.May != null)
                optionalAttributesList = ObjectDTO.May;
            OptionalAttributesListView.DataSource = new StringItemsListView(optionalAttributesList);
            if (ObjectDTO.Aux != null)
                auxiliaryClassesList = ObjectDTO.Aux;
            AuxiliaryClassesListView.DataSource = new StringItemsListView(auxiliaryClassesList);

        }

        partial void AddAuxiliaryClasses(Foundation.NSObject sender)
        {
            if (AuxiliaryClassesListView.Enabled == true)
            {
				SelectListItemsWindowController swc = new SelectListItemsWindowController(auxiliaryClasses, auxiliaryClassesList, null);
                nint ret = NSApplication.SharedApplication.RunModalForWindow(swc.Window);
                if (ret == VMIdentityConstants.DIALOGOK)
                {
                    auxiliaryClassesList.AddRange(swc.SelectedItemsList);
                    AuxiliaryClassesListView.DataSource = new StringItemsListView(auxiliaryClassesList);
                    AuxiliaryClassesListView.ReloadData();
                }
            }
        }


        partial void AddMandatoryAttributes(Foundation.NSObject sender)
        {
            if (MandatoryAttributesListView.Enabled == true && isAddMode)
            {
                SelectListItemsWindowController swc = new SelectListItemsWindowController(allAttributesList, mandatoryAttributesList, null);
                nint ret = NSApplication.SharedApplication.RunModalForWindow(swc.Window);
                if (ret == VMIdentityConstants.DIALOGOK)
                {
                    mandatoryAttributesList.AddRange(swc.SelectedItemsList);
                    MandatoryAttributesListView.DataSource = new StringItemsListView(mandatoryAttributesList);
                    MandatoryAttributesListView.ReloadData();
                }
            }
        }

        partial void AddObjectClass(Foundation.NSObject sender)
        {
            SelectObjectClassWindowController swc = new SelectObjectClassWindowController(parentClassList);
            nint ret = NSApplication.SharedApplication.RunModalForWindow(swc.Window);
            if (ret == VMIdentityConstants.DIALOGOK)
            {
                ParentClass.StringValue = swc.SelectedItem;

                // mandatoryAttributesList.RemoveAll(item => parentMandatoryAttributes.Contains(item));

                // parentMandatoryAttributes.Clear();
                // parentOptionalAttributes.Clear();

                // parentMandatoryAttributes.AddRange(this.schemaManager.GetRequiredAttributes(ParentClass.StringValue).Select(e => e.Name).ToList());
                //  parentOptionalAttributes.AddRange(this.schemaManager.GetOptionalAttributes(ParentClass.StringValue).Select(e => e.Name).ToList());


                // mandatoryAttributesList.AddRange(parentMandatoryAttributes);
                MandatoryAttributesListView.DataSource = new StringItemsListView(mandatoryAttributesList);
                MandatoryAttributesListView.ReloadData();

            }
        }

        partial void AddOptionalAttributes(Foundation.NSObject sender)
        {
            if (OptionalAttributesListView.Enabled == true)
            {
                SelectListItemsWindowController swc = new SelectListItemsWindowController(allAttributesList, optionalAttributesList, null);
                nint ret = NSApplication.SharedApplication.RunModalForWindow(swc.Window);
                if (ret == VMIdentityConstants.DIALOGOK)
                {
                    optionalAttributesList.AddRange(swc.SelectedItemsList);
                    OptionalAttributesListView.DataSource = new StringItemsListView(optionalAttributesList);
                    OptionalAttributesListView.ReloadData();
                }
            }
        }

        partial void RemoveAuxiliaryClasses(Foundation.NSObject sender)
        {
            if (AuxiliaryClassesListView.Enabled == true && isAddMode)
            {
                int row = (int)this.AuxiliaryClassesListView.SelectedRow;
                if (row >= 0)
                {
                    auxiliaryClassesList.RemoveAt(row);
                }
                this.AuxiliaryClassesListView.ReloadData();
            }
        }

        partial void RemoveMandatoryAttributes(Foundation.NSObject sender)
        {
            if (MandatoryAttributesListView.Enabled == true && isAddMode)
            {
                int row = (int)this.MandatoryAttributesListView.SelectedRow;
                if (row >= 0)
                {
                    mandatoryAttributesList.RemoveAt(row);
                }
                this.MandatoryAttributesListView.ReloadData();
            }
        }

        partial void RemoveOptionalAttributes(Foundation.NSObject sender)
        {
            if (OptionalAttributesListView.Enabled == true && isAddMode)
            {
                int row = (int)this.OptionalAttributesListView.SelectedRow;
                if (row >= 0)
                {
                    optionalAttributesList.RemoveAt(row);
                }
                this.OptionalAttributesListView.ReloadData();
            }
        }

        private void DoValidateControls()
        {
            if (String.IsNullOrWhiteSpace(ObjectClassName.StringValue) || String.IsNullOrWhiteSpace(ClassTypePopup.TitleOfSelectedItem) || String.IsNullOrWhiteSpace(ObjectClassID.StringValue)
                || String.IsNullOrWhiteSpace(ParentClass.StringValue) || String.IsNullOrWhiteSpace(GovernsIDField.StringValue))
                throw new Exception(VMIdentityConstants.EMPTY_FIELD);

            if (!isAddMode)
            {
                if (ObjectClassModDTO == null)
                    throw new Exception(VMDirSchemaConstants.NO_CHANGE_DETECTED);
            }
        }

        private void UpdateObjectClassDTOWithUIValues()
        {
            if (isAddMode)
            {
                ObjectDTO.Name = ObjectClassName.StringValue;
                ObjectDTO.Description = String.IsNullOrWhiteSpace(ObjectClassDescription.StringValue) ? String.Empty : ObjectClassDescription.StringValue;
                ObjectDTO.SuperClass = ParentClass.StringValue;
                //optionalAttributesList.AddRange(parentOptionalAttributes);
                ObjectDTO.GovernsID = GovernsIDField.StringValue;
                ObjectDTO.May = optionalAttributesList;
                ObjectDTO.Must = mandatoryAttributesList;
				ObjectDTO.Aux = auxiliaryClassesList;
                ObjectDTO.ClassType = (ObjectClassDTO.ObjectClassType)((int)ClassTypePopup.IndexOfSelectedItem) + 1;
            }
            else
            {
                ObjectClassModDTO.Name = ObjectClassName.StringValue;
                if (String.Equals(ObjectDTO.Description, ObjectClassDescription.StringValue) != true)
                    ObjectClassModDTO.Description = ObjectClassDescription.StringValue;
                ObjectClassModDTO.May = optionalAttributesList;
                ObjectClassModDTO.Aux = auxiliaryClassesList;

            }
        }

        //set only editable object class fields to true.
        private void SetEditableFields()
        {
            ObjectClassDescription.Enabled = true;
            ObjectClassDescription.BecomeFirstResponder();
            OptionalAttributesListView.Enabled = true;
            AuxiliaryClassesListView.Enabled = true;
            AddAuxiliaryClassesButton.Enabled = true;
            AddOptionalAttributesButton.Enabled = true;
            AddMandatoryAttributesButton.Hidden = true;
            AddObjectClassButton.Hidden = true;
            RemoveAuxiliaryClassesButton.Hidden = true;
            RemoveMandatoryAttributesButton.Hidden = true;
            RemoveOptionalAttributesButton.Hidden = true;
        }

        partial void OnActionButton(Foundation.NSObject sender)
        {
            if (ActionButton.Title == VMIdentityConstants.EDIT)
            {
                ActionButton.Title = VMIdentityConstants.UPDATE;
                SetEditableFields();
            }
            else if (UIErrorHelper.ConfirmOperation(VMIdentityConstants.CONFIRM_MSG) == true)
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        DoValidateControls();
                        UpdateObjectClassDTOWithUIValues();
                        this.Close();
                        NSApplication.SharedApplication.StopModalWithCode(1);

                    });
            }
        }

        public new ObjectClassWindow Window
        {
            get { return (ObjectClassWindow)base.Window; }
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }
    }
}
