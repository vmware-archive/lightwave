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
using Microsoft.ManagementConsole;
using VMCASnapIn.DTO;
using VMCASnapIn.ListViews;
using VMCASnapIn.UI;
using System.Windows.Forms;
using VMCASnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.Nodes
{
    public class VMCAKeyPairNode : ChildScopeNode
    {
        const int ACTION_CREATE_KEY_PAIR = 1;

        public VMCAKeyPairNode(VMCAServerDTO dto)
            : base(dto)
        {
            DisplayName = "Key Pairs";
            Tag = -1;

            this.EnabledStandardVerbs = StandardVerbs.Refresh;
            ImageIndex = SelectedImageIndex = (int)VMCAImageIndex.KeyPairs;
            InitNode();
        }

        void InitNode()
        {
            MmcListViewDescription lvd = new MmcListViewDescription();
            lvd.DisplayName = "keypair details";
            lvd.ViewType = typeof(KeyPairDetailListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;
            ViewDescriptions.Add(lvd);
            ViewDescriptions.DefaultIndex = 0;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Create Key Pair",
                                       "Create Key Pair", -1, ACTION_CREATE_KEY_PAIR));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_CREATE_KEY_PAIR:
                    if (CreateKeyPair())
                        OnRefresh(status);
                    break;
            }
        }

        bool CreateKeyPair()
        {
            bool bResult = false;
            MMCActionHelper.CheckedExec(delegate()
            {
                var frm = new CreateKeyPairForm();
                if (MMCDlgHelper.ShowForm(frm))
                {
                    ServerDTO.KeyPairs.Add(frm.DTO);
                    bResult = true;
                    VMCASnapInEnvironment.Instance.SaveLocalData();
                }
            });
            return bResult;
        }

        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
            OnRefresh(status);
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            if (ListView != null)
            {
                var list = ListView as KeyPairDetailListView;
                list.RefreshList();
            }
        }
    }
}
