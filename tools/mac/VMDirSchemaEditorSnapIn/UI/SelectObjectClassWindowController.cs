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
