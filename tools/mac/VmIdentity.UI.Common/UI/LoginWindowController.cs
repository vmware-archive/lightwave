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
using AppKit;
using Foundation;
using VmIdentity.UI.Common.Utilities;
using VMIdentity.CommonUtils;
using VMIdentity.CommonUtils.Utilities;

namespace VmIdentity.UI.Common
{
    public partial class LoginWindowController : AppKit.NSWindowController
    {
        public String UserName{ get; set; }

        public String Password { get; set; }

        public String DomainName { get; set; }

        public String[] ServerArray { get; set; }

        public string Server { get; set; }

        public String Upn { get; set; }

        #region Constructors

        // Called when created from unmanaged code
        public LoginWindowController(IntPtr handle)
            : base(handle)
        {
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public LoginWindowController(NSCoder coder)
            : base(coder)
        {
        }

        // Call to load from the XIB/NIB file
        public LoginWindowController()
            : base("LoginWindow")
        {
        }

        // Call to load from the XIB/NIB file
        public LoginWindowController(String[] server)
            : base("LoginWindow")
        {
            //TODO - instead of server array, good to pass ServerDTO with username info.
            this.ServerArray = server;
        }

        #endregion

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            foreach (var obj in ServerArray)
                this.ServerCombo.Add(NSObject.FromObject(obj));
            this.UserNameTxtField.StringValue = "Administrator@" + MiscUtil.GetBrandConfig(CommonConstants.TENANT);
            //Events
            this.OKButton.Activated += OnClickOKButton;
            this.CancelButton.Activated += OnClickCancelButton;

        }

        public void OnClickOKButton(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(UserNameTxtField.StringValue) || string.IsNullOrWhiteSpace(PasswordTxtField.StringValue) || string.IsNullOrWhiteSpace(ServerCombo.StringValue))
            {
                UIErrorHelper.ShowAlert("", "Please enter values in the host, username and password");
            }
            else
            {
                Upn = UserNameTxtField.StringValue;
                String[] userAndDomain = Upn.Split('@');
                if (userAndDomain.Length != 2 ||
                    string.IsNullOrWhiteSpace(userAndDomain[0]) ||
                    string.IsNullOrEmpty(userAndDomain[0]) ||
                    string.IsNullOrWhiteSpace(userAndDomain[1]) ||
                    string.IsNullOrEmpty(userAndDomain[1]))
                {
                    UIErrorHelper.ShowAlert("", "Enter Valid UPN");
                }
                else
                {
                    UserName = userAndDomain[0];
                    DomainName = userAndDomain[1];
                    Password = PasswordTxtField.StringValue;
                    Server = ServerCombo.StringValue;
                    if (!Network.IsValidIP(Server))
                    {
                        UIErrorHelper.ShowAlert("", "Enter Valid IP");
                    }
                    else
                    {
                        this.Close();
                        NSApplication.SharedApplication.StopModalWithCode(1);
                    }
                }
            }
        }

        public void OnClickCancelButton(object sender, EventArgs e)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(0);
        }

        //strongly typed window accessor
        public new LoginWindow Window
        {
            get
            {
                return (LoginWindow)base.Window;
            }
        }
    }
}

