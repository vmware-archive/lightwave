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
using System.Net.NetworkInformation;
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
	/// <summary>
	/// Monitor controller.
	/// </summary>
	public partial class MonitorController : AppKit.NSViewController, IServiceTableViewController
	{
		/// <summary>
		/// Tracks the auto refresh.
		/// </summary>
		private bool isAutoRefresh = false;

		/// <summary>
		/// The name of the site.
		/// </summary>
		public string SiteName = string.Empty;

		/// <summary>
		/// Timer for auto-refresh.
		/// </summary>
		private Timer timer;

		/// <summary>
		/// Tracks any exception from the background thread to propogate to the UI thread.
		/// </summary>
		private Exception exception;

		/// <summary>
		/// Tracks last refresh timestamp.
		/// </summary>
		private string lastRefreshTimestamp;

		/// <summary>
		/// The management dto.
		/// </summary>
		private ManagementDto _mgmtDto;

		/// <summary>
		/// The root node.
		/// </summary>
		public GlobalTopologyNode RootNode;

		/// <summary>
		/// Psc table column.
		/// </summary>
		private enum PscTableColumn
		{
			PscName = 0,
			Affinitized,
			Status,
			LastHeartBeat,
			//SiteName,
			None
		}

		/// <summary>
		/// Gets or sets the server dto.
		/// </summary>
		/// <value>The server dto.</value>
		public ServerDto ServerDto{ get; set; }

		/// <summary>
		/// Gets or sets the psc data source.
		/// </summary>
		/// <value>The psc data source.</value>
		public PscDataSource PscDataSource{ get; set; }

		/// <summary>
		/// The service.
		/// </summary>
		private IPscHighAvailabilityService _service;

		/// <summary>
		/// The is connected.
		/// </summary>
		private bool _isConnected;

		#region Constructors

		// Called when created from unmanaged code
		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.MonitorController"/> class.
		/// </summary>
		/// <param name="handle">Handle.</param>
		public MonitorController (IntPtr handle) : base (handle)
		{

		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.MonitorController"/> class.
		/// </summary>
		/// <param name="coder">Coder.</param>
		public MonitorController (NSCoder coder) : base (coder)
		{

		}

		// Call to load from the XIB/NIB file
		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.MonitorController"/> class.
		/// </summary>
		public MonitorController (IPscHighAvailabilityService service) : base ("Monitor", NSBundle.MainBundle)
		{
			_service = service;
		}

		/// <summary>
		/// Awakes from nib.
		/// </summary>
		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			NetworkChange.NetworkAvailabilityChanged += (object sender, NetworkAvailabilityEventArgs e) => {
				_isConnected = e.IsAvailable;
			};
			Initialize();
			RefreshState ();

		}

		/// <summary>
		/// Initialize this instance.
		/// </summary>
		private void Initialize()
		{
			HostnameHeader.StringValue = ServerDto.Server;
			IpAddressTextField.StringValue = Network.GetIpAddress (ServerDto.Server);
			SitenameTextField.StringValue = SiteName;

			ServicesTableView.AddColumn (NSTableColumnHelper.ToNSTableColumn (Constants.TableColumnIconId, string.Empty, true, 20));
			ServicesTableView.MoveColumn (ServicesTableView.ColumnCount - 1, 0);

			PscTableView.AddColumn (NSTableColumnHelper.ToNSTableColumn (Constants.TableColumnIconId, string.Empty, true, 20));
			PscTableView.MoveColumn (PscTableView.ColumnCount - 1, 0);

			isAutoRefresh = false;
			lastRefreshTimestamp = DateTime.Now.ToString (Constants.DateFormat);
			SiteAffinityButton.Activated += SiteAffinityButton_Activated;
			AutoRefreshButton.Activated += AutoRefreshButton_Activated;
			IntervalComboBox.SelectionChanged += IntervalComboBox_SelectionChanged;
			RefreshButton.Activated += RefreshState_Activated;
			timer = new Timer (timerAutoRefresh_Tick, null, -1, -1);
		}

		/// <summary>
		/// Intervals the combo box selection changed.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void IntervalComboBox_SelectionChanged (object sender, EventArgs e)
		{
			var interval = int.Parse(IntervalComboBox.SelectedValue.ToString()) * Constants.MilliSecsMultiplier;
			timer.Change (interval, interval);
		}

		/// <summary>
		/// Timers the auto refresh tick.
		/// </summary>
		/// <param name="state">State.</param>
		private void timerAutoRefresh_Tick(Object state)
		{
			InvokeOnMainThread(RefreshState);
		}

		/// <summary>
		/// Auto refresh button activated.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void AutoRefreshButton_Activated (object sender, EventArgs e)
		{
			if (isAutoRefresh) {
				// turn auto refresh off
				timer.Change (-1, -1);
			} else {
				// turn auto refresh on
				var interval = IntervalComboBox.IntValue * Constants.MilliSecsMultiplier;
				timer.Change (interval, interval);
			}
			isAutoRefresh = !isAutoRefresh;
			IntervalComboBox.Enabled = isAutoRefresh;
		}

		#endregion

		/// <summary>
		/// Gets the view.
		/// </summary>
		/// <value>The view.</value>
		public new Monitor View {
			get {
				return (Monitor)base.View;
			}
		}
		/// <summary>
		/// Site-affinity button activated.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void SiteAffinityButton_Activated (object sender, EventArgs e)
		{
			try
			{
			var mode = _mgmtDto.Legacy ? Constants.HA : Constants.Legacy;
			var result = UIErrorHelper.ConfirmDeleteOperation (Constants.ModeChange + mode);
			if (result) {
				var change = !_mgmtDto.Legacy;
				_service.SetLegacyMode (change, ServerDto);
				RootNode.UpdateManagementNode (_mgmtDto, ServerDto);
				RefreshState ();
			}
			}
			catch (Exception exc) {
				UIErrorHelper.ShowAlert ("", exc.Message);
			}
		}

		/// <summary>
		/// Refreshs the state activated.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void RefreshState_Activated(object sender, EventArgs e)
		{
			RefreshState();
		}

		/// <summary>
		/// Refreshes the psc table view.
		/// </summary>
		void RefreshPscTableView ()
		{
			var serverDto = new ServerDto{
				Server = _mgmtDto.Name, 
				UserName = ServerDto.UserName, 
				Upn = ServerDto.Upn, 
				Password = ServerDto.Password 
			};

			if (_isConnected) {
				//var mgmtDto = _service.GetManagementNodeDetails (serverDto);
				var mgmtDto = RootNode.Hosts.FirstOrDefault(x => x.Sitename == _mgmtDto.Sitename && x.Name == _mgmtDto.Name) as ManagementDto;

				if (mgmtDto != null) {
					var infraNodes = FilterBySiteName (mgmtDto.DomainControllers);
					if (_mgmtDto.DomainController != null) {
						DomainControllerTextField.StringValue = _mgmtDto.DomainController.Name;

						foreach (var node in infraNodes) {
							node.IsAffinitized = (node.Name == _mgmtDto.DomainController.Name || node.Ip == _mgmtDto.DomainController.Ip);
						}
					}
					PscTableView.Delegate = new MonitorTableViewDelegate (this);
					PscDataSource = new PscDataSource (infraNodes);
					PscTableView.DataSource = PscDataSource;
					PscTableView.ReloadData ();

					if (infraNodes != null && infraNodes.Count > 0 && PscTableView.SelectedRowCount <= 0) {
						PscTableView.SelectRow (0, true);
					}

					if (_mgmtDto.State != null) {
						Health health = CdcDcStateHelper.GetHealth (_mgmtDto.State, infraNodes);
						var healthText = health.ToString ().ToUpper();
						CurrentStatusTextField.StringValue = healthText;
						CurrentStatusTextField.TextColor = GetHealthColor (health);
						var healthDesc = CdcDcStateHelper.GetHealthDescription (health);
						CurrentStatusTextField.ToolTip = healthDesc;
					}
					SiteAffinityButton.Title = "Enable " + (_mgmtDto.Legacy ? Constants.HA : Constants.Legacy);
					LegacyModeWarning.Hidden = !_mgmtDto.Legacy;
					LoadServices ();
				}
			}
		}

		private List<InfrastructureDto> FilterBySiteName(List<InfrastructureDto> allDcs)
		{
			if(allDcs != null)
			{
				foreach(var dc in allDcs)
				{	
					dc.IsRemote = !RootNode.Hosts.Exists (x => x.Sitename == _mgmtDto.Sitename && (x.Name == dc.Name || x.Ip == dc.Ip));
				}
			}
			return allDcs;
		}

		/// <summary>
		/// Sets the services table view.
		/// </summary>
		/// <param name="services">Services.</param>
		public void SetServicesTableView(List<ServiceDto> services)
		{
			ServicesTableView.Delegate = new ServicesTableViewDelegate (this);
			ServicesTableView.DataSource = new ServiceDataSource (services);
			ServicesTableView.ReloadData ();
		}

		/// <summary>
		/// Getsservice from selected row in the source.
		/// </summary>
		/// <param name="services">Service.</param>
		public ServiceDto GetService(int row)
		{
			var datasource = ServicesTableView.DataSource as ServiceDataSource;
			return datasource.Entries [row];
		}

		/// <summary>
		/// Loads the services.
		/// </summary>
		/// <param name="_controller">Controller.</param>
		public void LoadServices()
		{
			var source = PscTableView.DataSource as PscDataSource;
			var row = (int)PscTableView.SelectedRow;
			if (row > -1 && row < source.Entries.Count) {
				var node = source.Entries [row];
				ServicesheaderTextField.StringValue = "Services hosted on " + node.Name;
				var services = node.Services;
				SetServicesTableView (services);
			}
		}

		/// <summary>
		/// Shows the error.
		/// </summary>
		private void ShowError()
		{
			UIErrorHelper.ShowAlert (exception.Message, Constants.RefreshFailure);
		}

		/// <summary>
		/// Sets the host state and status controls.
		/// </summary>
		void SetHostStateAndStatusControls ()
		{
			if(_mgmtDto.State != null)
				StatusTextField.StringValue = _mgmtDto.State.Description;
		}

		/// <summary>
		/// Sets the auto refresh controls.
		/// </summary>
		void SetAutoRefreshControls ()
		{
			AutoRefreshButton.StringValue = isAutoRefresh ? "1" : "0";
			IntervalComboBox.Enabled = isAutoRefresh;
		}

		/// <summary>
		/// Posts refresh updates the state of the UI elements.
		/// </summary>
		private void RefreshState()
		{
			var mgmtDto = (ManagementDto)RootNode.Hosts.FirstOrDefault (x => x.Name == ServerDto.Server);

			if (mgmtDto != null) {
				_mgmtDto = mgmtDto;
				LastRefreshTextField.StringValue = DateTime.Now.ToString (Constants.DateFormat);
				SetHostStateAndStatusControls ();
				SetAutoRefreshControls ();
				RefreshPscTableView ();
			}
		}

		/// <summary>
		/// Gets the color of the health.
		/// </summary>
		/// <returns>The health color.</returns>
		/// <param name="health">Health.</param>
		public static NSColor GetHealthColor(Health health)
		{
			NSColor color;
			switch(health)
			{
			case Health.Full:
				color = NSColor.FromSrgb((nfloat)3.0/255,(nfloat)161/255,(nfloat)27/255,1);
				break;
			case Health.Limited:
				color = NSColor.Orange;
				break;
			case Health.Down:
				color = NSColor.Red;
				break;
			case Health.Legacy:
				color = NSColor.Black;
				break;
			default:
				color = NSColor.Purple;
				break;
			}

			return color;
		}
	}
}
