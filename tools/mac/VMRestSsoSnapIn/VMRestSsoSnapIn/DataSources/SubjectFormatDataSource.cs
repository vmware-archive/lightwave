/*
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

using System;
using AppKit;
using System.Collections.Generic;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.DataSource
{
	public class SubjectFormatDataSource : NSTableViewDataSource
	{
		public List<string> Entries { get; set; }
		public Dictionary<string,string> Datasource { get; set; }

		public SubjectFormatDataSource ()
		{
			Entries = new List<string> ();
			Datasource = new Dictionary<string, string> ();
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
					var name = (this.Entries [row]) as string;
					switch (col.Identifier) {
					case "Name":
						value = (NSString)name;
						break;
					case "Value": 
						string val = string.Empty;
						if(Datasource.TryGetValue(name,out val))
							value = (NSString)val;
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

