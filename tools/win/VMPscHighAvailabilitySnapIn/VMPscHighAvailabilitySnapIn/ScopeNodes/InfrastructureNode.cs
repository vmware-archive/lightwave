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
using VMAFD.Client;
using VMIdentity.CommonUtils;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.DataSources;
using VMPscHighAvailabilitySnapIn.SnapIn;
using VMPscHighAvailabilitySnapIn.UI;

namespace VMPscHighAvailabilitySnapIn.ScopeNodes
{
    /// <summary>
    /// Infrasturcture node
    /// </summary>
    public class InfrastructureNode : ScopeNode, IServiceListViewDatasource
    {
        public List<ServiceDto> Services { get; set; }
        public InfrastructureDto InfrastructureDto
        {
            get
            {
                var serverNode = GetServerNode();
                var siteName = GetSiteName();
                return serverNode.Hosts
                    .First(x => x.Sitename == siteName
                        && x.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure &&
                        x.Name == DisplayName) as InfrastructureDto;
            }
        }
        
        private ServerDto _serverDto;

        /// <summary>
        /// Infrastructure view user control
        /// </summary>
        public InfrastructureView UserControl;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="dto">Psc Dto</param>
        /// <param name="serverDto">Server Dto</param>
        public InfrastructureNode(string name, ServerDto serverDto)
            : base(true)
        {
            _serverDto = serverDto;
            DisplayName = name;
            ImageIndex = SelectedImageIndex = (int)VMPSCHighAvailability.Common.ImageIndex.Infrastructure;
            AddViewDescription();
        }

        /// <summary>
        /// Converts the info collection to the Service Dto collection
        /// </summary>
        /// <param name="info">VMAFD_HB_INFO collection</param>
        /// <returns>ServiceDto collection</returns>
        private List<ServiceDto> ConvertToServices(IList<VMAFD_HB_INFO> info)
        {
            return info.Select(x =>
                                   new ServiceDto
                                   {
                                       ServiceName = x.pszServiceName,
                                       Port = x.dwPort,
                                       Alive = x.bIsAlive == 1,
                                       LastHeartbeat = DateTimeConverter.FromUnixToDateTime(x.dwLastHeartbeat)
                                   }).ToList();
        }

        /// <summary>
        /// Add view description for the node
        /// </summary>
        void AddViewDescription()
        {
            var fvd = new FormViewDescription
            {
                DisplayName = Constants.PscTableColumnNameId,
                ViewType = typeof(InfrastructureFormView),
                ControlType = typeof(InfrastructureView)
            };
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }

        /// <summary>
        /// Gets the root node
        /// </summary>
        /// <returns>Root node</returns>
        public ServerNode GetServerNode()
        {
            return this.Parent.Parent.Parent as ServerNode;
        }

        /// <summary>
        /// Gets the site name
        /// </summary>
        /// <returns>Site name</returns>
        public string GetSiteName()
        {
            return this.Parent.Parent.DisplayName;
        }
    }
}
