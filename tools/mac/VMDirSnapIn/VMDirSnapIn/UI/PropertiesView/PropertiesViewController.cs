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
using Foundation;
using VMDirSnapIn.DataSource;
using VMDirInterop.LDAP;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;
using AppKit;
using VmIdentity.UI.Common;

namespace VMDirSnapIn.UI
{
	public partial class PropertiesViewController : AppKit.NSViewController
	{
		public PropertiesTableViewDataSource ds;
		#region Constructors

		// Called when created from unmanaged code
		public PropertiesViewController(IntPtr handle) : base(handle)
		{
			Initialize();
		}

		// Called when created directly from a XIB file
		[Export("initWithCoder:")]
		public PropertiesViewController(NSCoder coder) : base(coder)
		{
			Initialize();
		}

		// Call to load from the XIB/NIB file
		public PropertiesViewController() : base("PropertiesView", NSBundle.MainBundle)
		{
			Initialize();
		}

		// Shared initialization code
		void Initialize()
		{
		}

		#endregion

		//strongly typed view accessor
		public new PropertiesView View
		{
			get
			{
				return (PropertiesView)base.View;
			}
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			SetEditVisibility(false);
		}

		partial void PropApplyClick(NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate
			{
				if (ds != null && ds.modData.Count > 0)
				{
					var finalMods = new Dictionary<string, List<string>>();
					foreach (var item in ds.displayAttrDTOList)
					{
						var key = item.Name;
						var val = item.Value;
						if (ds.modData.Contains(key))
						{
							if (finalMods.ContainsKey(key))
							{
								finalMods[key].Add(val);
							}
							else
							{
								finalMods.Add(key, new List<string>() { val });
							}
						}
					}

					ModSubmitConfirmController mscwc = new ModSubmitConfirmController(finalMods);
					nint result = NSApplication.SharedApplication.RunModalForWindow(mscwc.Window);
					if (result != (nint)VMIdentityConstants.DIALOGOK)
					{
						return;
					}

					List<AttributeModStatus> modificationStatus = new List<AttributeModStatus>();
					int i = 0;
					foreach (var m in finalMods)
					{
						LdapMod[] attrMods = new LdapMod[1];
						var values = m.Value.Where(x => !string.IsNullOrWhiteSpace(x)).ToArray();
						Array.Resize(ref values, values.Count() + 1);
						attrMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, m.Key, values);
						try
						{
							ds.serverDTO.Connection.ModifyObject(ds.dn, attrMods);
							modificationStatus.Add(new AttributeModStatus(m.Key, true, "Success"));
						}
						catch (Exception exp)
						{
							modificationStatus.Add(new AttributeModStatus(m.Key, false, exp.Message));
						}
						i++;
					}
					ModSubmitStatusController msswc = new ModSubmitStatusController(modificationStatus);
					NSApplication.SharedApplication.RunModalForWindow(msswc.Window);
					ds.modData.Clear();
					ds.ReloadData();
					ReloadTable();
					SetEditVisibility(false);
				}
			});
		}

		partial void PropResetClick(NSObject sender)
		{
			if (ds != null)
			{
				ds.modData.Clear();
				ds.FillData();
				ReloadTable();
				SetEditVisibility(false);
			}
		}

		partial void AttrAddClick(NSObject sender)
		{
			if (PropTableView.SelectedRowCount == 1)
			{
				var indx = (int)PropTableView.SelectedRow;
				var item = ds.displayAttrDTOList[indx];
				if (!item.AttrSyntaxDTO.SingleValue)
				{
					indx++;
					ds.displayAttrDTOList.Insert(indx, new AttributeDTO(item.Name, string.Empty, item.AttrSyntaxDTO,true));
					ReloadTable();
				}
			}
		}

		public void ReloadTable()
		{
			PropTableView.ReloadData();
		}

		public void SetEditVisibility(bool v)
		{
			PropApply.Hidden = !v;
			PropReset.Hidden = !v;
		}
	}
}
