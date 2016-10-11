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
using VMDirSchemaEditorSnapIn.ListViews;
using System.Collections.Generic;

namespace VMDirSchemaEditorSnapIn
{
    public partial class SelectObjectClassWindowController : NSWindowController
    {
        private List<string> ItemsToSelect;
        StringItemsListView itemsToSelectListView;

        public string SelectedItem { get; set; }

        public SelectObjectClassWindowController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public SelectObjectClassWindowController(NSCoder coder)
            : base(coder)
        {
        }

        public SelectObjectClassWindowController()
            : base("SelectObjectClassWindow")
        {
        }

        public SelectObjectClassWindowController(List<string> items)
            : base("SelectObjectClassWindow")
        {
            ItemsToSelect = items;

        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            SearchField.Activated += StartSearch;
            SearchFieldCell.CancelButtonCell.Activated += OnCloseSearch;
            InitialiseViews();
        }

        public void InitialiseViews()
        {
            itemsToSelectListView = new StringItemsListView(ItemsToSelect);

            this.ObjectClassList.DataSource = itemsToSelectListView;
        }

        public  void StartSearch(object sender, EventArgs e)
        {
            if (!string.IsNullOrWhiteSpace(SearchField.StringValue))
            {
                itemsToSelectListView.Entries = itemsToSelectListView.Entries.FindAll(p => p.StartsWith(SearchField.StringValue));
                this.ObjectClassList.ReloadData();
            }
            else
            {
                itemsToSelectListView.Entries = ItemsToSelect;
                this.ObjectClassList.ReloadData();
            }
        }

        public  void OnCloseSearch(object sender, EventArgs e)
        {
            SearchFieldCell.StringValue = string.Empty;
            itemsToSelectListView.Entries = ItemsToSelect;
            this.ObjectClassList.ReloadData();
        }

        partial void SelectAction(Foundation.NSObject sender)
        {
            if ((int)ObjectClassList.SelectedRow >= 0)
            {
                SelectedItem = itemsToSelectListView.Entries[(int)ObjectClassList.SelectedRow];
                this.Close();
                NSApplication.SharedApplication.StopModalWithCode(1);
            }
        }

        public new SelectObjectClassWindow Window
        {
            get { return (SelectObjectClassWindow)base.Window; }
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }
    }
}
