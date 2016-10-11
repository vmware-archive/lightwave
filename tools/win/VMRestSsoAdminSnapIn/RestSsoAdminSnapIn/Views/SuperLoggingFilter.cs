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
using System.Linq;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;


namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class SuperLoggingFilter : Form
    {
        private FilterCriteriaDto[] _originalFilters;
        public List<FilterCriteriaDto> Filters;
        public SuperLoggingFilter()
        {
            InitializeComponent();
            Filters = new List<FilterCriteriaDto>();
        }

        private void LoadDefaults()
        {
            FilterColumn column = new FilterColumn();
            var dict = column.ToDictionary();
            var columns = dict.Select( x=> new ComboBoxItem {  Value = x.Key, Display = x.Value }).ToList();
            cbColumn.DataSource = columns;
            cbColumn.DisplayMember = "Display";
            cbColumn.ValueMember = "Value";
            SetDeleteButtonStatus();
            SetAddButtonStatus();
            SetUpdateButtonStatus();
            _originalFilters = new FilterCriteriaDto[Filters.Count];
            Filters.CopyTo(_originalFilters, 0);
            RefreshGrid(Filters);
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            var isValid = ValidateFilter();

            if(isValid)
            {
                var column = (ComboBoxItem)cbColumn.SelectedItem;
                var op = (ComboBoxItem)cbOperator.SelectedItem;
                var criteria = new FilterCriteriaDto
                {
                    Column = (FilterColumn)(int)(column.Value),
                    Operator = (Operator)(int)(op.Value),
                    Value = TxtValue.Text
                };

                var isDuplicate = IsDuplicate(criteria);

                if(!isDuplicate)
                {
                    Filters.Add(criteria);
                    var item = new ListViewItem(new[] {column.Display, op.Display, TxtValue.Text})
                    {
                        Tag = criteria
                    };
                    listView1.Items.Add(item);
                    TxtValue.Text = string.Empty;
                    cbColumn.SelectedIndex = 0;
                }
                else
                {
                    ErrorMessageDisplayHelper.ShowError("You are trying to add a duplicate filter.");
                }
            }
            SetAddButtonStatus();
            SetUpdateButtonStatus();
            SetDeleteButtonStatus();
        }

        public void RefreshGrid(List<FilterCriteriaDto> filters)
        {
            if(filters != null)
            {
                foreach(var filter in filters)
                {
                    var column = filter.Column.GetDescription();
                    var op = filter.Operator.GetDescription();
                    var item = new ListViewItem(new[] { column, op, filter.Value})
                    {
                        Tag = filter
                    };
                    listView1.Items.Add(item);
                }
            }
        }

        private void SetDeleteButtonStatus()
        {
            btnRemove.Enabled = Filters.Count() > 0;
        }
        private void SetUpdateButtonStatus()
        {
            btnUpdate.Enabled = Filters.Count() > 0 
                && listView1.SelectedItems != null
                && listView1.SelectedItems.Count > 0
                && IsUIStateValid();
        }
        private bool ValidateFilter()
        {
            if(string.IsNullOrWhiteSpace(TxtValue.Text))
            {
                ErrorMessageDisplayHelper.ShowError("Filter value cannot be left empty.");
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

        private void btnRemove_Click(object sender, EventArgs e)
        {
            if(listView1.Items.Count > 0 && listView1.SelectedItems.Count >  0)
            {
                foreach(ListViewItem item in listView1.SelectedItems)
                {
                    var dto = (FilterCriteriaDto)(item.Tag);
                    Filters.Remove(dto);
                    listView1.Items.Remove(item);
                }
            }
            SetDeleteButtonStatus();
            SetUpdateButtonStatus();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            if (_originalFilters.Count() > 0)
                Filters = new List<FilterCriteriaDto>(_originalFilters);
            else
                Filters = new List<FilterCriteriaDto>();

            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void cbColumn_SelectedIndexChanged(object sender, EventArgs e)
        {
            if(cbColumn.SelectedIndex > -1)
            {   
                var value = (int)((ComboBoxItem)cbColumn.SelectedItem).Value;
                var columns = GetDataSource(value);
                cbOperator.DataSource = columns;
                cbOperator.DisplayMember = "Display";
                cbOperator.ValueMember = "Value";
                SetAddButtonStatus();
                SetUpdateButtonStatus();
            }
        }

        private List<ComboBoxItem> GetDataSource(int value)
        {
            var isNum = IsNumeric(value);
            var columns = isNum ? GetNumericSource() : GetStringSource();
            return columns;
        }

        private bool IsNumeric(int value)
        {
            return
            (value == (int)FilterColumn.Start)
                ||
            (value == (int)FilterColumn.Duration);
        }

        private List<ComboBoxItem> GetStringSource()
        {
            Operator column = new Operator();
            var dict = column.ToDictionary();
            var stringSource = dict.Where(x => (x.Key != (int)Operator.GreaterThan) && (x.Key != (int)Operator.LessThan));
            return stringSource.Select(x => new ComboBoxItem { Value = x.Key, Display = x.Value }).ToList();
        }

        private List<ComboBoxItem> GetNumericSource()
        {
            Operator column = new Operator();
            var dict = column.ToDictionary();
            var numSource = dict
                .Where(x => x.Key != (int)Operator.BeginsWith && x.Key != (int)Operator.EndsWith && x.Key != (int)Operator.Contains);
            return numSource.Select(x => new ComboBoxItem { Value = x.Key, Display = x.Value }).ToList();
        }

        private void SuperLoggingFilter_Load(object sender, EventArgs e)
        {
            LoadDefaults();
        }

        private void TxtValue_TextChanged(object sender, EventArgs e)
        {
            SetAddButtonStatus();
            SetUpdateButtonStatus();
        }

        private void SetAddButtonStatus()
        {
            btnAdd.Enabled = IsUIStateValid();
        }

        private bool IsUIStateValid()
        {
             return (cbColumn.SelectedIndex > -1)
                && (cbOperator.SelectedIndex > -1)
                && (!string.IsNullOrWhiteSpace(TxtValue.Text));
        }

        private void cbOperator_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cbOperator.SelectedIndex > -1)
            {
                SetAddButtonStatus();
                SetUpdateButtonStatus();
            }
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if(listView1.SelectedItems != null && listView1.SelectedItems.Count > 0 )
            {
                var dto = listView1.SelectedItems[0].Tag as FilterCriteriaDto;
                var colDesc = dto.Column.GetDescription();
                var optDesc = dto.Operator.GetDescription();

                cbColumn.SelectedItem = new ComboBoxItem
                {
                    Value = (int)dto.Column,
                    Display = colDesc
                };

                cbOperator.SelectedItem = new ComboBoxItem
                {
                    Value = (int)dto.Operator,
                    Display = optDesc
                };

                cbColumn.Text = colDesc;
                cbOperator.Text = optDesc;
                TxtValue.Text = dto.Value;
                btnUpdate.Enabled = true;
            }
        }

        private void btnUpdate_Click(object sender, EventArgs e)
        {
            if (listView1.SelectedItems != null 
                && listView1.SelectedItems.Count > 0
                && ValidateFilter())
            {
                var column = (ComboBoxItem)cbColumn.SelectedItem;
                var op = (ComboBoxItem)cbOperator.SelectedItem; 
                var criteria = new FilterCriteriaDto
                {
                    Column = (FilterColumn)(int)(column.Value),
                    Operator = (Operator)(int)(op.Value),
                    Value = TxtValue.Text
                };
                var isDuplicate = IsDuplicate(criteria);

                if (!isDuplicate)
                {
                    listView1.SelectedItems[0].Tag = criteria;
                    listView1.SelectedItems[0].SubItems[0].Text = criteria.Column.GetDescription();
                    listView1.SelectedItems[0].SubItems[1].Text = criteria.Operator.GetDescription();
                    listView1.SelectedItems[0].SubItems[2].Text = criteria.Value;
                    TxtValue.Text = string.Empty;
                    SetUpdateButtonStatus();
                    SetAddButtonStatus();
                }
            }
        }
    }

    public class ComboBoxItem
    {
        public int Value { get; set; }
        public string Display { get; set; }
    }
}
