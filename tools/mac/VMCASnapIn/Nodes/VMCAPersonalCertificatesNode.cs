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
using AppKit;
using VMCA;
using VMCASnapIn.DTO;
using VMCASnapIn.Services;
using VMCASnapIn.UI;
using VmIdentity.UI.Common.Utilities;
using Foundation;

namespace VMCASnapIn.Nodes
{
    public class VMCAPersonalCertificatesNode : ChildScopeNode
    {
        public VMCAPersonalCertificatesNode (VMCAServerDTO dto)
            : base (dto)
        {
            DisplayName = "Certificates";
            Tag = -1;
        }

        public void CreateCertificate (object sender, EventArgs args)
        {
            CreateSelfSignedCertificate ();
        }

        public void CreateSelfSignedCertificate ()
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
                        var vmcaCert = request.GetSelfSignedCertificate (dto.PrivateKey.ToString (), dto.NotBefore, dto.NotAfter);
                        var cert = vmcaCert.GetX509Certificate2 ();

                        var localCertDTO = new PrivateCertificateDTO {
                            Certificate = Convert.ToBase64String (cert.RawData)
                        };
                        UIErrorHelper.ShowAlert ("", "Successfully Created A Self Signed Certificate");
                        this.ServerDTO.PrivateCertificates.Add (localCertDTO);
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                        CertificateService.DisplayX509Certificate2 (this, cert);
                    }
                }
            } catch (Exception e) {
                UIErrorHelper.ShowAlert (e.Message, "Operation could not complete successfully.");
            } finally {
                VMCAAppEnvironment.Instance.MainWindow.EndSheet (cwc.Window);
                cwc.Dispose ();
            }
        }

        public void CreateCASignedCertificate (object sender, EventArgs args)
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
                        var vmcaCert = ServerDTO.VMCAClient.GetVMCASignedCertificate (request.GetRequestData(), dto.PrivateKey.ToString (), dto.NotBefore, dto.NotAfter);

                        var localCertDTO = new PrivateCertificateDTO {
                            Certificate = Convert.ToBase64String (vmcaCert.RawData)
                        };
                        UIErrorHelper.ShowAlert ("", "Successfully Created A CA Signed Certificate");
                        this.ServerDTO.PrivateCertificates.Add (localCertDTO);
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                        CertificateService.DisplayX509Certificate2 (this, vmcaCert);
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

