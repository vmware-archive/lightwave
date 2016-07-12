/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using AppKit;
using Foundation;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;
using VmIdentity.UI.Common;

namespace VMDirSchemaEditorSnapIn.UI
{
    public partial class ConnectToLdapWindowController : AppKit.NSWindowController
    {
        VMDirServerDTO _dto;

        public VMDirServerDTO ServerDTO { get { return _dto; } }

        #region Constructors

        // Called when created from unmanaged code
        public ConnectToLdapWindowController(IntPtr handle)
            : base(handle)
        {
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public ConnectToLdapWindowController(NSCoder coder)
            : base(coder)
        {
        }
        
        // Call to load from the XIB/NIB file
        public ConnectToLdapWindowController(VMDirServerDTO dto)
            : base("ConnectToLdapWindow")
        {
            Initialize(dto);
        }
        
        // Shared initialization code
        void Initialize(VMDirServerDTO dto)
        {
            _dto = dto;
        }

        #endregion

        public override void AwakeFromNib()
        {
            base.AwakeFromNib(); 

            PrePopulateFields();
            AddEventListeners();
        }

        private void PrePopulateFields()
        {
            BindDN.StringValue = VMIdentityConstants.BINDUPN_VSPHERE;
        }

        private void AddEventListeners()
        {
            OKButton.Activated += OnClickOKButton;
            CancelButton.Activated += OnClickCancelButton;
        }

        //Event Handlers
        private void OnClickCancelButton(object sender, EventArgs e)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(0);
        }

        private void OnClickOKButton(object sender, EventArgs e)
        {
            FillDtoFromUIFields();

            UIErrorHelper.CheckedExec(delegate()
                {
                    ValidateDto();
                    this.Close();
                    NSApplication.SharedApplication.StopModalWithCode(VmIdentity.UI.Common.VMIdentityConstants.DIALOGOK);
                });
        }

        private void ValidateDto()
        {
            if (string.IsNullOrWhiteSpace(_dto.BindDN) || string.IsNullOrWhiteSpace(_dto.Server) || string.IsNullOrWhiteSpace(_dto.Password))
                throw new Exception(VMIdentityConstants.EMPTY_FIELD);
        }

        private void FillDtoFromUIFields()
        {
            _dto.BindDN = BindDN.StringValue;
            _dto.Server = ServerName.StringValue;
            _dto.Password = Password.StringValue;
        }

        //strongly typed window accessor
        public new ConnectToLdapWindow Window
        {
            get
            {
                return (ConnectToLdapWindow)base.Window;
            }
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }
    }
}

