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
using System.Security.Cryptography.X509Certificates;
using AppKit;
using CoreGraphics;
using Foundation;
using VMCA.Client;
using VMCASnapIn.ListViews;
using VMCASnapIn.Nodes;
using VMCASnapIn.Services;
using VmIdentity.UI.Common.Utilities;
using VmIdentity.CommonUtils.Utilities;

namespace VMCASnapIn.UI
{
    [Register ("CustomTableView")]
    public class VMCATableView : NSTableView
    {
        private nint _selectedRow;

        [Foundation.Export ("init")]
        public VMCATableView () : base ()
        {
        }

        [Foundation.Export ("initWithCoder:")]
        public VMCATableView (NSCoder coder) : base (coder)
        {
        }

        public VMCATableView (IntPtr handle) : base (handle)
        {
        }

        public override NSMenu MenuForEvent (NSEvent theEvent)
        {
            UIErrorHelper.CheckedExec (delegate() {
                CGPoint pt = this.ConvertPointFromView (theEvent.LocationInWindow, null);
                _selectedRow = this.GetRow (pt);
                //get datasource and node information
                NSTableViewDataSource ds = (NSTableViewDataSource)this.DataSource;
                NSMenu menu = new NSMenu ();
                if (_selectedRow >= 0) {
                    if (ds is NodesListView) {
                        string data = (ds as NodesListView).Entries [(int)_selectedRow].DisplayName;
                        switch (data) {
                        case "Key Pairs":
                            NSMenuItem createKeyPair = new NSMenuItem ("Create Key Pair", HandleKeyPairRequest); 
                            menu.AddItem (createKeyPair);
                            break;
                        case "Certificates":
                            NSMenuItem createCertificate = new NSMenuItem ("Create Certificate", HandleCreateSelfSignedCertificate);
                            menu.AddItem (createCertificate);
                            break;
                        case "Signing Requests":
                            NSMenuItem createSigningRequest = new NSMenuItem ("Create SigningRequest", HandleCreateSigningRequest);
                            menu.AddItem (createSigningRequest);
                            break;
                        default:
                            break;
                        }
                    } else if (ds is CertificateDetailsListView || ds is PrivateCertsListView) {
                        X509Certificate2 cert = null;
                        if (ds is CertificateDetailsListView) {
                            CertificateDetailsListView lw = ds as CertificateDetailsListView;
                            cert = lw.Entries [(int)_selectedRow];
                            if (lw.CertificateState == (int)VMCA.CertificateState.Active) {
                                NSMenuItem revokeCert = new NSMenuItem ("Revoke Certificate", (object sender, EventArgs e) => {
                                    UIErrorHelper.CheckedExec (delegate() {
                                        VMCACertificateService.RevokeCertificate (cert, lw.ServerDto);
                                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadAll", this);
                                    });
                                });
                                menu.AddItem (revokeCert);
                            }
                        }
                        if (ds is PrivateCertsListView) {
                            cert = CertificateExtensions.GetX509Certificate2FromString ((ds as PrivateCertsListView).Entries [(int)_selectedRow].Certificate);
                        }
                        NSMenuItem showCert = new NSMenuItem ("Show Certificate", (object sender, EventArgs e) => {
                            CertificateService.DisplayX509Certificate2 (this, cert);
                        });
                        menu.AddItem (showCert);
                   
                        NSMenuItem showCertString = new NSMenuItem ("Show Certificate String", (object sender, EventArgs e) => {
                            UIHelper.ShowGenericWindowAsSheet (VMCACertificate.GetCertificateAsString (cert), "Certificate String", VMCAAppEnvironment.Instance.MainWindow);
                        });
                        menu.AddItem (showCertString);
                        /* if (lw.CertificateState == -1) {
                        NSMenuItem deleteString = new NSMenuItem ("Delete Certificate", (object sender, EventArgs e) => {
                            lw.ServerDto.PrivateCertificates.RemoveAll (x => x.Certificate.Equals (cert.ToString ()));
                            NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);

                        });
                        menu.AddItem (deleteString);
                    }*/
                    } else if (ds is CSRDetailListView) {

                        CSRDetailListView lw = ds as CSRDetailListView;
                        var csr = lw.Entries [(int)_selectedRow].CSR;
                        NSMenuItem showCert = new NSMenuItem ("Show CSR", (object sender, EventArgs e) => {
                            UIHelper.ShowGenericWindowAsSheet (csr, "CSR", VMCAAppEnvironment.Instance.MainWindow);
                        });
                        menu.AddItem (showCert);
                    } else if (ds is KeyPairDetailListView) {
                        KeyPairDetailListView lw = ds as KeyPairDetailListView;
                        var privateKey = lw.Entries [(int)_selectedRow].PrivateKey;
                        var publicKey = lw.Entries [(int)_selectedRow].PublicKey;
                        KeyPairData keyPair = new KeyPairData (privateKey, publicKey);
                        NSMenuItem showCert = new NSMenuItem ("Export KeyPair", (object sender, EventArgs e) => {
                            VMCAKeyPairNode.SaveKeyData (keyPair);
                        });
                        menu.AddItem (showCert);
                    }
                    NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
                }

            });
            return base.MenuForEvent (theEvent);
        }


        public void HandleKeyPairRequest (object sender, EventArgs e)
        {
            NodesListView lw = this.DataSource as NodesListView;
            UIErrorHelper.CheckedExec (delegate () {
                ChildScopeNode node = lw.Entries [(int)_selectedRow] as ChildScopeNode;
                (node as VMCAKeyPairNode).HandleCreateKeyPairRequest ();
            });
        }

        public void HandleCreateSelfSignedCertificate (object sender, EventArgs e)
        {
            NodesListView lw = this.DataSource as NodesListView;
            UIErrorHelper.CheckedExec (delegate () {
                ChildScopeNode node = lw.Entries [(int)_selectedRow] as ChildScopeNode;
                (node as VMCAPersonalCertificatesNode).CreateSelfSignedCertificate ();
            });
        }

        public void HandleCreateSigningRequest (object sender, EventArgs e)
        {
            NodesListView lw = this.DataSource as NodesListView;
            UIErrorHelper.CheckedExec (delegate () {
                ChildScopeNode node = lw.Entries [(int)_selectedRow] as ChildScopeNode;
                (node as VMCACSRNode).CreateSigningRequest ();
            });
        }
    }
}

