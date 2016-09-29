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
using System.Linq;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.ScopeNodes;
using VMPscHighAvailabilitySnapIn.SnapIn;
using VMPscHighAvailabilitySnapIn.Utils;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;
using VMPSCHighAvailability.Common.Helpers;

namespace VMPscHighAvailabilitySnapIn.UI
{
    /// <summary>
    /// Login form
    /// </summary>
    public partial class Login : Form
    {
        /// <summary>
        /// Management Dto
        /// </summary>
        public ManagementDto ManagementDto { get; set; }

        /// <summary>
        /// Server dto
        /// </summary>
        private ServerDto _serverDTO;

        /// <summary>
        /// Server dto
        /// </summary>
        public ServerDto Server { get; private set;}

        /// <summary>
        /// Error message
        /// </summary>
        private string _error;

        /// <summary>
        /// Constructor
        /// </summary>
        public Login()
        {
            InitializeComponent();
            this.txtBindUPN.Text = "Administrator@" + MMCMiscUtil.GetBrandConfig(CommonConstants.TENANT);
        }

        /// <summary>
        /// Parameterized constructor
        /// </summary>
        /// <param name="_serverDTO">Server dto</param>
        public Login(ServerDto serverDTO)
        {
            InitializeComponent();
            _serverDTO = serverDTO;
        }

        /// <summary>
        /// Ok event handler
        /// </summary>
        /// <param name="sender">Server</param>
        /// <param name="e">args</param>
        private void btnOK_Click(object sender, EventArgs e)
        {
            OnOkClick();
        }

        private async Task OnOkClick()
        {
            try
            {
                btnOK.Enabled = false;
                if (IsValid())
                {
                    if (Server == null)
                        Server = new ServerDto();
                    Server.Server = txtDirectoryServer.Text;
                    Server.Upn = txtBindUPN.Text;
                    Server.Password = txtPassword.Text;
                    var parts = Server.Upn.Split('@');
                    Server.UserName = parts[0];
                    Server.DomainName = parts[1];
                    var task = new Task(Connect);
                    task.Start();
                    if (task == await Task.WhenAny(task, Task.Delay(Constants.TopologyTimeout * Constants.MilliSecsMultiplier)))
                    {
                        await task;
                    }

                    if (ManagementDto != null)
                    {
                        this.DialogResult = DialogResult.OK;
                    }
                    else
                    {
                        MiscUtilsService.ShowError(Constants.LoginError, "Error", MessageBoxButtons.OK);
                        btnOK.Enabled = true;
                    }
                }
                else
                    btnOK.Enabled = true;
            }
            catch (AggregateException exc)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
                if (exc.InnerExceptions.Count > 0)
                {
                    var msg = exc.InnerExceptions.Select(x => x.Message).Aggregate((x, y) => x + " , " + y);
                    MMCDlgHelper.ShowError(msg);
                }
            }
            catch (Exception exp)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exp);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exp, custom);
                MiscUtilsService.ShowError(exp);
            }
        }

        public async void Connect()
        {
            ManagementDto = PscHighAvailabilityAppEnvironment.Instance.Service.Connect(Server);
        }
        private bool IsValid()
        {
            bool success = true;
            if (string.IsNullOrEmpty(txtDirectoryServer.Text) ||
                string.IsNullOrEmpty(txtBindUPN.Text) ||
                string.IsNullOrEmpty(txtPassword.Text))
            {
                MessageBox.Show("Server, UPN or password cannot be left empty");
                success = false;
            }
            else
            {
                if (!txtBindUPN.Text.Contains("@") ||
                    txtBindUPN.Text.Split('@').Length > 2)
                {
                    MessageBox.Show("Invalid UPN");
                    success = false;
                }
                else if (_serverDTO == null && DupCheck())
                {
                    var msg = string.Format("Server {0} already exists.", txtDirectoryServer.Text);
                    MessageBox.Show(msg);
                    success = false;
                }
            }
            return success;
        }

        private bool DupCheck()
        {
            var server = txtDirectoryServer.Text;
            var serverNodes = PscHighAvailabilityAppEnvironment.Instance.SnapIn.RootNode.Children;
            var isDuplicate = false;
            if(serverNodes != null)
            {
                foreach(ScopeNode node in serverNodes)
                {
                    var parts = node.DisplayName.Split('(');
                    if (parts[0].Trim() == server.Trim())
                    {
                        isDuplicate = true;
                        break;
                    }
                }
            }
            return isDuplicate;
        }

        /// <summary>
        /// Form load event handler
        /// </summary>
        /// <param name="sender">Sender</param>
        /// <param name="e">Args</param>
        private void Login_Load(object sender, EventArgs e)
        {
            if(_serverDTO != null)
            {
                txtDirectoryServer.Text = _serverDTO.Server;
                txtDirectoryServer.Enabled = false;
                txtBindUPN.Text = string.IsNullOrEmpty(_serverDTO.Upn) 
                    ? "Administrator@lightwave.local"
                    : _serverDTO.Upn;
            }
        }

        /// <summary>
        /// Cancel event handler
        /// </summary>
        /// <param name="sender">Sender</param>
        /// <param name="e">Args</param>
        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
        }
    }
}
