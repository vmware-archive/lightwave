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
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.DataSources;

namespace VMPscHighAvailabilitySnapIn.UI
{
    /// <summary>
    /// Service list view
    /// </summary>
    public class ServiceListView : MmcListView
    {
        private List<ServiceDto> DataSource { get; set; }

        /// <summary>
        /// View initialization
        /// </summary>
        /// <param name="status">Status</param>
        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);
            Columns[0].SetWidth(300);
            Columns[0].Title = Constants.ServiceTableColumnNameId;
            Columns.Add(new MmcListViewColumn(Constants.ServiceTableColumnPortId, 60));
            Columns.Add(new MmcListViewColumn(Constants.ServiceTableColumnStatusId, 80));
            Columns.Add(new MmcListViewColumn(Constants.ServiceTableColumnLastHeartbeatId, 120));
            Mode = MmcListViewMode.Report;
            if (ScopeNode is IServiceListViewDatasource)
            {
                DataSource = (ScopeNode as IServiceListViewDatasource).Services;
                FillListView();
            }
        }

        /// <summary>
        /// Fills list view
        /// </summary>
        void FillListView()
        {
            ResultNodes.Clear();

            if (DataSource != null)
            {
                foreach (var dto in DataSource)
                {
                    var resultNode = new ResultNode { DisplayName = dto.ServiceName };
                    resultNode.SubItemDisplayNames.Add(dto.Description);
                    resultNode.SubItemDisplayNames.Add(dto.Port.ToString());
                    var status = dto.Alive ? Constants.Active : Constants.InActive;
                    resultNode.SubItemDisplayNames.Add(status);
                    resultNode.SubItemDisplayNames.Add(dto.LastHeartbeat.ToString(Constants.DateFormat));
                    resultNode.ImageIndex = (int)(VMPSCHighAvailability.Common.ImageIndex.Service);
                    ResultNodes.Add(resultNode);
                }
            }
        }
    }
}
