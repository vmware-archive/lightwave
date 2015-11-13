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
using VMDirSnapIn.UI;
using VmIdentity.UI.Common.Utilities;
using VmIdentity.UI.Common;
using VMDir.Common.DTO;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.UI
{
    public partial class CreateObjectWindowController : AppKit.NSWindowController
    {
        private string _objectClass;
        private VMDirServerDTO _serverDTO;
        public Dictionary<string,VMDirBagItem> _properties;
        private CreateObjectTableViewDataSource ds;

        #region Constructors

        // Called when created from unmanaged code
        public CreateObjectWindowController (IntPtr handle) : base (handle)
        {
        }
        
        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public CreateObjectWindowController (NSCoder coder) : base (coder)
        {
        }
        
        // Call to load from the XIB/NIB file
        public CreateObjectWindowController () : base ("CreateObjectWindow")
        {
        }

        // Call to load from the XIB/NIB file
        public CreateObjectWindowController (string objectClass, VMDirServerDTO serverDTO) : base ("CreateObjectWindow")
        {
            _objectClass = objectClass;
            _serverDTO = serverDTO;
            Bind ();
            Utilities.RemoveDontShowAttributes (_properties);
        }

        #endregion

        private void Bind ()
        {
            var requiredProps = _serverDTO.Connection.SchemaManager.GetRequiredAttributesWithContentRules (_objectClass);
            _properties = new Dictionary<string, VMDirBagItem> ();
            foreach (var prop in requiredProps) {
                var item = new VMDirBagItem {
                    Value = prop.Name.Equals ("objectClass") ? (Utilities.StringToLdapValue (_objectClass)) : null,
                    Description = prop.Description,
                    IsRequired = true,
                    IsReadOnly = prop.ReadOnly
                };
                _properties.Add (prop.Name, item);

            }

            VMDirBagItem itemCN = null;
            if (!_properties.TryGetValue ("cn", out itemCN)) {
                _properties.Add ("cn", new VMDirBagItem{ Value = null, Description = "", IsRequired = true });
            }
        }

        public IEnumerable<KeyValuePair<string,string>> GetCurrentOptionalProperties ()
        {
            return _properties.Where (x => !x.Value.IsRequired).Select (x => new KeyValuePair<string,string> (x.Key, x.Value.Description));
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
            try {
                ds = new CreateObjectTableViewDataSource (_properties);
                this.PropertiesTableView.DataSource = ds;
                NSTableColumn col;
                col = this.PropertiesTableView.TableColumns () [0];
                if (col != null)
                    col.DataCell = new NSBrowserCell ();
                this.PropertiesTableView.Delegate = new MainWindowController.GenericTableDelegate ();

                this.ManageAttributesButton.Activated += OnClickManageAttributesButton;
                this.CreateButton.Activated += OnClickCreateButton;
                this.CancelButton.Activated += OnClickCancelButton;
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine ("Error " + e.Message);
            }
        }

        private void OnClickCancelButton (object sender, EventArgs e)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (0);
        }

        private void OnClickCreateButton (object sender, EventArgs e)
        {
            if (DoValidate ()) {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (1);
            }
        }

        private void OnClickManageAttributesButton (object sender, EventArgs e)
        {
            try {
                var optionalProps = GetCurrentOptionalProperties ();
                if (!_properties.ContainsKey ("objectClass"))
                    throw new Exception ("Unable to fetch data for the object specified. Please ensure the user has access.");
                var oc = _properties ["objectClass"].Value;
                string cn = "";
                if (oc is string)
                    cn = oc.ToString ();
                else if (oc is List<string>)
                    cn = (oc as List<string>) [0];
                ManagePropertiesWindowController awc = new ManagePropertiesWindowController (_objectClass, optionalProps, _serverDTO);
                nint result = NSApplication.SharedApplication.RunModalForWindow (awc.Window);
                if (result == (nint)VMIdentityConstants.DIALOGOK) {
                    var retainList = awc.OptionalAttributes.Intersect (optionalProps);
                    var removeList = optionalProps.Except (retainList).ToList ();
                    foreach (var item in removeList)
                        _properties.Remove (item.Key);
                    var addList = awc.OptionalAttributes.Except (retainList);
                    foreach (var item in addList) {
                        var dto = _serverDTO.Connection.SchemaManager.GetAttributeType (item.Key);
                        _properties.Add (item.Key, new VMDirBagItem {
                            Description = dto.Description,
                            IsReadOnly = dto.ReadOnly,
                            Value = null
                        });
                    }
                    this.PropertiesTableView.ReloadData ();
                }
            } catch (Exception ex) {
                UIErrorHelper.ShowAlert ("", ex.Message);
            }
        }

        private bool DoValidate ()
        {
            var requiredPropsNotFilled = _properties.Where (x => {
                var val = x.Value;
                if (val.Value == null)
                    return true;
                if (val.Value is string)
                    return string.IsNullOrEmpty (val.Value as string);
                else if (val.Value is List<string>)
                    return (val.Value as List<string>).Count == 0;
                return false;
            });
            if (requiredPropsNotFilled.Count () > 0) {
                string error = string.Format ("{0} is empty", requiredPropsNotFilled.First ().Key);
                UIErrorHelper.ShowAlert ("", error);
                return false;
            }
            return true;
        }

        [Export ("windowWillClose:")]
        public void WindowWillClose (NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal ();
        }

        //strongly typed window accessor
        public new CreateObjectWindow Window {
            get {
                return (CreateObjectWindow)base.Window;
            }
        }
    }
}

