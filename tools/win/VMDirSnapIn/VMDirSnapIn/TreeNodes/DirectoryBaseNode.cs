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
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMDirSnapIn.Utilities;
using VMDirSnapIn.UI;
using VMDirSnapIn.Views;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;
using System.Linq;
using VmdirUtil = VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.TreeNodes
{
    public class DirectoryBaseNode : BaseTreeNode
    {
        public string Dn { get; private set; }
        public string Cn { get; private set; }
        public List<string> ObjectClass { get; private set; }

        private Dictionary<string, VMDirAttributeDTO> _properties;

        public Dictionary<string, VMDirAttributeDTO> NodeProperties
        {
            get
            {
                if (_properties == null)
                    FillProperties();
                return _properties;
            }
            set
            {
                _properties = value;
            }
        }

        public DirectoryBaseNode(string dn, List<string> oc, VMDirServerDTO serverDTO, PropertiesControl propCtl)
            : base(serverDTO, propCtl)
        {
            this.Dn = dn;
            this.ObjectClass = oc;
            Cn = VMDirServerDTO.DN2CN(dn);
            ImageIndex = SelectedImageIndex = MiscUtilsService.GetImgIndx(ObjectClass);
            this.Tag = "directory";
        }

        private void FillProperties()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                TextQueryDTO dto = new TextQueryDTO(Dn, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC, null, 0, IntPtr.Zero, 0);
                ServerDTO.Connection.Search(dto,
                    (l, e) =>
                    {
                        if (e.Count > 0)
                            _properties = ServerDTO.Connection.GetEntryProperties(e[0]);
                    });
            });
        }
        public void RefreshProperties()
        {
            _properties.Clear();
            FillProperties();
            DoSelect();
        }

        public override void DoSelect()
        {
            PropertiesCtl.Visible = true;
            var oc = string.Empty;
            if (ObjectClass.Count > 0)
                oc = ObjectClass[ObjectClass.Count - 1];
            PropertiesCtl.Init(Dn, oc, ServerDTO, NodeProperties);
        }
        public void Delete()
        {
            ServerDTO.Connection.DeleteObject(Dn);
        }

        public void AddUserToGroup()
        {
            var frm = new AddToGroup(ServerDTO);
            if (frm.ShowDialog() == DialogResult.OK)
            {
                MiscUtilsService.CheckedExec(delegate()
                {
                    string[] values = new string[2];
                    values[1] = null;
                    values[0] = Dn;
                    LdapMod[] ldapVal = new LdapMod[1];
                    ldapVal[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_MEMBER, values);
                    ServerDTO.Connection.ModifyObject(frm.DNText, ldapVal);
                    MMCDlgHelper.ShowInformation(VMDirConstants.STAT_MEMBER_ADD_SUCC);
                    RefreshProperties();
                });
            }
        }

        public void ResetPassword()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                var frm = new ResetUserPwdForm(Dn);
                if (frm.ShowDialog() == DialogResult.OK)
                {
                    LdapMod[] mod = new LdapMod[1];
                    mod[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.ATTR_USER_PASSWORD, new string[] {
                        frm.Password,
                        null
                    });
                    ServerDTO.Connection.ModifyObject(frm.Dn, mod);
                    MMCDlgHelper.ShowInformation(VMDirConstants.STAT_PWD_RESET_SUCC);
                }
            });
        }

        public void VerifyPassword()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                var frm = new CheckUserPwdForm(VmdirUtil.Utilities.GetAttrLastVal(NodeProperties, VMDirConstants.ATTR_KRB_UPN));
                if (frm.ShowDialog() == DialogResult.OK)
                {
                    VMDirServerDTO ser = new VMDirServerDTO();
                    ser.Server = ServerDTO.Server;
                    ser.Password = frm.Password;
                    ser.BindDN = frm.UPN;
                    ser.Connection = new VmdirUtil.LdapConnectionService(ser.Server, ser.BindDN, ser.Password);
                    if (ser.Connection.CheckCredentials())
                        MMCDlgHelper.ShowInformation(CommonConstants.CORRECT_PWD);
                    else
                        MMCDlgHelper.ShowInformation(CommonConstants.INVALID_PWD);
                }
            });
        }
    }
}
