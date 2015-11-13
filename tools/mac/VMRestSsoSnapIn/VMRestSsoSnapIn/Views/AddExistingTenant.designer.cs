// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace RestSsoAdminSnapIn
{
	[Register ("AddExistingTenantController")]
	partial class AddExistingTenantController
	{
		[Outlet]
		AppKit.NSButton BtnCancel { get; set; }

		[Outlet]
		AppKit.NSButton BtnOk { get; set; }

		[Outlet]
		AppKit.NSTextField TXTTenantName { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TXTTenantName != null) {
				TXTTenantName.Dispose ();
				TXTTenantName = null;
			}

			if (BtnOk != null) {
				BtnOk.Dispose ();
				BtnOk = null;
			}

			if (BtnCancel != null) {
				BtnCancel.Dispose ();
				BtnCancel = null;
			}
		}
	}

	[Register ("AddExistingTenant")]
	partial class AddExistingTenant
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}
