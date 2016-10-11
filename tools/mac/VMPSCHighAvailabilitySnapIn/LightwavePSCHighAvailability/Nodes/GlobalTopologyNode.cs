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
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using VMPSCHighAvailability.Common.DTO;
using VmIdentity.UI.Common.Utilities;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.Service;
using VMIdentity.CommonUtils.Utilities;

namespace VMPSCHighAvailability.Nodes
{
	public delegate void CacheRefreshEventHandler(object sender, EventArgs e);

	public class GlobalTopologyNode : ScopeNode
	{
		private ServerDto _serverDto;
		public ManagementDto _managementDto;
		private IPscHighAvailabilityService _service;
		public List<NodeDto> Hosts { get; private set;}
		public event CacheRefreshEventHandler OnCacheRefresh;
		public bool IsConnected;

		/// <summary>
		/// Timer for auto-refresh.
		/// </summary>
		private Timer timer;

		public GlobalTopologyNode (IPscHighAvailabilityService service, ServerDto serverDto, ManagementDto managementDto)
		{
			DisplayName = serverDto.DomainName;
			_managementDto = managementDto;
			_serverDto = serverDto;
			_service = service;
			Hosts = new List<NodeDto> ();
			IsConnected = true;
			NetworkChange.NetworkAvailabilityChanged += (object sender, NetworkAvailabilityEventArgs e) => {
				IsConnected = e.IsAvailable;
			};
			RefreshTopology ();
			UpdateNodes ();
			var interval = Constants.CacheCycleRefreshInterval * Constants.MilliSecsMultiplier;
			timer = new Timer (timerAutoRefresh_Tick, null, 0, interval);

		}

		/// <summary>
		/// Timers the auto refresh tick.
		/// </summary>
		/// <param name="state">State.</param>
		private void timerAutoRefresh_Tick(Object state)
		{
			if (Hosts != null) {

				if (IsConnected) {
					var tasks = new Task[Hosts.Count];
					int count = 0;
					foreach (var host in Hosts.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure)) {
						tasks [count++] = Task.Factory.StartNew (() => UpdateInfraNode (host, _serverDto));
					}

					foreach (var host in Hosts.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Management)) {
						tasks [count++] = Task.Factory.StartNew (() => UpdateManagementNode (host, _serverDto));
					}
				}
			}
		}

		/// <summary>
		/// Updates the management node.
		/// </summary>
		/// <param name="host">Host.</param>
		/// <param name="serverDto">Server dto.</param>
		public void UpdateManagementNode(NodeDto host, ServerDto serverDto)
		{
			var server = new ServerDto
			{
				Server = host.Name,
				Upn = serverDto.Upn,
				UserName = serverDto.UserName,
				Password = serverDto.Password,
				DomainName = serverDto.DomainName
			};
			var dto = _service.GetManagementNodeDetails(server);
			dto.Name = server.Server;
			dto.Domain = server.DomainName;
			dto.Ip =  Network.GetIpAddress (dto.Name);
			var index = Hosts.FindIndex (x => x.Name == dto.Name);
			if(index < Hosts.Count)
				Hosts[index] = dto;
			if (OnCacheRefresh != null)
				OnCacheRefresh (this, EventArgs.Empty);
		}

		/// <summary>
		/// Updates the infra node.
		/// </summary>
		/// <param name="host">Host.</param>
		/// <param name="serverDto">Server dto.</param>
		private void UpdateInfraNode(NodeDto host, ServerDto serverDto)
		{
			Console.WriteLine ("UpdateInfraNode " +  host.Name + " Start: " + DateTime.Now.ToString ("hh:mm:ss t z"));
			var server = new ServerDto {
				Server = host.Name, 
				UserName = serverDto.UserName, 
				Upn = serverDto.Upn,
				Password = serverDto.Password 
			};
			var dto = _service.UpdateStatus(host, server);
			dto.Ip =  Network.GetIpAddress (dto.Name);
			var index = Hosts.FindIndex (x => x.Name == dto.Name);
			if(index > -1 && index < Hosts.Count)
				Hosts[index] = dto;
			if (OnCacheRefresh != null)
				OnCacheRefresh (this, EventArgs.Empty);
			Console.WriteLine ("UpdateInfraNode " +  host.Name + " Service: " + ((InfrastructureDto)host).Services.Count + " End: " + DateTime.Now.ToString ("hh:mm:ss t z"));
		}

		/// <summary>
		/// Refreshs the topology.
		/// </summary>
		/// <param name="refresh">If set to <c>true</c> refresh.</param>
		public void RefreshTopology (bool refresh = false)
		{
			if (IsConnected) {
				
				// 1. Fetech all the nodes
				var nodes = _service.GetTopology (_managementDto, _serverDto);

				if (!refresh) {

					// 2. Cached hosts list

					Hosts = nodes == null ? new List<NodeDto> () : new List<NodeDto> (nodes);

					// 3. Update infrastructure node
					foreach (var host in nodes.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure)) {
						host.Active = true;
						var task = new Task (() => {
							UpdateInfraNode (host, _serverDto);
						});
						task.Start ();
					}

					// 4. Update management nodes 
					foreach (var host in nodes.Where(x => x.NodeType == VMPSCHighAvailability.Common.NodeType.Management)) {
						host.Active = true;
						var task = new Task (() => {
							UpdateManagementNode (host, _serverDto);
						});
						task.Start ();
					}
				} else {

					var newNodes = nodes.Where (x => Hosts.Count (y => y.Name == x.Name) == 0);

					if (newNodes != null && newNodes.Count () > 0)
						Hosts.AddRange (newNodes);

					var deleteNodes = Hosts.Where (x => nodes.Count (y => y.Name == x.Name) == 0);

					if (deleteNodes != null) {
						foreach (var node in deleteNodes)
							Hosts.Remove (node);
					}
				}
			} else
				throw new Exception ("Please check your network connection. Network is not available to refresh the topology");
		}

		/// <summary>
		/// Updates the nodes.
		/// </summary>
		public void UpdateNodes ()
		{
			// 1. Clear existing child nodes.
			this.Children.Clear ();

			// 2. Group all the nodes by sitename
			var groupedHosts = Hosts.GroupBy (x => x.Sitename);

			foreach (var siteGroup in groupedHosts) {
				// 3. Add a site node.
				var siteNode = new SiteNode{ DisplayName = siteGroup.Key, Parent = this };

				// 4. Add Infrastructures Node node.
				var infrasNode = new InfrastucturesGroupNode (){ Parent = siteNode };

				// 5. Add Managements Node node.
				var mgmtsNode = new ManagementsGroupNode () { Parent = siteNode };

				// 6. Add the leaf infrastucture nodes.
				var infraNodes = siteGroup
							.Where (x => x.NodeType == NodeType.Infrastructure)
							.Select (x => new InfrastructureNode{ DisplayName = x.Name, Parent = infrasNode }).ToList ();

				if (infraNodes.Count () > 0) {
					infrasNode.Children.AddRange (infraNodes);
				}

				// 7. Add the leaf management nodes.
				var mgmtNodes = siteGroup
							.Where (x => x.NodeType == NodeType.Management)
							.Select (x => new ManagementNode{ DisplayName = x.Name, Parent = mgmtsNode }).ToList ();

				if (mgmtNodes.Count () > 0) {
					mgmtsNode.Children.AddRange (mgmtNodes);
				}

				// 8. Add the nodes to the Tree
				siteNode.Children.Add (mgmtsNode);
				siteNode.Children.Add (infrasNode);
				this.Children.Add (siteNode);
			}
		}

		/// <summary>
		/// Filters the list of nodes.
		/// </summary>
		/// <returns>The filtered list.</returns>
		/// <param name="type">Type.</param>
		public List<NodeDto> FilterBy(NodeType type)
		{
			return Hosts.Where (x => x.NodeType == type).ToList ();
		}

		/// <summary>
		/// Filters the list of nodes.
		/// </summary>
		/// <returns>The filtered list.</returns>
		/// <param name="type">Type.</param>
		public List<InfrastructureDto> FilterInfrastructureNodes(string siteName)
		{
			return Hosts
				.Where (x => x.NodeType == NodeType.Infrastructure && x.Sitename == siteName)
				.Select(x=> (InfrastructureDto)x)
				.ToList ();
		}

		/// <summary>
		/// Filters the list of nodes.
		/// </summary>
		/// <returns>The filtered list.</returns>
		/// <param name="type">Type.</param>
		public List<ManagementDto> FilterManagementNodes()
		{
			return Hosts
				.Where (x => x.NodeType  == NodeType.Management)
				.Select(x=> (ManagementDto)x).ToList ();
		}
		/// <summary>
		/// Filters the list of nodes.
		/// </summary>
		/// <returns>The filtered list.</returns>
		/// <param name="type">Type.</param>
		/// <param name="siteName">Site name.</param>
		public List<NodeDto> FilterBy(NodeType type, string siteName)
		{
			return Hosts
				.Where (x => x.NodeType == type && x.Sitename == siteName)
				.ToList ();
		}


		/// <summary>
		/// Filters the list of nodes.
		/// </summary>
		/// <returns>The filtered list.</returns>
		/// <param name="siteName">Site name.</param>
		public List<NodeDto> FilterBy(string siteName)
		{
			return Hosts.Where (x => x.Sitename == siteName).ToList ();
		}
	}
}

