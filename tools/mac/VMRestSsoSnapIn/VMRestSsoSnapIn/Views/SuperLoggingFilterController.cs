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
using System.Text;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace RestSsoAdminSnapIn
{
	public partial class SuperLoggingFilterController : NSWindowController
	{
		private FilterCriteriaDto[] _originalFilters;
		public List<FilterCriteriaDto> Filters;

		public SuperLoggingFilterController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public SuperLoggingFilterController (NSCoder coder) : base (coder)
		{
		}

		public SuperLoggingFilterController () : base ("SuperLoggingFilter")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			ValueTextField.Activated += ValueTextField_Activated;
			ColumnComboBox.SelectionChanged += ColumnComboBox_SelectionChanged;
			OperatorComboBox.SelectionChanged += OperatorComboBox_SelectionChanged;
			RemoveButton.Activated += RemoveButton_Activated;
			AddButton.Activated += AddButton_Activated;
			UpdateButton.Activated += UpdateButton_Activated;
			CancelButton.Activated += CancelButton_Activated;
			FilterTableView.SelectionDidChange += FilterTableView_SelectionDidChange;
			CloseButton.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			LoadDefaults ();
		}

		void FilterTableView_SelectionDidChange (object sender, EventArgs e)
		{
			var row = (int)FilterTableView.SelectedRow;
			PopulateOnSelect (row);
		}

		void UpdateButton_Activated (object sender, EventArgs e)
		{
			var isValid = ValidateFilter();

			if(isValid)
			{
				FilterColumn col = new FilterColumn ();
				Operator oper = new Operator ();
				var selColumnValue = ColumnComboBox.SelectedValue as NSString;
				var column = (FilterColumn)col.GetByDescription(selColumnValue.ToString());

				var selOpValue = OperatorComboBox.SelectedValue as NSString;
				var op = (Operator)oper.GetByDescription(selOpValue.ToString());
				var criteria = new FilterCriteriaDto
				{
					Column = column,
					Operator = op,
					Value = ValueTextField.StringValue
				};

				var isDuplicate = IsDuplicate(criteria);

				if(!isDuplicate)
				{
					var row = (int)FilterTableView.SelectedRow;
					if(row > -1)
					{
						Filters [row] = criteria;
						RefreshGrid (Filters);
						ValueTextField.StringValue = (NSString)string.Empty;
					}
				}
				else
				{
					UIErrorHelper.ShowAlert(null,"You are trying to add a duplicate filter.");
				}
			}
			SetDeleteButtonStatus();
			SetUpdateButtonStatus ();
			SetAddButtonStatus ();
		}

		void AddButton_Activated (object sender, EventArgs e)
		{
			var isValid = ValidateFilter();

			if(isValid)
			{
				FilterColumn col = new FilterColumn ();
				Operator oper = new Operator ();
				var selColumnValue = ColumnComboBox.SelectedValue as NSString;
				var column = (FilterColumn)col.GetByDescription(selColumnValue.ToString());
				var selOpValue = OperatorComboBox.SelectedValue as NSString;
				var op = (Operator)oper.GetByDescription(selOpValue.ToString());
				var criteria = new FilterCriteriaDto
				{
					Column = column,
					Operator = op,
					Value = ValueTextField.StringValue
				};

				var isDuplicate = IsDuplicate(criteria);

				if(!isDuplicate)
				{
					Filters.Add(criteria);
					RefreshGrid (Filters);
					ValueTextField.StringValue = (NSString)string.Empty;
				}
				else
				{
					UIErrorHelper.ShowAlert(null,"You are trying to add a duplicate filter.");
				}
			}
			SetDeleteButtonStatus();
			SetUpdateButtonStatus ();
		}

		private void PopulateOnSelect(int row)
		{
			if (row > -1) {
				ValueTextField.StringValue = Filters [row].Value;
				var col = Filters [row].Column.GetDescription();
				ColumnComboBox.Select(NSObject.FromObject (col));
				var op = Filters [row].Operator.GetDescription ();
				OperatorComboBox.Select (NSObject.FromObject (op));
			}
		}

		private void SetUpdateButtonStatus()
		{
			UpdateButton.Enabled = Filters.Count() > 0
				&& FilterTableView.SelectedRowCount > 0
				&& IsUIStateValid();
		}
		void CancelButton_Activated (object sender, EventArgs e)
		{
			if (_originalFilters.Count() > 0)
				Filters = new List<FilterCriteriaDto>(_originalFilters);
			else
				Filters = new List<FilterCriteriaDto>();
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode (0);
		}

		void RemoveButton_Activated (object sender, EventArgs e)
		{
			var rowCount = (int)FilterTableView.RowCount;
			var selectedRowCount = (int)FilterTableView.SelectedRowCount;
			if(rowCount > 0 && selectedRowCount >  0)
			{
				var index = (int)FilterTableView.SelectedRow;
				Filters.RemoveAt(index);
				RefreshGrid (Filters);
			}
			SetDeleteButtonStatus();
			SetUpdateButtonStatus ();
		}

		void OperatorComboBox_SelectionChanged (object sender, EventArgs e)
		{
			SetAddButtonStatus();
			SetUpdateButtonStatus ();
		}

		void ColumnComboBox_SelectionChanged (object sender, EventArgs e)
		{
			var rowId = ((int)ColumnComboBox.SelectedIndex);
			if(rowId > -1)
			{ 
				FilterColumn column = new FilterColumn ();
				var selColumnValue = ColumnComboBox.SelectedValue as NSString;
				var col = (FilterColumn)column.GetByDescription(selColumnValue.ToString());
				var operators = GetDataSource(col);
				var ops = operators.Select(x => NSObject.FromObject(x)).ToArray();
				OperatorComboBox.StringValue = (NSString)string.Empty;
				OperatorComboBox.RemoveAll ();
				OperatorComboBox.Add (ops);
				SetAddButtonStatus();
				SetUpdateButtonStatus ();
			}
		}

		void ValueTextField_Activated (object sender, EventArgs e)
		{
			SetAddButtonStatus();
			SetUpdateButtonStatus ();
		}

		public new SuperLoggingFilter Window {
			get { return (SuperLoggingFilter)base.Window; }
		}

		private void LoadDefaults()
		{
			FilterColumn column = new FilterColumn();
			var list = column.ToDictionary().Select(x => NSObject.FromObject(x.Value)).ToArray();
			ColumnComboBox.Add(list);
			SetDeleteButtonStatus();
			SetAddButtonStatus();
			SetUpdateButtonStatus ();
			_originalFilters = new FilterCriteriaDto[Filters.Count];
			Filters.CopyTo(_originalFilters, 0);
			RefreshGrid(Filters);
		}

		private List<string> GetDataSource(FilterColumn value)
		{
			var isNum = IsNumeric(value);
			var columns = isNum ? GetNumericSource() : GetStringSource();
			return columns;
		}

		private bool IsNumeric(FilterColumn value)
		{
			return
				(value == FilterColumn.Start)
				||
				(value == FilterColumn.Duration);
		}

		private List<string> GetStringSource()
		{
			Operator column = new Operator();
			var dict = column.ToDictionary();
			var stringSource = dict.Where(x => (x.Key != (int)Operator.GreaterThan)
				&& (x.Key != (int)Operator.LessThan));
			return stringSource.Select(x => x.Value).ToList();
		}

		private List<string> GetNumericSource()
		{
			Operator column = new Operator();
			var dict = column.ToDictionary();
			var numSource = dict
				.Where(x => x.Key != (int)Operator.BeginsWith
					&& x.Key != (int)Operator.EndsWith
					&& x.Key != (int)Operator.Contains);
			return numSource.Select(x=>x.Value).ToList();
		}

		private void SetDeleteButtonStatus()
		{
			RemoveButton.Enabled = Filters.Count() > 0;
		}
		private bool ValidateFilter()
		{
			if(string.IsNullOrWhiteSpace(ValueTextField.StringValue))
			{
				UIErrorHelper.ShowAlert(null, "Filter value cannot be left empty.");
				return false;
			}
			return true;
		}

		private bool IsDuplicate(FilterCriteriaDto criteria)
		{
			return Filters.Exists(x =>
				x.Column == criteria.Column
				&& x.Operator == criteria.Operator
				&& x.Value == criteria.Value);
		}
		private void SetAddButtonStatus()
		{
			AddButton.Enabled = IsUIStateValid();
		}

		private bool IsUIStateValid()
		{
			return (ColumnComboBox.SelectedIndex > -1)
			&& (OperatorComboBox.SelectedIndex > -1)
			&& (!string.IsNullOrWhiteSpace (ValueTextField.StringValue));
		}

		public void RefreshGrid(List<FilterCriteriaDto> filters)
		{
			var listView = new SuperLogFilterDataSource { Entries = filters};
			FilterTableView.DataSource = listView;
			FilterTableView.ReloadData ();
		}
	}
}
