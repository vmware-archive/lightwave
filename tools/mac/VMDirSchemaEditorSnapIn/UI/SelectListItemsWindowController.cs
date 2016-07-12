using System;

using Foundation;
using AppKit;
using System.Collections.Generic;
using VMDirSchemaEditorSnapIn.ListViews;
using System.Linq;
using VmIdentity.UI.Common;

namespace VMDirSchemaEditorSnapIn
{
    public partial class SelectListItemsWindowController : NSWindowController
    {
        public List<string> ItemsToSelect;

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

        public SelectListItemsWindowController(List<string> items)
            : base("SelectListItemsWindow")
        {
            this.ItemsToSelect = items;
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
                SelectedItemsList.Add((this.SelectItemsListBox.DataSource as StringItemsListView).Entries[row]);
            }
            this.SelectedItemsListBox.ReloadData();
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
