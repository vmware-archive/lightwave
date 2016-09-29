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
using CoreGraphics;
using AppKit;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;

namespace RestSsoAdminSnapIn
{
	[Register ("CustomOutlineView")]
	public class TreeView : NSOutlineView
	{
		[Foundation.Export ("init")]
		public TreeView ()
		{
		}

		[Foundation.Export ("initWithCoder:")]
		public TreeView (NSCoder coder) : base (coder)
		{
		}

		public TreeView (IntPtr handle) : base (handle)
		{
		}

		public override NSMenu MenuForEvent (NSEvent theEvent)
		{
			CGPoint pt = this.ConvertPointToView (theEvent.LocationInWindow, null);
			int row = (int)this.GetRow (pt);
			if (row >= 0) {
				NSObject obj = this.ItemAtRow (row);
				if (obj != null) {
					NSMenu menu = new NSMenu ();
					menu.Font = NSFont.UserFontOfSize((float)12.0);
					if (obj is ServerNode) {
						ServerNode serverNode = obj as ServerNode;
						if (serverNode.IsLoggedIn) {

							NSMenuItem addNewTenant = new NSMenuItem ("Add New Tenant", serverNode.OnAddNewTenant);
							var image = NSImage.ImageNamed ("NSAddTemplate");
							addNewTenant.Image = image;
							addNewTenant.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
							menu.AddItem (addNewTenant);

							NSMenuItem aboutServer = new NSMenuItem ("About", serverNode.OnShowAbout);
							image = NSImage.ImageNamed ("NSInfo");
							aboutServer.Image = image;
							aboutServer.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
							menu.AddItem (aboutServer);

							var enable = serverNode != null && serverNode.Children.Count > 0 && ((TenantNode)serverNode.Children [0]).IsSystemTenant;

							if (enable) {
								NSMenuItem getComputers = new NSMenuItem ("Computers", serverNode.OnShowComputers);
								image = NSImage.ImageNamed ("NSComputer");
								getComputers.Image = image;
								getComputers.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
								menu.AddItem (getComputers);
							}

							NSMenuItem tokenWizard = new NSMenuItem ("Diagnostics", serverNode.ShowTokenWizard);
							image = NSImage.ImageNamed ("NSSmartBadgeTemplate");
							tokenWizard.Image = image;
							tokenWizard.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
							menu.AddItem (tokenWizard);
						} 
					} else if (obj is TenantNode) {
						TenantNode tenantNode = obj as TenantNode;
						NSMenuItem showConfig = new NSMenuItem ("Tenant Configuration", tenantNode.ShowConfiguration);
						var image = NSImage.ImageNamed ("config.png");
						showConfig.Image = image;
						showConfig.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (showConfig);

					} else if (obj is IdentitySourcesNode) {
						IdentitySourcesNode identitySources = obj as IdentitySourcesNode;

						NSMenuItem refreshSource = new NSMenuItem ("Refresh", identitySources.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						refreshSource.Image = image;
						refreshSource.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (refreshSource);
					}
					else if (obj is IdentitySourceNode) {
						IdentitySourceNode identitySourceNode = obj as IdentitySourceNode;

						NSMenuItem refreshSource = new NSMenuItem ("Refresh", identitySourceNode.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						refreshSource.Image = image;
						refreshSource.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (refreshSource);

						if (!identitySourceNode.IsDefaultDomain) {
							NSMenuItem setAsDefault = new NSMenuItem ("Set as default domain", identitySourceNode.SetAsDefault);
							menu.AddItem (setAsDefault);
						}
					}
					else if (obj is UsersNode) {
						UsersNode node = obj as UsersNode;
						NSMenuItem item = null;
						if (node.IsSystemDomain) {
							item = new NSMenuItem ("Add New User", node.AddNewUser);
							var image1 = NSImage.ImageNamed ("Add_User_64.png");
							item.Image = image1;
							item.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
							menu.AddItem (item);
						}
						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					else if (obj is SolutionUsersNode) {
						SolutionUsersNode node = obj as SolutionUsersNode;
						NSMenuItem item = null;

						if (node.IsSystemDomain) {
							item = new NSMenuItem ("Add New Solution User", node.AddNewUser);
							var image1 = NSImage.ImageNamed ("Add_User_64.png");
							item.Image = image1;
							item.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
							menu.AddItem (item);
						}

						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					else if (obj is GroupsNode) {
						GroupsNode node = obj as GroupsNode;
						NSMenuItem item = null;

						if (node.IsSystemDomain) {
							item = new NSMenuItem ("Add New Group", node.AddNewGroup);
							var image1 = NSImage.ImageNamed ("Add_Group_64.png");
							item.Image = image1;
							item.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
							menu.AddItem (item);
						}

						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					else if (obj is RelyingPartyNode) {
						RelyingPartyNode node = obj as RelyingPartyNode;
						NSMenuItem item = null;

						item = new NSMenuItem ("Add Relying Party", node.AddRelyingParty);
						var image1 = NSImage.ImageNamed ("NSAddTemplate");
						item.Image = image1;
						item.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);

						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					} else if (obj is OidcClientNode) {
						OidcClientNode node = obj as OidcClientNode;
						NSMenuItem item = null;

						item = new NSMenuItem ("Add OIDC Client", node.AddOidcClient);
						var image1 = NSImage.ImageNamed ("NSAddTemplate");
						item.Image = image1;
						item.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);

						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					else if (obj is IdentityProvidersNode) {
						IdentityProvidersNode node = obj as IdentityProvidersNode;
						NSMenuItem item = null;

						item = new NSMenuItem ("Add External Identity Provider", node.AddExternalIdentityProvider);
						menu.AddItem (item);

						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					else if (obj is TrustedCertificateNode) {
						TrustedCertificateNode node = obj as TrustedCertificateNode;
						NSMenuItem item = null;

						item = new NSMenuItem ("Add Certificate Chain", node.AddCertificateChain);
						var image = NSImage.ImageNamed ("certificate.png");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);

						item = new NSMenuItem ("Refresh", node.Refresh);
						image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					else if (obj is ExternalDomainsNode) {
						ExternalDomainsNode node = obj as ExternalDomainsNode;
						NSMenuItem item = null;

						item = new NSMenuItem ("Add New External Domain", node.AddNewExternalDomain);
						var image1 = NSImage.ImageNamed ("NSAddTemplate");
						item.Image = image1;
						item.Image.Size = new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);

						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					else if (obj is ExternalDomainNode) {
						ExternalDomainNode node = obj as ExternalDomainNode;
						NSMenuItem item = null;

						item = new NSMenuItem ("Refresh", node.Refresh);
						var image = NSImage.ImageNamed ("NSRefresh");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);

						item = new NSMenuItem ("Properties", node.View);
						image = NSImage.ImageNamed ("config.png");
						item.Image = image;
						item.Image.Size =  new CGSize{ Width = (float)16.0, Height = (float)16.0 };
						menu.AddItem (item);
					}
					NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
				}
			}
			return base.MenuForEvent (theEvent);
		}
	}
}

