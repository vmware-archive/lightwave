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
using VmIdentity.UI.Common.Utilities;
using VMCertStoreSnapIn.ListViews;
using VMCertStoreSnapIn.Nodes;
using Foundation;
using AppKit;
using CoreGraphics;
using Vecs;

namespace VMCertStoreSnapIn
{
    [Register ("CustomTableView")]
    public class VMCertStoreTableView : NSTableView
    {
        private nint _selectedRow;

        [Export ("init")]
        public VMCertStoreTableView () : base ()
        {
        }

        [Export ("initWithCoder:")]
        public VMCertStoreTableView (NSCoder coder) : base (coder)
        {
        }

        public VMCertStoreTableView (IntPtr handle) : base (handle)
        {

        }

        public override NSMenu MenuForEvent (NSEvent theEvent)
        {
            CGPoint pt = this.ConvertPointFromView (theEvent.LocationInWindow, null);
            _selectedRow = this.GetRow (pt);
            NSTableViewDataSource ds = (NSTableViewDataSource)this.DataSource;
            NSMenu menu = new NSMenu ();
            if (_selectedRow >= (nint)0) {
                if (ds is NodesListView) {
                    string data = (ds as NodesListView).Entries [(int)_selectedRow].DisplayName;
                    switch (data) {
                    case "Private Entities":
                        NSMenuItem addPrivateEntity = new NSMenuItem ("Add Private Entity", ((ds as NodesListView).Entries [(int)_selectedRow] as VecsPrivateKeysNode).AddPrivateKeyHandler); 
                        menu.AddItem (addPrivateEntity);
                        break;
                    case "Secret Keys":
                        NSMenuItem createCertificate = new NSMenuItem ("Add Secret Key", ((ds as NodesListView).Entries [(int)_selectedRow] as VecsSecretKeysNode).AddSecretKey);
                        menu.AddItem (createCertificate);
                        break;
                    case "Trusted Certs":
                        NSMenuItem createSigningRequest = new NSMenuItem ("Create Certificate", ((ds as NodesListView).Entries [(int)_selectedRow] as VecsTrustedCertsNode).AddCertificate);
                        menu.AddItem (createSigningRequest);
                        break;
                    default:
                        break;
                    }
                } else if (ds is CertificateDetailsListView) {
                    CertificateDetailsListView lw = ds as CertificateDetailsListView;
                    CertDTO cert = lw.Entries [(int)_selectedRow];
                    NSMenuItem showCert = new NSMenuItem ("Show Certificate", (object sender, EventArgs e) => CertificateService.DisplayX509Certificate2 (this, cert.Cert));
                    menu.AddItem (showCert);
                    NSMenuItem deleteEntry = new NSMenuItem ("Delete", (object sender, EventArgs e) => {
                        UIErrorHelper.CheckedExec (delegate() {
                            if (UIErrorHelper.ConfirmDeleteOperation ("Are you sure?") == true) {
                                using (var session = new VecsStoreSession (lw.ServerDto.VecsClient, lw.Store, "")) {
                                    session.DeleteCertificate (cert.Alias);
                                }
                                lw.Entries.Remove (cert);
                                UIErrorHelper.ShowAlert ("", "Successfully deleted the entry.");
                                NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadServerData", this);
                            }
                        });
                    });
                    menu.AddItem (deleteEntry);
                }
				else if (ds is SecretKeysListView)
				{
					SecretKeysListView lw = ds as SecretKeysListView;
					CertDTO cert = lw.Entries[(int)_selectedRow];
					NSMenuItem deleteEntry = new NSMenuItem("Delete", (object sender, EventArgs e) =>
					{
						UIErrorHelper.CheckedExec(delegate ()
						{
							if (UIErrorHelper.ConfirmDeleteOperation("Are you sure?") == true)
							{
								using (var session = new VecsStoreSession(lw.ServerDto.VecsClient, lw.Store, ""))
								{
									session.DeleteCertificate(cert.Alias);
								}
								lw.Entries.Remove(cert);
								UIErrorHelper.ShowAlert("", "Successfully deleted the entry.");
								NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadServerData", this);
							}
						});
					});
					menu.AddItem(deleteEntry);
				}
                NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
            }
            return base.MenuForEvent (theEvent);
        }

    }
}


