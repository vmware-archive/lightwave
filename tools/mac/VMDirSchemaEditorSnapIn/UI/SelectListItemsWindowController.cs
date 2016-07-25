﻿using System;

using Foundation;
using AppKit;
using System.Collections.Generic;
using VMDirSchemaEditorSnapIn.ListViews;
using System.Linq;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common;

namespace VMDirSchemaEditorSnapIn
{
    public partial class SelectListItemsWindowController : NSWindowController
    {
        public List<string> ItemsToSelect;
        private List<string> currentItems;
        private List<string> parentItems;

        public List<string> SelectedItemsList { get; set; }

        StringItemsListView itemsToSelectListView;
        StringItemsListView selectedItemsListview;

        public SelectListItemsWindowController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public SelectListItemsWindowController(NSCoder coder)
            : base(coder)
        {
        }

        public SelectListItemsWindowController(List<string> items, List<string> currentAttributes, List<string> parentAttributes)
            : base("SelectListItemsWindow")
        {
            this.ItemsToSelect = items;
            this.currentItems = currentAttributes;
            this.parentItems = parentAttributes;
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            InitialiseViews();

            SearchBox.Activated += StartSearch;
            SearchBoxCell.CancelButtonCell.Activated += OnCloseSearch;
        }

        public void InitialiseViews()
        {
            SelectedItemsList = new List<string>();
            itemsToSelectListView = new StringItemsListView(ItemsToSelect);
            selectedItemsListview = new StringItemsListView(SelectedItemsList);

            this.SelectItemsListBox.DataSource = itemsToSelectListView;
            this.SelectedItemsListBox.DataSource = selectedItemsListview;
        }

        partial void AddItems(Foundation.NSObject sender)
        {
            int row = (int)this.SelectItemsListBox.SelectedRow;
            if (row >= 0)
            {
                string selectedItem = (this.SelectItemsListBox.DataSource as StringItemsListView).Entries[row];
                if (currentItems != null && parentItems != null && (currentItems.Contains(selectedItem) || parentItems.Contains(selectedItem)))
                {
                    UIErrorHelper.ShowAlert(VMDirConstants.WRN_SEL_ITEM_PRESENT, string.Empty);
                }
                else
                {
                    SelectedItemsList.Add(selectedItem);
                    this.SelectedItemsListBox.ReloadData();
                }
            }

        }

        partial void RemoveItems(Foundation.NSObject sender)
        {
            int row = (int)this.SelectedItemsListBox.SelectedRow;
            if (row >= 0)
            {
                SelectedItemsList.RemoveAt(row);
            }
            this.SelectedItemsListBox.ReloadData();
        }

        public  void StartSearch(object sender, EventArgs e)
        {
            if (!string.IsNullOrWhiteSpace(SearchBox.StringValue))
            {
                var view = SelectItemsListBox.DataSource as StringItemsListView;
                view.Entries = view.Entries.FindAll(p => p.StartsWith(SearchBox.StringValue));
                this.SelectItemsListBox.ReloadData();
            }
        }

        public  void OnCloseSearch(object sender, EventArgs e)
        {
            SearchBoxCell.StringValue = string.Empty;
            itemsToSelectListView.Entries = ItemsToSelect;
            this.SelectItemsListBox.ReloadData();
        }

        partial void ApplyAction(Foundation.NSObject sender)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(VMIdentityConstants.DIALOGOK);
        }

        public new SelectListItemsWindow Window
        {
            get { return (SelectListItemsWindow)base.Window; }
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }
    }
}
