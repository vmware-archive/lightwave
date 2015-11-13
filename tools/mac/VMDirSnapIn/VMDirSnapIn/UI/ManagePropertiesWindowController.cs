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
using Foundation;
using AppKit;
using VMDirSnapIn.DataSource;
using VMDir.Common.DTO;

namespace VMDirSnapIn.UI
{
    public partial class ManagePropertiesWindowController : AppKit.NSWindowController
    {
        private string _objectClass;
        private VMDirServerDTO _serverDTO;
        private List<KeyValuePair<string,string>> rhsList = new List<KeyValuePair<string,string>> ();
        private List<KeyValuePair<string,string>> lhsList = new List<KeyValuePair<string,string>> ();

        public List<KeyValuePair<string,string>> OptionalAttributes { get { return rhsList; } }


        #region Constructors

        // Called when created from unmanaged code
        public ManagePropertiesWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public ManagePropertiesWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public ManagePropertiesWindowController () : base ("ManagePropertiesWindow")
        {
        }

        public ManagePropertiesWindowController (string objectClass, IEnumerable<KeyValuePair<string,string>> existingAttributes, VMDirServerDTO serverDTO) : base ("ManagePropertiesWindow")
        {
            _objectClass = objectClass;
            _serverDTO = serverDTO;
            Bind (existingAttributes);
        }

        int ListSort (KeyValuePair<string,string> lhs, KeyValuePair<string,string> rhs)
        {
            return lhs.Key.CompareTo (rhs.Key);
        }

        void Bind (IEnumerable<KeyValuePair<string,string>> currentAttributes)
        {
            rhsList = currentAttributes.ToList ();
            lhsList = _serverDTO.Connection.SchemaManager.GetOptionalAttributes (_objectClass).Select (x => new KeyValuePair<string,string> (x.Name, x.Description)).ToList ();
            //is it required to remove the rhslist from lhslist?
            //foreach (var item in rhsList) {
            //    lhsList.Remove (item);
            //}
            lhsList.Sort (ListSort);
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
            NewAttributesList.DataSource = new GenericTableViewDataSource (lhsList);
            ExistingAttributesList.DataSource = new GenericTableViewDataSource (rhsList);

            NSTableColumn col;
            col = this.NewAttributesList.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();

            this.NewAttributesList.Delegate = new MainWindowController.GenericTableDelegate ();

            col = this.ExistingAttributesList.TableColumns () [0];
            if (col != null)
                col.DataCell = new NSBrowserCell ();
            this.ExistingAttributesList.Delegate = new MainWindowController.GenericTableDelegate ();

            AddButton.Activated += OnClickAddButton;
            RemoveButton.Activated += OnClickRemoveButton;
            ApplyButton.Activated += OnClickApplyButton;
            CancelButton.Activated += OnClickCancelButton;
        }

        private void OnClickAddButton (object sender, EventArgs e)
        {
            nint index = NewAttributesList.SelectedRow;
            if (index >= 0) {
                rhsList.Add (lhsList [(int)NewAttributesList.SelectedRow]);
                lhsList.RemoveAt ((int)index);
                RefreshData ();
            }
        }

        private void OnClickRemoveButton (object sender, EventArgs e)
        {
            nint index = ExistingAttributesList.SelectedRow;
            if (index >= (nint)0) {
                lhsList.Add (rhsList [(int)index]);
                rhsList.RemoveAt ((int)index);
                RefreshData ();
            }
        }

        private void OnClickApplyButton (object sender, EventArgs e)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (1);
        }

        private void OnClickCancelButton (object sender, EventArgs e)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (0);
        }

        private void RefreshData ()
        {
            NewAttributesList.ReloadData ();
            ExistingAttributesList.ReloadData ();
        }

        #endregion

        [Export ("windowWillClose:")]
        public void WindowWillClose (NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal ();
        }

        //strongly typed window accessor
        public new ManagePropertiesWindow Window {
            get {
                return (ManagePropertiesWindow)base.Window;
            }
        }
    }
}

