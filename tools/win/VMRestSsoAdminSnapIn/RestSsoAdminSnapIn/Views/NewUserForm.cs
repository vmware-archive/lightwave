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
using System.IO;
using System.Net;
using System.Net.Mail;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewUserForm : Form, IView
    {
        private UserDto _userDto;
        private bool shouldClose = true;
        private string _systemDomain;
        private string _domainName;
        private ServerDto _serverDto;
        private string _tenantName;
        public NewUserForm(string systemDomain, string domainName, ServerDto serverDto, string tenantName)
        {
            InitializeComponent();
            _systemDomain = systemDomain;
            _domainName = domainName;
            _serverDto = serverDto;
            _tenantName = tenantName;
        }

        private bool ValidateControls()
        {
            if (string.IsNullOrEmpty(txtUserName.Text.Trim()))
            {
                MMCDlgHelper.ShowWarning(MMCUIConstants.USERNAME_ENT);
                return false;
            } else 
            if (string.IsNullOrEmpty(txtPassword.Text.Trim()))
            {
                MMCDlgHelper.ShowWarning(MMCUIConstants.PASSWORD_ENT);
                return false;
            } else 
            if (txtPassword.Text != txtConfirmPass.Text)
            {
                MMCDlgHelper.ShowWarning("Password and confirm do not match");
                return false;
            }
            else if (!string.IsNullOrEmpty(txtEmail.Text.Trim()))
            {
                try
                {
                    var m = new MailAddress(txtEmail.Text.Trim());                    
                }
                catch (FormatException)
                {
                    MMCDlgHelper.ShowWarning("Enter a valid email");
                    return false;
                }
            }
            return true;
        }

        private void ViewToDataContext()
        {
            _userDto = new UserDto
            {
                PersonDetails = new PersonUserDto
                {
                    FirstName = txtFirstName.Text,
                    LastName = txtLastName.Text,
                    EmailAddress = txtEmail.Text,
                    Description = txtDescription.Text,

                },
                PasswordDetails = new PasswordDetailsDto { Password = txtPassword.Text },
                Name = txtUserName.Text
            };
            _userDto.Domain = _systemDomain;
            _userDto.Alias = new PrincipalDto { Name = _userDto.Name, Domain = _domainName };
            _userDto.Disabled = chkDisabled.Checked;
            _userDto.PersonDetails.UserPrincipalName = string.Format("{0}@{1}", _userDto.Name, _userDto.Domain);
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            shouldClose = false;
            if (ValidateControls())
            {
                ViewToDataContext();              
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
                ActionHelper.Execute(delegate
                {
                    try
                    {
                        var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                        _userDto = service.User.Create(_serverDto, _tenantName, _userDto, auth.Token);
                        shouldClose = true;
                        this.DialogResult = DialogResult.OK;
                        Close();
                    }
                    catch (WebException exp)
                    {
                        if (exp.Response is HttpWebResponse)
                        {
                            var response = exp.Response as HttpWebResponse;
                            if (response != null && response.StatusCode == HttpStatusCode.BadRequest && response.ContentType == "application/json")
                            {
                                var resp = new StreamReader(response.GetResponseStream()).ReadToEnd();
                                var error = JsonConvert.Deserialize<AuthErrorDto>(resp);
                                if (error.Cause == "Constraint violation")
                                {
                                    MMCDlgHelper.ShowError("Password does not match the password policy set on the tenant. Check tenant configuration for password policy or contact administrator");
                                }
                                else
                                {
                                    MMCDlgHelper.ShowError(error.Details + " Cause - " + error.Cause);
                                }
                            }
                            else
                            {
                                throw exp;
                            }
                        }
                    }  

                }, auth);
            }   
        }
        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            base.OnClosing(e);
            e.Cancel = !shouldClose;
            shouldClose = true;
        }
        private void btnClose_Click(object sender, EventArgs e)
        {
            _userDto = null;
            shouldClose = true;
            Close();
        }

        public IDataContext DataContext
        {
            get { return _userDto; }
        }
    }
}
