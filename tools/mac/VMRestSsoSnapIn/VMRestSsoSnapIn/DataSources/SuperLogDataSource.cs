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
using AppKit;
using System.Collections.Generic;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Security.Cryptography.X509Certificates;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.DataSource
{
	public class SuperLogDataSource : NSTableViewDataSource
	{
		private const string StatusColumnId = "Status";
		private const string EventTypeColumnId = "EventType";
		private const string CorrelationIdColumnId = "CorrelationId";
		private const string AccountColumnId = "Account";
		private const string ProviderColumnId = "Provider";
		private const string StartColumnId = "Start";
		private const string DurationColumnId = "Duration";

		public List<EventLogDto> Entries { get; set; }

		public SuperLogDataSource ()
		{
			Entries = new List<EventLogDto> ();
		}

		// This method will be called by the NSTableView control to learn the number of rows to display.
		[Export ("numberOfRowsInTableView:")]
		public int NumberOfRowsInTableView (NSTableView table)
		{
			if (Entries != null)
				return Entries.Count;
			else
				return 0;
		}

		// This method will be called by the control for each column and each row.
		[Export ("tableView:objectValueForTableColumn:row:")]
		public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
		{
			var value = (NSString)string.Empty;
			ActionHelper.Execute (delegate() {
				if (Entries != null) {
					var obj = (this.Entries [row]) as EventLogDto;

					switch (col.Identifier) {
					case StatusColumnId:
						value = (NSString)obj.Level.ToString();
						break;
					case EventTypeColumnId:
						value = (NSString)obj.Type;
						break;
					case CorrelationIdColumnId:
						value = (NSString)obj.CorrelationId;
						break;
					case AccountColumnId:
						value = (NSString)obj.AccountName;
						break;
					case ProviderColumnId:
						value = (NSString)obj.ProviderName;
						break;
					case StartColumnId:
						value = (NSString)DateTimeHelper.UnixToWindowsMilliSecs(obj.Start).ToString("dd-MMM-yy hh:mm:ss");
						break;
					case DurationColumnId:
						value = (NSString)obj.ElapsedMillis.ToString();
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

