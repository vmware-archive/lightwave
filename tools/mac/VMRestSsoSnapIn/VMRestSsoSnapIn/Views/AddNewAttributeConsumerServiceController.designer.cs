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
	[Register ("AddNewAttributeConsumerServiceController")]
	partial class AddNewAttributeConsumerServiceController
	{
		[Outlet]
		AppKit.NSTableView AttributeTableView { get; set; }

		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveAttribute { get; set; }

		[Outlet]
		AppKit.NSButton ChDefault { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAttributeName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtFriendlyName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtIndex { get; set; }

		[Outlet]
		AppKit.NSTextField TxtName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtNameFormat { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AttributeTableView != null) {
				AttributeTableView.Dispose ();
				AttributeTableView = null;
			}

			if (BtnAdd != null) {
				BtnAdd.Dispose ();
				BtnAdd = null;
			}

			if (BtnAddAttribute != null) {
				BtnAddAttribute.Dispose ();
				BtnAddAttribute = null;
			}

			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemoveAttribute != null) {
				BtnRemoveAttribute.Dispose ();
				BtnRemoveAttribute = null;
			}

			if (ChDefault != null) {
				ChDefault.Dispose ();
				ChDefault = null;
			}

			if (TxtAttributeName != null) {
				TxtAttributeName.Dispose ();
				TxtAttributeName = null;
			}

			if (TxtFriendlyName != null) {
				TxtFriendlyName.Dispose ();
				TxtFriendlyName = null;
			}

			if (TxtIndex != null) {
				TxtIndex.Dispose ();
				TxtIndex = null;
			}

			if (TxtNameFormat != null) {
				TxtNameFormat.Dispose ();
				TxtNameFormat = null;
			}
		}
	}
}
