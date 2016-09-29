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
using VMDir.Common.VMDirUtilities;
using VmIdentity.UI.Common.Utilities;
using VmIdentity.UI.Common;
using VMDir.Common.Schema;
using System.Linq;

namespace VMDirSchemaEditorSnapIn
{
    public partial class AttributeTypeWindowController : NSWindowController
    {
        private bool isAddMode = false;

        public  AttributeTypeDTO AttributeDTO { get; set; }

        public AttributeTypeDTO AttributeModDTO { get; set; }

        private string[] syntaxList;

        public AttributeTypeWindowController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public AttributeTypeWindowController(NSCoder coder)
            : base(coder)
        {
        }

        public AttributeTypeWindowController()
            : base("AttributeTypeWindow")
        {
            isAddMode = true;
            AttributeDTO = new AttributeTypeDTO();
        }

        public AttributeTypeWindowController(AttributeTypeDTO attrType)
            : base("AttributeTypeWindow")
        {
            isAddMode = false;
            AttributeDTO = attrType;
            AttributeModDTO = new AttributeTypeDTO();
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            InitialiseControls();
        }

        private void FillControlsWithAttributeData()
        {
            AttributeName.StringValue = AttributeDTO.Name;
            AttributeDescription.StringValue = AttributeDTO.Description;
            AttributeX500ID.StringValue = AttributeDTO.AttributeID;
            if (!AttributeDTO.SingleValue)
                MultiValuedCheckBox.State = NSCellStateValue.On;
            AttribtueSyntaxPopUp.SelectItem(Array.IndexOf(syntaxList, AttributeDTO.Type));
        }

        private void InitialiseControls()
        {
            syntaxList = VMDirCommonEnvironment.Instance.SyntaxDefs.SyntaxList.Select(x => x.Name).ToArray();
            this.AttribtueSyntaxPopUp.AddItems(syntaxList);
            if (!isAddMode)
            {
                FillControlsWithAttributeData();
                EnableUIControls(false);
                ActionButton.Title = VMIdentityConstants.EDIT;
            }

        }

        private void EnableUIControls(bool state)
        {
            this.AttributeName.Enabled = state;
            this.AttributeX500ID.Enabled = state;
            this.AttribtueSyntaxPopUp.Enabled = state;
            this.AttributeDescription.Enabled = state;
            this.MultiValuedCheckBox.Enabled = state;
        }

        private void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(AttributeName.StringValue) || string.IsNullOrWhiteSpace(AttribtueSyntaxPopUp.StringValue)
                || string.IsNullOrWhiteSpace(AttributeX500ID.StringValue))
                throw new Exception(VMIdentityConstants.EMPTY_FIELD);
        }

        private void FillDTOWithUIControls()
        {
            if (isAddMode)
            {
                AttributeDTO.Description = AttributeDescription.StringValue;
                AttributeDTO.Name = AttributeName.StringValue;
                AttributeDTO.LDAPDisplayName = AttributeName.StringValue;
                AttributeDTO.AttributeID = AttributeX500ID.StringValue;
                var syntax = VMDirCommonEnvironment.Instance.SyntaxDefs.LookupSyntaxByName(AttribtueSyntaxPopUp.TitleOfSelectedItem);
                if (syntax != null)
                    AttributeDTO.AttributeSyntax = syntax.Value;
                AttributeDTO.SingleValue = (MultiValuedCheckBox.State != NSCellStateValue.On);
            }
            else
            {
                AttributeModDTO.Name = AttributeDTO.Name;
                if (!AttributeDescription.StringValue.Equals(AttributeDTO.Description))
                    AttributeModDTO.Description = AttributeDescription.StringValue;
                if (AttributeDTO.SingleValue == true && MultiValuedCheckBox.State == NSCellStateValue.On)
                    AttributeModDTO.SingleValue = false;
                else
                    AttributeModDTO.SingleValue = AttributeDTO.SingleValue;
            }
        }

        private void SetUIToolsEditability()
        {
            AttributeDescription.Enabled = true;
            AttributeDescription.BecomeFirstResponder();
            if (MultiValuedCheckBox.State != NSCellStateValue.On)
                MultiValuedCheckBox.Enabled = true;
        }

        partial void OnClickActionButton(Foundation.NSObject sender)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    if (ActionButton.Title == VMIdentityConstants.EDIT)
                    {
                        SetUIToolsEditability();
                        ActionButton.Title = VMIdentityConstants.UPDATE;
                    }
                    else if (UIErrorHelper.ConfirmOperation(VMIdentityConstants.CONFIRM_MSG) == true)
                    {
                        DoValidateControls();
                        FillDTOWithUIControls();
                        this.Close();
                        NSApplication.SharedApplication.StopModalWithCode(VMIdentityConstants.DIALOGOK);
                    }
                });

        }

        public new AttributeTypeWindow Window
        {
            get { return (AttributeTypeWindow)base.Window; }
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }
    }
}
