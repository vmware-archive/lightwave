/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using AppKit;
using Foundation;
using VMDirSnapIn.DataSource;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.UI
{
    public partial class SchemaBrowserWindowController : NSWindowController
    {
        private string _currentObject;
        private VMDirServerDTO _serverDTO;
        private List<string> _classList;
        private List<AttributeTypeDTO> _requiredAttributesList, _optionalAttributesList;
        private ContentRuleDTO _contentRule;

        #region Constructors

        // Called when created from unmanaged code
        public SchemaBrowserWindowController (IntPtr handle) : base (handle)
        {
        }
        
        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public SchemaBrowserWindowController (NSCoder coder) : base (coder)
        {
        }
        
        // Call to load from the XIB/NIB file
        public SchemaBrowserWindowController () : base ("SchemaBrowserWindow")
        {
        }

        // Call to load from the XIB/NIB file
        public SchemaBrowserWindowController (VMDirServerDTO dto) : base ("SchemaBrowserWindow")
        {
            _serverDTO = dto;
            Bind ();
        }

        #endregion

        private void Bind ()
        {
            UIErrorHelper.CheckedExec (delegate() {
                if (_serverDTO.Connection == null) {
                    _serverDTO.Connection = new LdapConnectionService (_serverDTO.Server, _serverDTO.BindDN, _serverDTO.Password);
                    _serverDTO.Connection.CreateConnection ();
                }
                _classList = _serverDTO.Connection.SchemaManager.GetObjectClassManager ().Data.Select (x => x.Key).ToList ();

                _classList.Sort ();
            });
        }

        private void RefreshTabs ()
        {
            RefreshHirearchy ();
            RefreshRequiredAttributes ();
            RefreshOptionalAttributes ();
            RefreshDitContentRules ();
        }

        private void RefreshHirearchy ()
        {
            if (schemaTabView.Selected.Identifier.ToString () == "Hierarchy") {
                if (objectClassesList.SelectedRows.Count > 0) {
                    HierarchyItemDatasource ds = new HierarchyItemDatasource (_currentObject);

                    var mgr = _serverDTO.Connection.SchemaManager;
                    var dto = mgr.GetObjectClass (_currentObject);
                    HierarchyItem rootItem = new HierarchyItem (_currentObject);
                    HierarchyItem prevItem = rootItem;
                    while (dto != null) {
                        dto = mgr.GetObjectClass (dto.SuperClass);
                        if (dto != null) {
                            HierarchyItem currItem = new HierarchyItem (dto.Name);
                            prevItem.Children.Add (currItem);
                            prevItem = currItem;
                        }
                        ds.Item = rootItem;
                    }
                    this.HierarchyOutlineView.DataSource = ds;
                    this.HierarchyOutlineView.ReloadData ();
                }
            }
        }

        private void RefreshRequiredAttributes ()
        {
            if (schemaTabView.Selected.Identifier.ToString ().Equals ("Required")) {
                _requiredAttributesList = _serverDTO.Connection.SchemaManager.GetRequiredAttributes (_currentObject);
                this.RequiredAttributesView.DataSource = new SchemaAttributesTableViewDataSource (_requiredAttributesList);
                this.RequiredAttributesView.ReloadData ();
            }
        }

        private void RefreshOptionalAttributes ()
        {
            if (schemaTabView.Selected.Identifier.ToString ().Equals ("Optional")) {
                _optionalAttributesList = _serverDTO.Connection.SchemaManager.GetOptionalAttributes (_currentObject);
                this.OptionalAttributesView.DataSource = new SchemaAttributesTableViewDataSource (_optionalAttributesList);
                this.OptionalAttributesView.ReloadData ();
            }
        }

        private void RefreshDitContentRules ()
        {
            if (schemaTabView.Selected.Identifier.ToString ().Equals ("ContentRules")) {
                if (!FillContentRule ()) {
                    //flush out the existing datasources which are displayed in the tabview
                    this.ContentRulesAuxTableView.DataSource = null;
                    this.ContentRulesMustTableView.DataSource = null;
                    this.ContentRulesMayTableView.DataSource = null;
                    return;
                }
                switch (ContentRulesTabView.Selected.Identifier.ToString ()) {
                case "Aux":
                    this.ContentRulesAuxTableView.DataSource = new GenericListViewDataSource (_contentRule.Aux);
                    this.ContentRulesAuxTableView.ReloadData ();
                    break;
                case "Must":
                    this.ContentRulesMustTableView.DataSource = new GenericListViewDataSource (_contentRule.Must);
                    this.ContentRulesAuxTableView.ReloadData ();
                    break;
                case "May":
                    this.ContentRulesMayTableView.DataSource = new GenericListViewDataSource (_contentRule.May);
                    this.ContentRulesAuxTableView.ReloadData ();
                    break;
                
                }
            }
        }

        private bool FillContentRule ()
        {
            if (_contentRule == null)
                _contentRule = _serverDTO.Connection.SchemaManager.GetContentRule (_currentObject);
            return _contentRule != null;
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            //Events and Delegates
            NSTableColumn col;
            this.objectClassesList.Delegate = new SchemaBrowserTableDelegate (this);
            col = this.objectClassesList.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();

            this.RequiredAttributesView.Delegate = new MainWindowController.GenericTableDelegate ();
            col = this.RequiredAttributesView.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();

            this.OptionalAttributesView.Delegate = new MainWindowController.GenericTableDelegate ();
            col = this.OptionalAttributesView.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();

            this.ContentRulesMustTableView.Delegate = new MainWindowController.GenericTableDelegate ();
            col = this.ContentRulesMustTableView.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();

            this.ContentRulesMayTableView.Delegate = new MainWindowController.GenericTableDelegate ();
            col = this.ContentRulesMayTableView.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();

            this.ContentRulesAuxTableView.Delegate = new MainWindowController.GenericTableDelegate ();
            col = this.ContentRulesAuxTableView.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();

            this.HierarchyOutlineView.OutlineTableColumn.DataCell = new NSBrowserCell ();
            this.HierarchyOutlineView.Delegate = new OutlineDelegate (this);
            this.objectClassesList.DataSource = new GenericListViewDataSource (_classList);
            this.schemaTabView.DidSelect += (sender, e) => RefreshTabs ();
            this.ContentRulesTabView.DidSelect += (sender, e) => RefreshDitContentRules ();

            this.objectClassesList.SelectRow ((nint)0, false);

            this.CloseButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (0);
            };
        }

        [Export ("windowWillClose:")]
        public void WindowWillClose (NSNotification notification)
        {
            NSApplication.SharedApplication.StopModalWithCode (0);
        }

        //strongly typed window accessor
        public new SchemaBrowserWindow Window {
            get {
                return (SchemaBrowserWindow)base.Window;
            }
        }

        class SchemaBrowserTableDelegate : NSTableViewDelegate
        {
            SchemaBrowserWindowController ob;
            private NSImage directoryIcon;

            public SchemaBrowserTableDelegate (SchemaBrowserWindowController ob)
            {
                this.ob = ob;
                directoryIcon = NSImage.ImageNamed ("directoryObject.png");
            }

            public override void SelectionDidChange (NSNotification notification)
            {
                ob._contentRule = null;
                ob._currentObject = string.Empty;
                if (ob.objectClassesList.SelectedRows.Count > 0)
                    ob._currentObject = ob._classList [(int)ob.objectClassesList.SelectedRow];
                ob.RefreshTabs ();

            }

            public override void WillDisplayCell (NSTableView tableView, NSObject cell,
                                                  NSTableColumn tableColumn, nint row)
            {
                try {
                    NSBrowserCell browserCell = cell as NSBrowserCell;
                    if (browserCell != null) {
                        browserCell.Leaf = true;
                        browserCell.Image = directoryIcon;
                    }
                } catch (Exception e) {
                    System.Diagnostics.Debug.WriteLine ("Exception in casting : " + e.Message);
                }
            }
        }

        //Delegate and data source classes used by schema browser

        public class OutlineDelegate : NSOutlineViewDelegate
        {
            private NSImage directoryIcon;
            SchemaBrowserWindowController ob;

            public OutlineDelegate (SchemaBrowserWindowController ob)
            {
                this.ob = ob;
                directoryIcon = NSImage.ImageNamed ("directoryObject.png");
            }

            public override void WillDisplayCell (NSOutlineView outlineView, NSObject cell,
                                                  NSTableColumn tableColumn, NSObject item)
            {
                try {
                    NSBrowserCell browserCell = cell as NSBrowserCell;
                    if (browserCell != null) {
                        browserCell.Leaf = true;
                        browserCell.Image = directoryIcon;
                    }
                } catch (Exception e) {
                    System.Diagnostics.Debug.WriteLine ("Exception in casting : " + e.Message);
                }
            }
        }

        class HierarchyItem:NSObject
        {
            public string Name { get; set; }

            public List<HierarchyItem> Children { get; set; }

            public HierarchyItem ()
            {
            }

            public HierarchyItem (string name)
            {
                Name = name;
                Children = new List<HierarchyItem> ();
            }
        }

        class HierarchyItemDatasource:NSOutlineViewDataSource
        {
            public HierarchyItem Item;

            // Constructor
            public HierarchyItemDatasource (string currentObject)
            {
                Item = new HierarchyItem (currentObject);
            }

            public override nint GetChildrenCount (NSOutlineView outlineView, NSObject item)
            {
                // If the item is not null, return the child count of our item
                return item != null ? (item as HierarchyItem).Children.Count : 1;
            }

            public override NSObject GetObjectValue (NSOutlineView outlineView, NSTableColumn tableColumn, NSObject item)
            {
                if (item != null) {
                    var p = ((HierarchyItem)item);
                    return (NSString)p.Name;

                }
                return (NSString)"item null";
            }

            public override NSObject GetChild (NSOutlineView outlineView, nint childIndex, NSObject item)
            {
                // If the item is null, it's asking for a root element.
                return item == null ? this.Item : (NSObject)((item as HierarchyItem).Children [(int)childIndex]);
            }

            public override bool ItemExpandable (NSOutlineView outlineView, NSObject item)
            {
                if (item == null)
                    return false;
                return (item as HierarchyItem).Children.Count > 0;
            }
        }

    }


}

