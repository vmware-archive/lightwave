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
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.DataSources;
using VMPscHighAvailabilitySnapIn.SnapIn;
using VMPscHighAvailabilitySnapIn.UI;
using VMPscHighAvailabilitySnapIn.Utils;
using c = VMPSCHighAvailability.Common;
using mmc = Microsoft.ManagementConsole;
using VMPSCHighAvailability.Common;
using System.Threading.Tasks;
using System;
using System.IO;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils.Log;
using VMPSCHighAvailability.Common.Helpers;
using VMIdentity.CommonUtils.Utilities;

namespace VMPscHighAvailabilitySnapIn.ScopeNodes
{
    /// <summary>
    /// Server node
    /// </summary>
    public class ServerNode : ScopeNode, IHostListViewDatasource
    {
        /// <summary>
        /// Server node actions
        /// </summary>
        private enum ServerNodeAction
        {
            Login = 1,
            Logout = 2
        }
        public ServerDto ServerDto { get; set; }
        public string ServerGUID { get; set; }

        private ManagementDto _dto;

        private Timer _timer;

        /// <summary>
        /// Server view user control
        /// </summary>
        public ServerView UserControl;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="dto">Server dto</param>
        public ServerNode(ServerDto dto)
        {
            ServerDto = dto;
            DisplayName = GetDisplayName(dto);
            ImageIndex = SelectedImageIndex = (int)VMPSCHighAvailability.Common.ImageIndex.Server;
            AddViewDescription();
            _timer = new Timer() { Enabled = false, Interval = Constants.CacheCycleRefreshInterval * Constants.MilliSecsMultiplier };
            _timer.Tick += _timer_Tick;
        }

        private string GetDisplayName(ServerDto dto)
        {
            var domainName = (string.IsNullOrEmpty(dto.DomainName)
                ? string.Empty
                : " (" + dto.DomainName + ")");
            return dto.Server + domainName;
        }

        void _timer_Tick(object sender, System.EventArgs e)
        {
            if (Hosts != null && Hosts.Count > 0)
            {
                var infras = Hosts.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure).ToList();
                if (infras != null)
                {
                    foreach (var host in infras)
                    {
                        Task.Run(() =>
                        {
                            try
                            {
                                UpdateInfraNode(host, ServerDto);
                            }
                            catch (Exception exc)
                            {
                                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
                            }
                        }
                            );
                    }
                }

                var mgmts = Hosts.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Management);

                if (mgmts != null)
                {
                    foreach (var host in mgmts)
                    {
                        Task.Run(() =>
                        {
                            try
                            {
                                UpdateManagementNode(host, ServerDto);
                            }
                            catch (Exception exc)
                            {
                                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
                            }
                        }
                            );
                    }
                }
            }
        }

        /// <summary>
        /// Action event handler
        /// </summary>
        /// <param name="action">Action</param>
        /// <param name="status">Status</param>
        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            try
            {
                base.OnAction(action, status);

                switch ((ServerNodeAction)(int)action.Tag)
                {
                    case ServerNodeAction.Login:
                        Login();
                        break;
                    case ServerNodeAction.Logout:
                        Logout();
                        break;
                }
            }
            catch (Exception exc)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
                MMCDlgHelper.ShowException(exc);
            }
        }

        /// <summary>
        /// On delete event handler
        /// </summary>
        /// <param name="status">Status</param>
        protected override void OnDelete(SyncStatus status)
        {
            try
            {
                base.OnDelete(status);
                StopBackgroundRefresh();
                RemoveServer();
            }
            catch (Exception exc)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
                MMCDlgHelper.ShowException(exc);
            }
        }

        /// <summary>
        /// On refresh event handler
        /// </summary>
        /// <param name="status">Status</param>
        protected override void OnRefresh(AsyncStatus status)
        {
            try
            {
                base.OnRefresh(status);
                RefreshTopology(_dto);
                RefreshNodeTree();
            }
            catch (AggregateException exc)
            {
                if (exc.InnerExceptions.Count > 0)
                {
                    var msg = exc.InnerExceptions.Select(x => x.Message).Aggregate((x, y) => x + " , " + y);
                    var message = string.Format(Constants.VMDirConnectFailure, msg);
                    MMCDlgHelper.ShowMessage(message);
                }
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
            }
            catch (Exception exc)
            {
                var custom = new CustomExceptionExtractor().GetCustomMessage(exc);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exc, custom);
                MMCDlgHelper.ShowException(exc);
            }
        }

        private void RefreshNodeTree()
        {
            UpdateNodes();
            if (this.UserControl != null)
                this.UserControl.LoadData();
        }

        /// <summary>
        /// Adds logout actions
        /// </summary>
        public void AddLogoutActions()
        {
            ActionsPaneItems.Clear();
            EnabledStandardVerbs = StandardVerbs.Delete | StandardVerbs.Refresh;
            var logoutAction = new mmc.Action(ServerNodeAction.Logout.ToString(), ServerNodeAction.Logout.ToString(), (int)VMPSCHighAvailability.Common.ImageIndex.Management, ServerNodeAction.Logout);
            ActionsPaneItems.Add(logoutAction);
        }

        /// <summary>
        /// Adds login actions
        /// </summary>
        public void AddLoginActions()
        {
            ActionsPaneItems.Clear();
            EnabledStandardVerbs = StandardVerbs.Delete;
            var logoutAction = new mmc.Action(ServerNodeAction.Login.ToString(), ServerNodeAction.Login.ToString(), (int)VMPSCHighAvailability.Common.ImageIndex.Management, ServerNodeAction.Login);
            ActionsPaneItems.Add(logoutAction);
        }

        /// <summary>
        /// Login form
        /// </summary>
        public void Login()
        {
            (this.Parent as RootNode).Login(this);
        }

        /// <summary>
        /// Set control states on logout
        /// </summary>
        public void Logout()
        {
            if (this.Children != null)
            {
                foreach (var child in this.Children)
                {
                    if (child is ManagementNode)
                    {
                        ((ManagementNode)child).Cleanup();
                    }
                }
                this.Children.Clear();
            }


            if (Hosts != null)
                Hosts.Clear();

            if (this.UserControl != null)
                this.UserControl.LoadData(false);

            if (ServerDto != null)
            {
                ServerDto.Password = null;
                ServerDto.UserName = null;
                ServerDto.Upn = null;
            }

            if (_timer != null)
            {
                _timer.Tick -= _timer_Tick;
                _timer.Dispose();
                _timer = null;
            }
            AddLoginActions();
        }

        /// <summary>
        /// Removes the server from the node tree
        /// </summary>
        void RemoveServer()
        {
            var result = MMCDlgHelper.ShowConfirm(Constants.DeleteServer + "?");
            if (result)
            {
                if (PscHighAvailabilityAppEnvironment.Instance.LocalData.SerializableList.Contains(ServerDto.Server))
                {
                    PscHighAvailabilityAppEnvironment.Instance.LocalData.SerializableList.Remove(ServerDto.Server);
                }

                var parent = this.Parent as RootNode;

                if (parent != null)
                    parent.Children.Remove(this);
                else
                    MMCDlgHelper.ShowMessage(Constants.CannotDeleteTheRootNode);
            }
        }

        /// <summary>
        /// Hosts data
        /// </summary>
        public List<NodeDto> Hosts
        {
            get;
            set;
        }

        /// <summary>
        /// Add view description for the node
        /// </summary>
        void AddViewDescription()
        {
            var fvd = new FormViewDescription
            {
                DisplayName = Constants.PscTableColumnNameId,
                ViewType = typeof(ServerFormView),
                ControlType = typeof(ServerView)
            };
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }

        /// <summary>
        /// Refreshs the topology.
        /// </summary>
        /// <param name="refresh">If set to <c>true</c> refresh.</param>
        public void RefreshTopology(ManagementDto dto, bool refresh = false)
        {
            var service = PscHighAvailabilityAppEnvironment.Instance.Service;

            // 0. For subsequent refresh fetch the affinitized dc for the management dto from the cache
            if (Hosts != null)
            {
                var mgmtDto = Hosts.FirstOrDefault(x => x.Name == dto.Name || x.Ip == dto.Name) as ManagementDto;

                if (mgmtDto != null)
                    dto.DomainController = mgmtDto.DomainController;
            }

            // 1. Fetech all the nodes
            var nodes = service.GetTopology(dto, ServerDto);

            if (nodes == null)
            {
                Hosts = new List<NodeDto>();
                return;
            }

            // 2. Update cached hosts list
            Hosts = new List<NodeDto>(nodes);

            // 3. Set infrastructure node status
            foreach (var host in nodes.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure))
            {
                host.Active = true;
                var task = new Task(() => { UpdateInfraNode(host, ServerDto); });
                task.Start();
            }

            // 4. Set management nodes to Active
            foreach (var host in nodes.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Management))
            {
                host.Active = true;
                var task = new Task(() => { UpdateManagementNode(host, ServerDto); });
                task.Start();
            }
        }

        private void UpdateManagementNode(NodeDto host, ServerDto serverDto)
        {
            var message = string.Format("Method: UpdateManagementNode refresh cache cycle start for Server: {0}", host.Name);
            var logger = PscHighAvailabilityAppEnvironment.Instance.Logger;
            logger.Log(message, LogLevel.Info);
            var service = PscHighAvailabilityAppEnvironment.Instance.Service;
            var server = new ServerDto
            {
                Server = host.Name,
                Upn = serverDto.Upn,
                UserName = serverDto.UserName,
                Password = serverDto.Password,
                DomainName = serverDto.DomainName
            };
            var dto = service.GetManagementNodeDetails(server);
            dto.Name = server.Server;
            dto.Domain = server.DomainName;
            dto.Ip = Network.GetIpAddress(dto.Name);
            message = string.Format("Method: UpdateManagementNode - for Server: {0}", host.Name);
            logger.Log(message, LogLevel.Info);
            var index = Hosts.FindIndex(x => x.Name == dto.Name);
            if (index > -1 && index < Hosts.Count)
            {
                Hosts[index] = dto;

                message = string.Format("Method: UpdateManagementNode updated VC Server: {0}", host.Name);
                logger.Log(message, LogLevel.Info);
            }
            message = string.Format("Method: UpdateManagementNode refresh cache cycle end for Server: {0}", host.Name);
            logger.Log(message, LogLevel.Info);
        }

        private void UpdateInfraNode(NodeDto host, ServerDto serverDto)
        {
            var service = PscHighAvailabilityAppEnvironment.Instance.Service;
            var server = new ServerDto
            {
                Server = host.Name,
                Upn = serverDto.Upn,
                UserName = serverDto.UserName,
                Password = serverDto.Password
            };
            var dto = service.UpdateStatus(host, server);
            dto.Ip = Network.GetIpAddress(dto.Name);
            var index = Hosts.FindIndex(x => x.Name == dto.Name);
            if (index > -1 && index < Hosts.Count)
                Hosts[index] = dto;
        }

        /// <summary>
        /// Updates the nodes.
        /// </summary>
        public void UpdateNodes()
        {
            // 1. Clear existing child nodes.
            this.Children.Clear();

            // 2. Group all the nodes by sitename
            var groupedHosts = Hosts.GroupBy(x => x.Sitename);

            foreach (var siteGroup in groupedHosts)
            {
                // 3. Add a site node.
                var siteNode = new SiteNode(siteGroup.Key);
                this.Children.Add(siteNode);

                // 6. Add the leaf infrastucture nodes.
                var infraDtos = siteGroup
                            .Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure)
                            .ToList();

                var infraNodes = infraDtos
                            .Select(x => new InfrastructureNode(x.Name, ServerDto))
                            .ToList();

                var infraExists = infraDtos.Count() > 0;

                // 4. Add Infrastructures Node node.
                var infrasNode = new InfrastructuresNode(!infraExists);

                if (infraExists)
                {
                    foreach (var node in infraNodes)
                    {
                        infrasNode.Children.Add(node);
                    }
                }

                // 7. Add the leaf management nodes.
                var mgmtDtos = siteGroup
                            .Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Management)
                            .ToList();

                var mgmtNodes = mgmtDtos
                            .Select(x => new ManagementNode(x.Name, ServerDto))
                            .ToList();

                var mgmtExists = mgmtNodes.Count() > 0;

                // 5. Add Managements Node node.
                var mgmtsNode = new ManagementsNode(!mgmtExists);

                if (mgmtExists)
                {
                    foreach (var node in mgmtNodes)
                    {
                        mgmtsNode.Children.Add(node);
                    }
                }

                // 8. Add the nodes to the Tree
                siteNode.Children.Add(mgmtsNode);
                siteNode.Children.Add(infrasNode);
            }
        }

        /// <summary>
        /// Filters the list of nodes.
        /// </summary>
        /// <returns>The filtered list.</returns>
        /// <param name="type">Type.</param>
        public List<NodeDto> FilterBy(NodeType type)
        {
            return Hosts.Where(x => x.NodeType == type).ToList();
        }

        /// <summary>
        /// Filters the list of nodes.
        /// </summary>
        /// <returns>The filtered list.</returns>
        /// <param name="type">Type.</param>
        public List<InfrastructureDto> FilterInfrastructureNodes()
        {
            return Hosts.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure).Select(x => (InfrastructureDto)x).ToList();
        }

        /// <summary>
        /// Filters the list of nodes.
        /// </summary>
        /// <returns>The filtered list.</returns>
        /// <param name="type">Type.</param>
        public List<ManagementDto> FilterManagementNodes()
        {
            return Hosts.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Management).Select(x => (ManagementDto)x).ToList();
        }
        /// <summary>
        /// Filters the list of nodes.
        /// </summary>
        /// <returns>The filtered list.</returns>
        /// <param name="type">Type.</param>
        /// <param name="siteName">Site name.</param>
        public List<NodeDto> FilterBy(NodeType type, string siteName)
        {
            return Hosts.Where(x => x.NodeType == type && x.Sitename == siteName).ToList();
        }


        /// <summary>
        /// Filters the list of nodes.
        /// </summary>
        /// <returns>The filtered list.</returns>
        /// <param name="siteName">Site name.</param>
        public List<NodeDto> FilterBy(string siteName)
        {
            return Hosts.Where(x => x.Sitename == siteName).ToList();
        }

        public void Load(ManagementDto dto)
        {
            _dto = dto;
            RefreshTopology(dto);
            UpdateNodes();
            StartBackgroundRefresh();

            if (this.UserControl != null)
                this.UserControl.LoadData();
        }

        public void StartBackgroundRefresh()
        {
            if (_timer != null)
                _timer.Enabled = true;
        }

        public void StopBackgroundRefresh()
        {
            if (_timer != null)
                _timer.Enabled = false;
        }
    }
}
