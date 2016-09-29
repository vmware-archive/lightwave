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
using System.IO;
using System.Security.Cryptography.X509Certificates;
using AppKit;
using VMCA;
using VMCA.Client;
using VMCASnapIn.DTO;
using VMCASnapIn.Services;
using VMCASnapIn.UI;
using VmIdentity.UI.Common.Utilities;

namespace VMCASnapIn.Nodes
{
    public class VMCAServerNode : ChildScopeNode
    {
        public bool IsLoggedIn { get; set; }

        public int NoOfExpiringCert60days { get; set; }

        public int NoOfActiveCerts{ get; set; }

        public int NoOfExpiredCerts{ get; set; }

        public int NoOfRevokedCerts{ get; set; }

        public List<X509Certificate2> ActiveCertsList;
        public List<X509Certificate2> RevokedCertsList;
        public List<X509Certificate2> ExpiredCertsList;

        public VMCAServerNode (VMCAServerDTO dto) : base (dto)
        {
            this.DisplayName = dto.Server;
            IsLoggedIn = false;
            ActiveCertsList = new List<X509Certificate2> ();
            RevokedCertsList = new List<X509Certificate2> ();
            ExpiredCertsList = new List<X509Certificate2> ();
        }

        public void Initialise ()
        {
            try {
                IsLoggedIn = true;
                resetCounters ();
                FillActiveCertsList ();
                FillRevokedCertsList ();
                FillExpiredCertsList ();
                GetCountOfCerts ();
                PopulateChildren ();
            } catch (Exception e) {
                UIErrorHelper.ShowAlert (e.Message, "Error in initialising certificates list");
            }
        }

        public void ClearServerNode ()
        {
            IsLoggedIn = false;
            resetCounters ();
            resetCounters ();
            ServerDTO.IsLoggedIn = false;
            ServerDTO.Password = null;
        }

        void resetCounters ()
        {
            ActiveCertsList.Clear ();
            ExpiredCertsList.Clear ();
            RevokedCertsList.Clear ();
            NoOfActiveCerts = 0;
            NoOfExpiredCerts = 0;
            NoOfRevokedCerts = 0;
            NoOfExpiringCert60days = 0;
        }

        public void GetCountOfCerts ()
        {
            NoOfActiveCerts = ActiveCertsList.Count;
            NoOfExpiredCerts = ExpiredCertsList.Count;
            NoOfRevokedCerts = RevokedCertsList.Count;
            foreach (var cert in ActiveCertsList) {
                if (DateTime.Compare (cert.NotAfter, DateTime.UtcNow.AddDays (60)) < 0)
                    NoOfExpiringCert60days++;
            }
        }

        public void FillActiveCertsList ()
        {
            UIErrorHelper.CatchAndThrow (delegate() {
                using (var context = new VMCAEnumContext (this.ServerDTO.VMCAClient, VMCA.CertificateState.Active)) {
                    foreach (var cert in context.GetCertificates())
                        ActiveCertsList.Add (cert);
                }
            });
        }

        public void FillRevokedCertsList ()
        {
            UIErrorHelper.CatchAndThrow (delegate() {
                using (var context = new VMCAEnumContext (this.ServerDTO.VMCAClient, VMCA.CertificateState.Revoked)) {
                    foreach (var cert in context.GetCertificates())
                        RevokedCertsList.Add (cert);
                }
            });
        }

        public void FillExpiredCertsList ()
        {
            UIErrorHelper.CatchAndThrow (delegate() {
                using (var context = new VMCAEnumContext (this.ServerDTO.VMCAClient, VMCA.CertificateState.Expired)) {
                    foreach (var cert in context.GetCertificates())
                        ExpiredCertsList.Add (cert);
                }
            });
        }

        public void PopulateChildren ()
        {
            this.Children.Clear ();
            AddChildren ();
        }

        void AddChildren ()
        {
            var activeNode = new VMCACertsNode (ServerDTO);
            activeNode.DisplayName = "Active Certificates";
            activeNode.Tag = CertificateState.Active;
            this.Children.Add (activeNode);

            var revokedNode = new VMCACertsNode (ServerDTO);
            revokedNode.DisplayName = "Revoked Certificates";
            revokedNode.Tag = CertificateState.Revoked;
            this.Children.Add (revokedNode);

            var expiredNodes = new VMCACertsNode (ServerDTO);
            expiredNodes.DisplayName = "Expired Certificates";
            expiredNodes.Tag = CertificateState.Expired;
            this.Children.Add (expiredNodes);

            var personalNodes = new VMCAPersonalNode (ServerDTO);
            personalNodes.DisplayName = "Personal Certificates";
            this.Children.Add (personalNodes);
        }


        public void ShowRootCertificate (object sender, EventArgs e)
        {
            ShowRootCertificate ();
        }

        public void ShowRootCertificate ()
        {
            UIErrorHelper.CheckedExec (delegate() {
                var cert = ServerDTO.VMCAClient.GetRootCertificate ();
                CertificateService.DisplayX509Certificate2 (this, cert);
            });
        }

        public void AddRootCertificate (object sender, EventArgs e)
        {
            AddRootCertificate ();
        }

        public void AddRootCertificate ()
        {
            UIErrorHelper.CheckedExec (delegate() {
                var dto = new AddCertificateDTO { PrivateKey = new PrivateKeyDTO () };
                AddCertificateWindowController cwc = new AddCertificateWindowController (dto);
                nint result = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
                if (result == (int)Constants.DIALOGOK) {
                    var cert = File.ReadAllText (dto.Certificate);
                    ServerDTO.VMCAClient.AddRootCertificate (cert, "", dto.PrivateKey.ToString ());
                }
            });
        }


        public void GetServerVersion (object sender, EventArgs e)
        {
            ShowServerVersion ();
        }

        public void ShowServerVersion ()
        {
            UIErrorHelper.CheckedExec (delegate () { 
                UIErrorHelper.ShowAlert (VMCACertificateService.GetVersion (ServerDTO), "Version");
            });
        }
    }
}

