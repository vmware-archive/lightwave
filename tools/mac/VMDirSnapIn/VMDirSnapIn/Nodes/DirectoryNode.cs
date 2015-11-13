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
using System.Linq;
using AppKit;
using Foundation;
using VMDirSnapIn.Nodes;
using VMDirSnapIn.UI;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using System.Collections.Generic;
using VMDir.Common;

namespace VMDirSnapIn.Nodes
{
    public class DirectoryNode: ChildScopeNode
    {
        public enum DirectoryNodeType
        {
            User = 1,
            Groups,
            Computers,
            GenericObject
        }

        public string Name { get; set; }

        bool isChildrenLoaded;

        public bool IsBaseNode { get; set; }

        public DirectoryNodeType NodeType { get; set; }

        private Dictionary<string,VMDirBagItem> _properties;

        public Dictionary<string,VMDirBagItem> NodeProperties {
            get {
                FillProperties ();
                return _properties;
            }
        }

        public DirectoryNode (string dn, VMDirServerDTO dto, ScopeNode parent) : base (dto)
        {
            Name = dn;
            DisplayName = VMDirServerDTO.DN2CN (Name);
            Parent = parent;
            IsBaseNode = false;
            _properties = new Dictionary<string, VMDirBagItem> ();
        }

        public void FillProperties ()
        {
            try {
                Utilities.GetItemProperties (Name, ServerDTO, _properties);
                if (_properties.Count > 0) {
                    VMDirBagItem value = new VMDirBagItem ();
                    if (_properties.ContainsKey ("objectClass"))
                        _properties.TryGetValue ("objectClass", out value);
                    if (value != null) {
                        LdapValue[] valArray = value.Value as LdapValue[];
                        foreach (var ob in valArray) {
                            switch (ob.StringValue) {
                            case "user":
                                NodeType = DirectoryNodeType.User;
                                break;
                            case "group":
                                NodeType = DirectoryNodeType.Groups;
                                break;
                            default:
                                NodeType = DirectoryNodeType.GenericObject;
                                break;
                            }

                        }
                    }
                }
            } catch (Exception ex) {
                UIErrorHelper.ShowAlert ("", ex.Message);
            }
        }

        public void RefreshProperties ()
        {
            _properties.Clear ();
            FillProperties ();
        }

        public void PopulateChildren (string itemDN)
        {
            if (!isChildrenLoaded) {
                ILdapMessage ldMsg = null;
                try {
                    string[] ob = ServerDTO.Connection.SearchAndGetDN (itemDN, LdapScope.SCOPE_ONE_LEVEL, "(objectClass=*)", null, 0, ref ldMsg);
                    int numChildren, i;
                    if (ob != null) {
                        numChildren = ob.Length;
                        for (i = 0; i < numChildren; i++) {
                            this.Children.Add (new DirectoryNode (ob [i], ServerDTO, this));
                        }
                    } else {
                        this.Children = null;
                    }
                    isChildrenLoaded = true;
                    //NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
                } catch (Exception e) {
                    throw e;
                }
            }
        }

        public void ReloadChildren ()
        {
            UIErrorHelper.CheckedExec (delegate() {

                if (this.Children != null) {
                    this.Children.Clear ();
                    isChildrenLoaded = false;
                    PopulateChildren (Name);
                }
            });
        }

        //Events
        public void RefreshNode (object sender, EventArgs e)
        {
            RefreshProperties ();
            ReloadChildren ();
        }

        public void Add (object sender, EventArgs e)
        {
            ShowAddWindow ();
        }

        public void Delete (object sender, EventArgs e)
        {
            PerformDelete ();
        }

        public void ViewProperties (object sender, EventArgs e)
        {
            ShowPropertiesWindow ();
        }

        public void AddUser (object sender, EventArgs e)
        {
            ShowAddUser ();
        }

        public void AddGroup (object sender, EventArgs e)
        {
            ShowAddGroup ();
        }

        //Launch Dialogs
        public void AddUserToGroup (object sender, EventArgs e)
        {
            AddGroupByCNWindowController gwc = new AddGroupByCNWindowController (ServerDTO);
            nint result = NSApplication.SharedApplication.RunModalForWindow (gwc.Window);
            if (result == (nint)VMIdentityConstants.DIALOGOK) {
                UIErrorHelper.CheckedExec (delegate() {
                    string[] values = new string[2];
                    values [1] = null;
                    values [0] = Name;
                    LdapMod[] ldapVal = new LdapMod[1];
                    ldapVal [0] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, "member", values); 
                    ServerDTO.Connection.ModifyObject (gwc.DNText, ldapVal);
                    UIErrorHelper.ShowAlert ("", "Successfully Added Member");
                    ReloadChildren ();
                    RefreshProperties ();
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                });
            }
        }

        public void ShowAddWindow ()
        {
            SelectObjectClassWindowController swc = new SelectObjectClassWindowController (ServerDTO.Connection.SchemaManager);
            nint result = NSApplication.SharedApplication.RunModalForWindow (swc.Window);
            if (result == (nint)VMIdentityConstants.DIALOGOK) {
                CreateObjectWindowController cwc = new CreateObjectWindowController (swc.SelectedObject, ServerDTO);
                nint res = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
                if (res == (nint)VMIdentityConstants.DIALOGOK) {
                    UIErrorHelper.CheckedExec (delegate() {
                        if (cwc._properties.Count > 0) {
                            LdapMod[] user = new LdapMod[cwc._properties.Count];
                            int count = 0;
                            foreach (var entry in cwc._properties) {
                                string[] values = new string[2];
                                values [0] = Utilities.LdapValueToString (entry.Value.Value);
                                if (string.IsNullOrEmpty (values [0]))
                                    continue;
                                values [1] = null;
                                user [count] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, entry.Key, values);
                                count++;
                            }
                            string dn = string.Format ("cn={0},{1}", Utilities.LdapValueToString (cwc._properties.First (x => x.Key == "cn").Value.Value), Name);
                            ServerDTO.Connection.AddObject (dn, user);
                            UIErrorHelper.ShowAlert ("", "Successfully Added object");
                            ReloadChildren ();
                            RefreshProperties ();
                            NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
                            NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                        }
                    });
                }
                VMDirSnapInEnvironment.Instance.MainWindow.EndSheet (cwc.Window);
                swc.Dispose ();
            }
        }

        public void ShowAddGroup ()
        {
            GroupDTO dto = new GroupDTO ();
            AddNewGroupController agc = new AddNewGroupController (dto);
            nint res = NSApplication.SharedApplication.RunModalForWindow (agc.Window);
            if (res == (nint)VMIdentityConstants.DIALOGOK) {
                UIErrorHelper.CheckedExec (delegate() {
                    LdapMod[] user = new LdapMod[4];
                    user [0] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] {
                        dto.cn,
                        null
                    });
                    user [1] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_GROUPTYPE, new string[] {
                        dto.groupType.ToString (),
                        null
                    });
                    user [2] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SAM_ACCOUNT_NAME, new string[] {
                        dto.sAMAccountName,
                        null
                    });
                    user [3] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] {
                        dto.objectClass,
                        null
                    });
                    string dn = string.Format ("cn={0},{1}", dto.cn, Name);
                    ServerDTO.Connection.AddObject (dn, user);
                    UIErrorHelper.ShowAlert ("", "Successfully added object");
                    ReloadChildren ();
                    RefreshProperties ();
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                });
            }
        }

        public void ShowAddUser ()
        {
            AddNewUserDTO userDTO = new AddNewUserDTO ();
            AddNewUserController awc = new AddNewUserController (userDTO);
            nint res = NSApplication.SharedApplication.RunModalForWindow (awc.Window);
            if (res == (nint)VMIdentityConstants.DIALOGOK) {
                UIErrorHelper.CheckedExec (delegate() {
                    LdapMod[] user = new LdapMod[6];
                    user [0] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_GIVEN_NAME, new string[] {
                        userDTO.FirstName,
                        null
                    });
                    user [1] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SN, new string[] {
                        userDTO.LastName,
                        null
                    });
                    user [2] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] {
                        userDTO.Cn,
                        null
                    });
                    user [3] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_KRB_UPN, new string[] {
                        userDTO.UPN,
                        null
                    });
                    user [4] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SAM_ACCOUNT_NAME, new string[] {
                        userDTO.SAMAccountName,
                        null
                    });
                    user [5] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] {
                        "user",
                        null
                    });
                    string dn = string.Format ("cn={0},{1}", userDTO.Cn, Name);
                    ServerDTO.Connection.AddObject (dn, user);
                    UIErrorHelper.ShowAlert ("", "Successfully added object");
                    ReloadChildren ();
                    RefreshProperties ();
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                });

            }
        }

        public void PerformDelete ()
        {
            ConfirmationDialogController cwc = new ConfirmationDialogController ("Are you sure?");
            nint result = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
            if (result == (nint)VMIdentityConstants.DIALOGOK) {
                UIErrorHelper.CheckedExec (delegate () {
                    ServerDTO.Connection.DeleteObject (Name);
                    ScopeNode node = this.Parent;
                    if (node != null) {
                        node.Children.Remove (this);
                        if (node is DirectoryNode)
                            (node as DirectoryNode).ReloadChildren ();
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", node);
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", node);
                        UIErrorHelper.ShowAlert ("", "Successfully deleted object");
                    } else {
                        UIErrorHelper.ShowAlert ("", "Deleted base object. Please Refresh the Server");
                    }
                });
            }
        }

        public void RestUserPassword (object sender, EventArgs e)
        {
            ResetPasswordWindowController cwc = new ResetPasswordWindowController ();
            nint result = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
            if (result == (nint)VMIdentityConstants.DIALOGOK) {
                UIErrorHelper.CheckedExec (delegate() {
                    LdapMod[] mod = new LdapMod[1];
                    mod [0] = new LdapMod ((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.ATTR_USER_PASSWORD, new string[] {
                        cwc.Password,
                        null
                    });
                    ServerDTO.Connection.ModifyObject (Name, mod);
                    UIErrorHelper.ShowAlert ("Successfully reset password for the object", "Info");
                    ReloadChildren ();
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                });
            }
        }

        public void ShowPropertiesWindow ()
        {
            LdapPropertiesWindowController awc = new LdapPropertiesWindowController (Name, ServerDTO);
            nint result = NSApplication.SharedApplication.RunModalForWindow (awc.Window);
            if (result == (nint)VMIdentityConstants.DIALOGOK) {
                ReloadChildren ();
                RefreshProperties ();
                NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
            }
        }
    }
}

