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
using VmIdentity.UI.Common;
using VMDir.Common.DTO;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;

namespace VMDirSnapIn.UI
{
    public partial class LdapPropertiesWindowController : NSWindowController
    {
        private String itemName;
        private VMDirServerDTO serverDTO;
        private Dictionary<string,VMDirBagItem> _properties;
        private PropertiesTableViewDataSource ds;

        // Called when created from unmanaged code
        public LdapPropertiesWindowController(IntPtr handle)
            : base(handle)
        {
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public LdapPropertiesWindowController(NSCoder coder)
            : base(coder)
        {
        }

        // Call to load from the XIB/NIB file
        public LdapPropertiesWindowController()
            : base("LdapPropertiesWindow")
        {
        }

        public LdapPropertiesWindowController(String itemName, VMDirServerDTO dto)
            : base("LdapPropertiesWindow")
        {
            this.itemName = itemName;
            this.serverDTO = dto;
            _properties = new Dictionary<string,VMDirBagItem>();
            try
            {
                Utilities.GetItemProperties(itemName, dto, _properties);
                Utilities.RemoveDontShowAttributes(_properties);
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert("", e.Message);
            }
        }

        public IEnumerable<KeyValuePair<string,string>> GetCurrentOptionalProperties()
        {
            return _properties.Where(x => !x.Value.IsRequired).Select(x => new KeyValuePair<string,string>(x.Key, x.Value.Description));
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();

            ds = new PropertiesTableViewDataSource(_properties);
            this.LdapAttributesTableView.DataSource = ds;
            NSTableColumn col;
            col = this.LdapAttributesTableView.TableColumns()[0];
            if (col != null)
                col.DataCell = new NSBrowserCell();
            this.LdapAttributesTableView.Delegate = new MainWindowController.GenericTableDelegate();

            //Events
            this.ManageAttributesButton.Activated += OnClickManageAttributes;
            this.CancelButton.Activated += OnClickCancelButton;
            this.ApplyButton.Activated += OnClickOKButton;
            this.Window.Title = itemName + " Properties";
        }

        private bool DoValidate()
        {
            var propsNotFilled = ds.PendingMod.Where(x =>
                {
                    var val = x;
                    if (val.Value == null)
                        return true;
                    if (val.Value is string)
                        return string.IsNullOrEmpty(val.Value as string);
                    return false;
                });
            if (propsNotFilled.Count() > 0)
            {
                string error = string.Format("{0} is empty", propsNotFilled.First().Key);
                UIErrorHelper.ShowAlert("", error);
                return false;
            }
            return true;
        }

        public void OnClickCancelButton(object sender, EventArgs e)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(0);
        }

        public void OnClickOKButton(object sender, EventArgs e)
        {
            try
            {
                DoValidate();
                if (_properties.Count > 0)
                {
                    LdapMod[] user = new LdapMod[ds.PendingMod.Count + ds.DeleteMod.Count];
                    int i = 0;
                    foreach (var entry in ds.PendingMod)
                    {
                        string[] values = new string[2];
                        values[0] = entry.Value;
                        values[1] = null;
                        user[i] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, entry.Key, values);
                        i++;
                    }
                    foreach (var entry in ds.DeleteMod)
                    {
                        string[] values = new string[2];
                        values[0] = entry.Value;
                        values[1] = null;
                        user[i] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_DELETE, entry.Key, values);
                        i++;
                    }
                    serverDTO.Connection.ModifyObject(itemName, user);
                }
            }
            catch (Exception ex)
            {
                UIErrorHelper.ShowAlert("", ex.Message);
            }
            finally
            {
                this.Close();
                NSApplication.SharedApplication.StopModalWithCode(1);
            }
        }

        public void OnClickManageAttributes(object sender, EventArgs e)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    var optionalProps = GetCurrentOptionalProperties();
                    if (!_properties.ContainsKey("objectClass"))
                        throw new Exception("Unable to fetch data for the object specified. Please ensure the user has access.");
                    var oc = _properties["objectClass"].Value;
                    string cn = "";
                    if (oc is string)
                        cn = oc.ToString();
                    else if (oc is LdapValue[])
                    {
                        LdapValue[] val = oc as LdapValue[];
                        cn = val[(val.Count() - 1)].StringValue; //Get the most derived object class
                    }
                    ManagePropertiesWindowController awc = new ManagePropertiesWindowController(cn, optionalProps, serverDTO);
                    nint result = NSApplication.SharedApplication.RunModalForWindow(awc.Window);
                    if (result == (nint)VMIdentityConstants.DIALOGOK)
                    {
                        var retainList = awc.OptionalAttributes.Intersect(optionalProps);
                        var removeList = optionalProps.Except(retainList).ToList();
                        foreach (var item in removeList)
                            _properties.Remove(item.Key);
                        var addList = awc.OptionalAttributes.Except(retainList);
                        foreach (var item in addList)
                        {
                            var dto = serverDTO.Connection.SchemaManager.GetAttributeType(item.Key);
                            _properties.Add(item.Key, new VMDirBagItem
                                {
                                    Description = dto.Description,
                                    IsReadOnly = dto.ReadOnly,
                                    Value = null
                                });
                        }
                        ds.FillData();
                        this.LdapAttributesTableView.ReloadData(); 
                    }
                });
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }

        //strongly typed window accessor
        public new LdapPropertiesWindow Window
        {
            get
            {
                return (LdapPropertiesWindow)base.Window;
            }
        }
    }
}

