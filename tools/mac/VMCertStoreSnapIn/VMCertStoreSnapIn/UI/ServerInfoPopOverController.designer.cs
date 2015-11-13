// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMCertStoreSnapIn
{
	[Register ("ServerInfoPopOverController")]
	partial class ServerInfoPopOverController
	{
		[Outlet]
		AppKit.NSTextField NoStoresLabel { get; set; }

		[Outlet]
		AppKit.NSTextField PrivateKeysLabel { get; set; }

		[Outlet]
		AppKit.NSTextField SecretKeysLabel { get; set; }

		[Outlet]
		AppKit.NSTextField TrustedCertsLabel { get; set; }

		[Action ("AddRootCertificate:")]
		partial void AddRootCertificate (Foundation.NSObject sender);

		[Action ("CreateStore:")]
		partial void CreateStore (Foundation.NSObject sender);

		[Action ("ShowRootCertificate:")]
		partial void ShowRootCertificate (Foundation.NSObject sender);

		[Action ("ValidateCA:")]
		partial void ValidateCA (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (NoStoresLabel != null) {
				NoStoresLabel.Dispose ();
				NoStoresLabel = null;
			}

			if (PrivateKeysLabel != null) {
				PrivateKeysLabel.Dispose ();
				PrivateKeysLabel = null;
			}

			if (TrustedCertsLabel != null) {
				TrustedCertsLabel.Dispose ();
				TrustedCertsLabel = null;
			}

			if (SecretKeysLabel != null) {
				SecretKeysLabel.Dispose ();
				SecretKeysLabel = null;
			}
		}
	}
}
