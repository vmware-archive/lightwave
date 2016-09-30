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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VMDirSchemaSnapIn.UI
{
    public partial class ViewDiffWindow : Form
    {
        private List<KeyValuePair<string, string>> diffList;
        private string baseServerNode;
        private string currentNode;

        public ViewDiffWindow(string server, string CurrentNode, List<KeyValuePair<string, string>> diffList)
        {
            InitializeComponent();
            CreateDataSet(diffList);
        }

        private void CreateDataSet(List<KeyValuePair<string, string>> diffList)
        {
            DataTable diffTable = new DataTable();
            DataColumn c1 = new DataColumn(VMDirSchemaConstants.BASE_TITLE);
            DataColumn c2 = new DataColumn(VMDirSchemaConstants.CURRENT_TITLE);
            diffTable.Columns.Add(c1);
            diffTable.Columns.Add(c2);

            foreach (var item in this.diffList)
            {
                diffTable.Rows.Add(item.Key, item.Value);
            }
            this.ViewDiffGrid.DataSource = diffTable;
        }

        private void DiffDoubleClicked(object sender, DataGridViewCellEventArgs e)
        {
            //Todo
            throw new NotImplementedException();
        }
    }
}
