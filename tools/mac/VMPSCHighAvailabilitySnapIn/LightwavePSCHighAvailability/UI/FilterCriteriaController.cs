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
using Foundation;
using AppKit;
using VMPSCHighAvailability.DataSources;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.Common.Helpers;
namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Filter criteria controller.
	/// </summary>
	public partial class FilterCriteriaController : NSWindowController
	{
		/// <summary>
		/// The filter criteria data source.
		/// </summary>
		public FilterCriteriaDataSource FilterCriteriaDataSource;

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.FilterCriteriaController"/> class.
		/// </summary>
		/// <param name="handle">Handle.</param>
		public FilterCriteriaController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.FilterCriteriaController"/> class.
		/// </summary>
		/// <param name="coder">Coder.</param>
		public FilterCriteriaController (NSCoder coder) : base (coder)
		{
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.FilterCriteriaController"/> class.
		/// </summary>
		public FilterCriteriaController () : base ("FilterCriteria")
		{
		}

		/// <summary>
		/// Awakes from nib.
		/// </summary>
		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			CancelButton.Activated += CancelButton_Activated;
			SaveButton.Activated += SaveButton_Activated;
			AddButton.Activated += AddButton_Activated;
			RemoveButton.Activated += RemoveButton_Activated;
		}

		/// <summary>
		/// Initialize this instance.
		/// </summary>
		private void Initialize()
		{
			FilterTableView.Delegate = new FilterTableViewDelegate (this);
			FilterCriteriaDataSource = new FilterCriteriaDataSource {
				Entries = new List<FilterCriteriaDto>()
			};
			FilterTableView.DataSource = FilterCriteriaDataSource;
			FilterTableView.ReloadData ();
			PopulateComboBox ();
		}

		/// <summary>
		/// Populates the combo box.
		/// </summary>
		private void PopulateComboBox()
		{
			ColumnComboBox.RemoveAll ();
			ColumnComboBox.UsesDataSource = true;
			ColumnComboBox.Completes = false;
			ColumnComboBox.DataSource = new ColumnsDataSource ();

			OperatorComboBox.RemoveAll ();
			OperatorComboBox.UsesDataSource = true;
			OperatorComboBox.Completes = false;
			OperatorComboBox.DataSource = new OperatorsDataSource ();
		}

		/// <summary>
		/// Removes the filter criteria when the button is activated.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void RemoveButton_Activated (object sender, EventArgs e)
		{
			var rowid = (int)FilterTableView.SelectedRow;
			if (-1 < rowid) {
				FilterCriteriaDataSource.Entries.RemoveAt (rowid);
				FilterTableView.DataSource = FilterCriteriaDataSource;
				FilterTableView.ReloadData ();
			}
		}

		/// <summary>
		/// Adds the filter criteria when the button is activated.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void AddButton_Activated (object sender, EventArgs e)
		{
			var op = Operator.Contains;
			var filterCriteria = new FilterCriteriaDto {
				Column = ColumnComboBox.SelectedValue.ToString (),
				Operator = (Operator)EnumHelper.GetByDescription(op, OperatorComboBox.SelectedValue.ToString ()),
				Value = ValueTextField.StringValue
			};
			FilterCriteriaDataSource.Entries.Add (filterCriteria);
			FilterTableView.DataSource = FilterCriteriaDataSource;
			FilterTableView.ReloadData ();

			ColumnComboBox.SelectItem (0);
			OperatorComboBox.SelectItem (0);
			ValueTextField.StringValue = string.Empty;
		}

		/// <summary>
		/// Saves the filter criteria when the button is activated.
		/// </summary>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void SaveButton_Activated (object sender, EventArgs e)
		{
			NSApplication.SharedApplication.StopModalWithCode (1);
			this.Close ();
		}

		/// <summary>
		/// Determines whether this instance cancel button activated the specified sender e.
		/// </summary>
		/// <returns><c>true</c> if this instance cancel button activated the specified sender e; otherwise, <c>false</c>.</returns>
		/// <param name="sender">Sender.</param>
		/// <param name="e">E.</param>
		void CancelButton_Activated (object sender, EventArgs e)
		{
			NSApplication.SharedApplication.StopModalWithCode (0);
			this.Close ();
		}

		/// <summary>
		/// Gets the window.
		/// </summary>
		/// <value>The window.</value>
		public new FilterCriteria Window {
			get { return (FilterCriteria)base.Window; }
		}


	}

	class ColumnsDataSource : NSComboBoxDataSource
	{
		List<string> columns = new List<string> 
		{ "None", "PSC Host Name", "Affinitized", "Status", "Service Status Details", "Last Heartbeat", "Site Name" };

			
		public override string CompletedString (NSComboBox comboBox, string uncompletedString)
		{
			return columns.Find (n => n.StartsWith (uncompletedString, StringComparison.InvariantCultureIgnoreCase));
		}

		public override nint IndexOfItem (NSComboBox comboBox, string value)
		{
			return columns.FindIndex (n => n.Equals (value, StringComparison.InvariantCultureIgnoreCase));
		}

		public override nint ItemCount (NSComboBox comboBox)
		{
			return columns.Count;
		}

		public override NSObject ObjectValueForItem (NSComboBox comboBox, nint index)
		{
			return NSObject.FromObject (columns [(int)index]);
		}
	}

	class OperatorsDataSource : NSComboBoxDataSource
	{
		private List<string> ops = new List<string> ();

		public OperatorsDataSource()
		{
			Operator op  = Operator.Contains;
			var data = EnumHelper.ToList (op);
			ops.Add ("None");
			ops.AddRange (data);
		}

		public override string CompletedString (NSComboBox comboBox, string uncompletedString)
		{
			return ops.Find (n => n.StartsWith (uncompletedString, StringComparison.InvariantCultureIgnoreCase));
		}

		public override nint IndexOfItem (NSComboBox comboBox, string value)
		{
			return ops.FindIndex (n => n.Equals (value, StringComparison.InvariantCultureIgnoreCase));
		}

		public override nint ItemCount (NSComboBox comboBox)
		{
			return ops.Count;
		}

		public override NSObject ObjectValueForItem (NSComboBox comboBox, nint index)
		{
			return NSObject.FromObject (ops [(int)index]);
		}
	}
}
