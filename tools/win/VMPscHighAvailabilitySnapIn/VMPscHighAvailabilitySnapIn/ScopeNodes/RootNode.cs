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
using System.Windows.Forms;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.SnapIn;
using VMPscHighAvailabilitySnapIn.UI;
using VMPscHighAvailabilitySnapIn.Utils;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;
using VMPSCHighAvailability.Common.Helpers;

namespace VMPscHighAvailabilitySnapIn.ScopeNodes
{
    /// <summary>
    /// Root node
    /// </summary>
    public class RootNode : ScopeNode
    {
        const int ConnectToServerAction = 1;
        private Login _loginUI;
        public string _name;

        /// <summary>
        /// Constructor
        /// </summary>
        public RootNode()
        {
            this.DisplayName = MMCMiscUtil.GetBrandConfig(CommonConstants.PSC_ROOT);
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(Constants.ConnectToServer,
                                       Constants.ConnectToServer, -1, ConnectToServerAction));
        }

        /// <summary>
        /// Refresh all the servers
        /// </summary>
        public void RefreshAll()
        {
            this.Children.Clear();
            var servers = PscHighAvailabilityAppEnvironment.Instance.LocalData.SerializableList;
            if (servers != null)
            {
                foreach (var severName in servers)
                {
                    var serverDto = new ServerDto { Server = severName };
                    var serverNode = new ServerNode(serverDto);
                    serverNode.AddLoginActions();
                    this.Children.Add(serverNode);
                }
            }
        }

        /// <summary>
        /// On Action event handler
        /// </summary>
        /// <param name="action">Action</param>
        /// <param name="status">Status</param>
        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ConnectToServerAction:
                    Login();
                    break;
            }
        }

        /// <summary>
        /// Login to server 
        /// </summary>
        /// <returns>True on success false otherwise</returns>
        public bool Login()
        {
			try
            {
				_loginUI = new Login();
                if (this.SnapIn.Console.ShowDialog(_loginUI) == DialogResult.OK)
                {
                    var node = new ServerNode(_loginUI.Server);
                    PscHighAvailabilityAppEnvironment.Instance.LocalData.AddServer(_loginUI.Server.Server);
                    node.AddLogoutActions();
                    this.Children.Add(node);
                    _loginUI.Close();
                    node.Load(_loginUI.ManagementDto);
                    return true;
                }
            }
            catch (AggregateException exc)
            {
                if (exc.InnerExceptions.Count > 0)
                {
                    var msg = exc.InnerExceptions.Select(x => x.Message).Aggregate((x, y) => x + " , " + y);
                    MMCDlgHelper.ShowMessage(msg);
                }
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
            }
            catch (Exception exp)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exp);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exp, custom);
                MMCDlgHelper.ShowException(exp);
            }
            finally
            {
                PscHighAvailabilityAppEnvironment.Instance.SaveLocalData();
            }
            return false;
        }

        /// <summary>
        /// Login into an already added server
        /// </summary>
        /// <param name="node">Server node</param>
        /// <returns>True on success false otherwise</returns>
        public bool Login(ServerNode node)
        {
            try
            {
                _loginUI = new Login(node.ServerDto);
                if (this.SnapIn.Console.ShowDialog(_loginUI) == DialogResult.OK)
                {
                    node.ServerDto = _loginUI.Server;
                    node.AddLogoutActions();
                    _loginUI.Close();
                    node.Load(_loginUI.ManagementDto);
                    return true;
                }
            }
            catch (AggregateException exc)
            {
                if (exc.InnerExceptions.Count > 0)
                {
                    var msg = exc.InnerExceptions.Select(x => x.Message).Aggregate((x, y) => x + " , " + y);
                    MMCDlgHelper.ShowMessage(msg);
                }
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
            }
            catch (Exception exp)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exp);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exp, custom);
                node.AddLoginActions();
                MMCDlgHelper.ShowException(exp);
            }
            return false;
        }
    }
}
