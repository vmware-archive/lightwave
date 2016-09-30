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
using VMCASnapIn.DTO;
using VMCASnapIn.UI;
using AppKit;
using VMCASnapIn.Services;
using VmIdentity.UI.Common.Utilities;
using Foundation;
using VMCA.Client;
using System.IO;

namespace VMCASnapIn.Nodes
{
    public class VMCAKeyPairNode : ChildScopeNode
    {
        public VMCAKeyPairNode (VMCAServerDTO dto) : base (dto)
        {
            DisplayName = "Key Pairs";
            Tag = -1;
        }

        public void CreateKeyPair (object sender, EventArgs eventargs)
        {
            HandleCreateKeyPairRequest ();
        }

        public void HandleCreateKeyPairRequest ()
        {
            CreateKeyPairWindowController cwc = new CreateKeyPairWindowController ();

            NSApplication.SharedApplication.BeginSheet (cwc.Window, VMCAAppEnvironment.Instance.MainWindow, () => {
            });
            nint result = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
            try {
                if (result == (nint)Constants.DIALOGOK) {
                    UIErrorHelper.ShowAlert ("", Constants.CREATED_KEY_PAIR);
                    this.ServerDTO.KeyPairs.Add (cwc.DTO);
                    NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                }
            } finally {
                VMCAAppEnvironment.Instance.MainWindow.EndSheet (cwc.Window);
                cwc.Dispose ();
            }
        }

        public  static void SaveKeyData (KeyPairData data)
        {
            UIErrorHelper.CheckedExec (delegate () {
                var save = NSSavePanel.SavePanel;
                save.AllowedFileTypes = new string[] { "key" };
                save.Title = Constants.SAVE_PUBLIC_KEY;
                nint result = save.RunModal ();
                if (result == (int)1) {
                    string path = save.Url.Path;
                    File.WriteAllText (path, data.PrivateKey);
                    UIErrorHelper.ShowAlert ("", Constants.SAVED_PUBLIC_KEY);
                }

                save.Title = Constants.SAVE_PRIVATE_KEY;
                result = save.RunModal ();
                if (result == (int)1) {
                    string path = save.Url.Path;
                    File.WriteAllText (path, data.PublicKey);
                    UIErrorHelper.ShowAlert ("", Constants.SAVED_PRIVATE_KEY);
                }
            });
        }
    }
}

