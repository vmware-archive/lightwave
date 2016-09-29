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
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using AppKit;
using Foundation;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using VMIdentity.CommonUtils;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.DataSources;
using VMPSCHighAvailability.Common.Helpers;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.Service;
using VMPSCHighAvailability.Nodes;
using VMIdentity.CommonUtils.Utilities;

namespace VMPSCHighAvailability.UI
{
	public partial class InfrastructureController : AppKit.NSViewController, IServiceTableViewController
	{
		/// <summary>
		/// The infrastructure dto.
		/// </summary>
		private InfrastructureDto _infrastructureDto;


		/// <summary>
		/// The root node.
		/// </summary>
		private GlobalTopologyNode _rootNode;

		/// <summary>
		/// The name of the node.
		/// </summary>
		public string _nodeName = string.Empty;

		/// <summary>
		/// The name of the site.
		/// </summary>
		public string SiteName = string.Empty;

		#region Constructors

		// Called when created from unmanaged code
		public InfrastructureController (IntPtr handle) : base (handle)
		{	
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public InfrastructureController (NSCoder coder) : base (coder)
		{
		}

		// Call to load from the XIB/NIB file
		public InfrastructureController (string nodeName, GlobalTopologyNode rootNode) : base ("Infrastructure", NSBundle.MainBundle)
		{
			
			_rootNode = rootNode;
			_nodeName = nodeName;
		}

		#endregion

		//strongly typed view accessor
		public new Infrastructure View {
			get {
				return (Infrastructure)base.View;
			}
		}

		/// <summary>
		/// Awakes from nib.
		/// </summary>
		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			IpAddressTextField.StringValue = Network.GetIpAddress (_nodeName);
			_infrastructureDto = (InfrastructureDto) _rootNode.Hosts.First(x=>x.Name == _nodeName);
			HotnametextField.StringValue = _infrastructureDto.Name;
			SitenameTextField.StringValue = SiteName;
			var health = _infrastructureDto.Active ? Constants.Active : Constants.InActive;
			var color = _infrastructureDto.Active 
				? NSColor.FromSrgb((nfloat)3.0/255,(nfloat)161/255,(nfloat)27/255,1) 
				: NSColor.Red;
			HealthTextField.StringValue = health.ToUpper();
			HealthTextField.TextColor = color;
			SetServicesTableView (_infrastructureDto.Services);
		}

		/// <summary>
		/// Sets the services table view.
		/// </summary>
		/// <param name="services">Services.</param>
		public void SetServicesTableView(List<ServiceDto> services)
		{
			ServiceTableView.Delegate = new ServicesTableViewDelegate (this);
			ServiceTableView.DataSource = new ServiceDataSource (services);
			ServiceTableView.ReloadData ();
		}

		/// <summary>
		/// Getsservice from selected row in the source.
		/// </summary>
		/// <param name="services">Service.</param>
		public ServiceDto GetService(int row)
		{
			var datasource = ServiceTableView.DataSource as ServiceDataSource;

			if (datasource != null && datasource.Entries != null && row > -1 &&
			   row < datasource.Entries.Count) {
				return datasource.Entries [row];
			}
			return null;
		}
	}
}
