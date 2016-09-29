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
using System.Windows.Forms;

namespace VMwareMMCIDP.UI
{
    public delegate bool ApplyDelegate(object obj, object objOld);

    public partial class GenericInputForm : Form
    {
        Object _object, _objectOld;
        public ApplyDelegate ApplyDelegate { get; set; }
        public void TurnSortOff()
        {
            this.propGridInput.PropertySort = PropertySort.NoSort;
        }

        public GenericInputForm(string title, string actionText, object objectToCollect, object objOld)
        {
            InitializeComponent();

            this.Text = title;
            btnApply.Text = actionText;

            _object = objectToCollect;
            _objectOld = objOld;

            this.propGridInput.SelectedObject = _object;
            this.propGridInput.PropertyValueChanged += new PropertyValueChangedEventHandler(propGridInput_PropertyValueChanged);
        }

        public GenericInputForm(string title, string actionText, object objectToCollect):this(title, actionText, objectToCollect, null)
        {
        }

        void propGridInput_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
        {
            btnApply.Enabled = true;
        }

        private void btnApply_Click(object sender, EventArgs e)
        {
            bool result = true;
            if (ApplyDelegate != null)
                result = ApplyDelegate(_object, _objectOld);

            if (result)
            {
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
        }
    }
}
