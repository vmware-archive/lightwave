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
using VMCASnapIn.Services;
using VMCASnapIn.UI;
using AppKit;
using VMCA;
using VmIdentity.UI.Common.Utilities;
using Foundation;
using VmIdentity.UI.Common;

namespace VMCASnapIn.Nodes
{
    public class VMCACSRNode : ChildScopeNode
    {
        public VMCACSRNode (VMCAServerDTO dto) : base (dto)
        {
            DisplayName = "Signing Requests";
        }

        public void HandleSigningRequest (object sender, EventArgs e)
        {
            CreateSigningRequest ();
        }

        public void CreateSigningRequest ()
        {
            var dto = new CertRequestDTO ();
            CreateCertificateWindowController cwc = new CreateCertificateWindowController (dto);
            NSApplication.SharedApplication.BeginSheet (cwc.Window, VMCAAppEnvironment.Instance.MainWindow, () => {
            });
            try {
                nint result = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
                if (result == (nint)Constants.DIALOGOK) {
                    using (var request = new VMCARequest (this.ServerDTO.VMCAClient)) {
                        dto.FillRequest (request);
                        string csr = request.GetCSR (dto.PrivateKey.ToString ());
                        this.ServerDTO.SigningRequests.Add (new SigningRequestDTO {
                            CreatedDateTime = DateTime.UtcNow,
                            CSR = csr
                        });
                        GenericTextViewWindowController gwc = new GenericTextViewWindowController (csr);
                        gwc.Window.Title = "CSR Data";
                        NSApplication.SharedApplication.RunModalForWindow (gwc.Window);
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                    }
                }
            } catch (Exception e) {
                UIErrorHelper.ShowAlert (e.Message, "Operation could not complete successfully.");
            } finally {
                VMCAAppEnvironment.Instance.MainWindow.EndSheet (cwc.Window);
                cwc.Dispose ();
            }
        }
    }
}

