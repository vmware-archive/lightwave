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
using System.Text;
using AppKit;
using Foundation;
using VMIdentity.CommonUtils;
using VmIdentity.UI.Common.Utilities;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.UI;
using VMPSCHighAvailability.Common;

namespace VMPSCHighAvailability.DataSources
{
	/// <summary>
	/// Psc data source.
	/// </summary>
	public class PscDataSource : NSTableViewDataSource
	{
		/// <summary>
		/// Gets or sets the entries.
		/// </summary>
		/// <value>The entries.</value>
		public List<InfrastructureDto> Entries { get; private set; }

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.DataSources.PscDataSource"/> class.
		/// </summary>
		public PscDataSource (List<InfrastructureDto> entries)
		{
			if (entries == null)
				entries = new List<InfrastructureDto>();
			Entries = entries;
		}

		// This method will be called by the NSTableView control to learn the number of rows to display.
		[Export ("numberOfRowsInTableView:")]
		/// <summary>
		/// Numbers the of rows in table view.
		/// </summary>
		/// <returns>The of rows in table view.</returns>
		/// <param name="table">Table.</param>
		public int NumberOfRowsInTableView (NSTableView table)
		{
			if (Entries != null)
				return Entries.Count;
			else
				return 0;
		}

		/// <summary>
		/// Gets the services status.
		/// </summary>
		/// <returns>The services status.</returns>
		/// <param name="obj">Object.</param>
		private static string GetServicesStatus (InfrastructureDto obj)
		{
			return obj.IsRemote ? string.Empty :
				(obj.Services != null && obj.Services.Count > 0
				? string.Format ("{0} of {1} " + Constants.Active,
					obj.Services.Count(x=>x.Alive),
					obj.Services.Count())
				: Constants.NoServicesAvailable);
		}

		/// <summary>
		/// Gets the heart beat date time.
		/// </summary>
		/// <returns>The heart beat date time.</returns>
		/// <param name="obj">Object.</param>
		private static string GetHeartBeatDateTime (InfrastructureDto obj)
		{
			return (obj.LastHeartBeat.HasValue ? DateTimeConverter.ToDurationAgo(obj.LastHeartBeat.Value) : string.Empty);
		}

		// This method will be called by the control for each column and each row.
		[Export ("tableView:objectValueForTableColumn:row:")]
		/// <summary>
		/// Objects the value for table column.
		/// </summary>
		/// <returns>The value for table column.</returns>
		/// <param name="table">Table.</param>
		/// <param name="col">Col.</param>
		/// <param name="row">Row.</param>
		public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
		{
			var value = (NSString)string.Empty;
			UIErrorHelper.CheckedExec (delegate() {
				if (Entries != null) {
					var obj = (this.Entries [row]) as InfrastructureDto;
					switch (col.Identifier) {
					case Constants.PscTableColumnNameId:
						value = (NSString)obj.Name;
						break;
					case Constants.PscTableColumnStatusId:
						value = (NSString) (obj.IsRemote ? Constants.UnKnown : (obj.Active ? Constants.Active : Constants.InActive));
						break;
					case Constants.PscTableColumnAffinitizedId:
						value = (NSString) (obj.IsAffinitized ? "Yes" : string.Empty);
						break;
					case Constants.PscTableColumnServicesId:
						value = (NSString)GetServicesStatus (obj) ;
						break;
					case Constants.PscTableColumnSiteLocationId:
						value = (NSString)(obj.IsRemote ? Constants.RemoteSite : Constants.SameSite) ;
						break;
					default:
						break;
					}
				}
			});
			return value;
		}
	}
}

