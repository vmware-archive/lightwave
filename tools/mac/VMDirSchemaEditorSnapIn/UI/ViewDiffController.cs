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

using Foundation;
using AppKit;
using System.Collections.Generic;
using System.Linq;

namespace VMDirSchemaEditorSnapIn
{
    public partial class ViewDiffController : NSWindowController
    {
        private List<KeyValuePair<string,string>> diffList;
        private string baseServerNode;
        private string currentNode;

        public ViewDiffController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public ViewDiffController(NSCoder coder)
            : base(coder)
        {
        }

        public ViewDiffController(string server, string currentNode, List<KeyValuePair<string,string>> diffList) //pass in currentdiff and metadata diff here and create a keyvaluepair.
            : base("ViewDiff")
        {
            this.diffList = diffList;
            this.baseServerNode = server;
            this.currentNode = currentNode;
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            InitialiseViews();
        }

        public void InitialiseViews()
        {
            NSTableColumn[] col = DiffTableView.TableColumns();
            col[0].Title = baseServerNode;
            col[1].Title = currentNode;

            this.DiffTableView.DataSource = new DiffListView(diffList);
            this.DiffTableView.ReloadData();
            this.DiffTableView.DoubleClick += OnDiffDoubleClicked;
        }

        public void OnDiffDoubleClicked(object sender, EventArgs e)
        {
            int row = (int)this.DiffTableView.SelectedRow;
            if (row >= 0)
            {
                KeyValuePair<string,string> item = diffList.ElementAt(row);
                DiffDetailViewerController dvc = new DiffDetailViewerController(this.baseServerNode, this.currentNode, item.Key, item.Value);
                NSApplication.SharedApplication.RunModalForWindow(dvc.Window);
            }
        }

        public new ViewDiff Window
        {
            get { return (ViewDiff)base.Window; }
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }
    }
}
