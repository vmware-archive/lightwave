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
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMDirSnapIn.UI;
using VMDirSnapIn.Utilities;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;
using VMDirUtil = VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.TreeNodes
{
    public class DirectoryExpandableNode : DirectoryBaseNode
    {
        private IntPtr _cookie;
        private int _totalCount;
        private bool _morePages;
        private int _pageNumber;
        private QueryDTO _qdto;
        public DirectoryExpandableNode(string dn, List<string> oc, VMDirServerDTO serverDTO, PropertiesControl propCtl)
            : base(dn, oc, serverDTO, propCtl)
        {
            this.Text = Cn + " ...";
            AddDummyNode();
            InitPageSearch();
        }

        public override void DoExpand()
        {
            if (Nodes.Count == 1 && Nodes[0].Tag == null)
            {
                this.Nodes.Clear();
                InitPageSearch();
                GetPage();
                Expand();
            }
        }
        public override void DoRefresh()
        {
            this.Nodes.Clear();
            AddDummyNode();
            DoExpand();
            RefreshProperties();
        }

        public void Search()
        {
            Thread t = new Thread(DoSearch);
            t.SetApartmentState(ApartmentState.STA);
            t.Start();
        }

        private void DoSearch()
        {
            var frm = new SearchForm(Dn, ServerDTO);
            var length = this.Dn.Length > 20 ? 20 : this.Dn.Length;
            frm.Text = "Server: " + ServerDTO.Server + "           Search In: " + this.Dn.Substring(0, length) + "...";
            Application.Run(frm);
            frm.BringToFront();
        }
        protected void InitPageSearch()
        {
            _qdto = new TextQueryDTO(Dn, LdapScope.SCOPE_ONE_LEVEL, VMDirConstants.SEARCH_ALL_OC,
       new string[] { VMDirConstants.ATTR_DN, VMDirConstants.ATTR_OBJECT_CLASS }, 0, IntPtr.Zero, 0);
            _cookie = IntPtr.Zero;
            _totalCount = 0;
            _pageNumber = 1;
            _morePages = true;
        }
        internal void GetNextPage()
        {
            if (_morePages)
            {
                GetPage();
            }
            else
                MMCDlgHelper.ShowInformation(VMDirConstants.WRN_NO_MORE_PAGES);
        }
        private void GetPage()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                var _lst = new List<DirectoryExpandableNode>();
                ServerDTO.Connection.PagedSearch(_qdto, ServerDTO.PageSize, _cookie, _morePages,
                    delegate(ILdapMessage ldMsg, IntPtr ck, bool moreP, List<ILdapEntry> entries)
                    {
                        _cookie = ck;
                        _morePages = moreP;
                        _totalCount += entries.Count();
                        _pageNumber++;
                        foreach (var entry in entries)
                        {
                            var ocList = new List<string>(entry.getAttributeValues(VMDirConstants.ATTR_OBJECT_CLASS).Select(x=>x.StringValue).ToArray());
                            _lst.Add(new DirectoryExpandableNode(entry.getDN(), ocList, ServerDTO, PropertiesCtl));
                        }
                    });
                if (!_morePages)
                    this.Text = Cn;
                this.Nodes.AddRange(_lst.ToArray());
            });
        }

        public void AddObject()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                var frmSelect = new SelectObjectClass(ServerDTO.Connection.SchemaManager);
                if (frmSelect.ShowDialog() == DialogResult.OK)
                {
                    var frm = new CreateForm(frmSelect.SelectedObject, ServerDTO, Dn);
                    if (frm.ShowDialog() == DialogResult.OK)
                    {
                        var attributes = frm.Attributes.Select(x => LdapTypesService.MakeAttribute(x)).ToArray();   
                        string newdn = frm.Rdn+","+Dn;
                        ServerDTO.Connection.AddObject(newdn, attributes);
                        ClearDummyNode();
                        var oc = VMDirUtil.Utilities.GetObjectClassList(ServerDTO, newdn, LdapScope.SCOPE_BASE);
                        this.Nodes.Insert(0, new DirectoryExpandableNode(newdn, oc, ServerDTO, PropertiesCtl));
                        MMCDlgHelper.ShowInformation(VMDirConstants.STAT_OBJ_ADD_SUCC);
                    }
                }
            });
        }
        public void AddGroup()
        {
            GroupDTO dto = new GroupDTO();
            var frm = new AddGroup(dto);
            if (frm.ShowDialog() == DialogResult.OK)
            {
                MiscUtilsService.CheckedExec(delegate()
                {
                    LdapMod[] user = new LdapMod[4];
                    user[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] {
						dto.cn,
						null
					});
                    user[1] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_GROUPTYPE, new string[] {
						dto.groupType.ToString (),
						null
					});
                    user[2] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SAM_ACCOUNT_NAME, new string[] {
						dto.sAMAccountName,
						null
					});
                    user[3] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] {
						dto.objectClass,
						null
					});
                    string dn = string.Format("cn={0},{1}", dto.cn, Dn);
                    ServerDTO.Connection.AddObject(dn, user);
                    ClearDummyNode();
                    var oc = VMDirUtil.Utilities.GetObjectClassList(ServerDTO,dn,LdapScope.SCOPE_BASE);
                    this.Nodes.Insert(0,new DirectoryExpandableNode(dn, oc, ServerDTO, PropertiesCtl));
                    MMCDlgHelper.ShowInformation(VMDirConstants.STAT_GRP_ADD_SUCC);
                });
            }
        }

        public void AddUser()
        {
            UserDTO userDTO = new UserDTO();
            var frm = new AddUser(userDTO);
            if (frm.ShowDialog() == DialogResult.OK)
            {
                MiscUtilsService.CheckedExec(delegate()
                {
                    LdapMod[] user = new LdapMod[6];
                    user[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_GIVEN_NAME, new string[] {
						userDTO.FirstName,
						null
					});
                    user[1] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SN, new string[] {
						userDTO.LastName,
						null
					});
                    user[2] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] {
						userDTO.Cn,
						null
					});
                    user[3] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_KRB_UPN, new string[] {
						userDTO.UPN,
						null
					});
                    user[4] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SAM_ACCOUNT_NAME, new string[] {
						userDTO.SAMAccountName,
						null
					});
                    user[5] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] {
						VMDirConstants.USER_OC,
						null
					});
                    string dn = string.Format("cn={0},{1}", userDTO.Cn, Dn);
                    ServerDTO.Connection.AddObject(dn, user);
                    ClearDummyNode();
                    var oc = VMDirUtil.Utilities.GetObjectClassList(ServerDTO, dn, LdapScope.SCOPE_BASE);
                    this.Nodes.Insert(0,new DirectoryExpandableNode(dn, oc, ServerDTO, PropertiesCtl));
                    MMCDlgHelper.ShowInformation(VMDirConstants.STAT_USR_ADD_SUCC);
                });
            }
        }
    }
}
