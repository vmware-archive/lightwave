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
	[Register ("ViewUserDetailsController")]
	partial class ViewUserDetailsController
	{
		[Outlet]
		AppKit.NSButton BtnAddGroup { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveGroup { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton ChActAsUser { get; set; }

		[Outlet]
		AppKit.NSButton ChDisabled { get; set; }

		[Outlet]
		AppKit.NSButton ChIdpAdmin { get; set; }

		[Outlet]
		AppKit.NSScrollView LstGroupMembershipView { get; set; }

		[Outlet]
		AppKit.NSMatrix RdoRoleGroup { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtEmail { get; set; }

		[Outlet]
		AppKit.NSTextField TxtFirstName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLastName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordDaysUntilExpiry { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordLastChanged { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddGroup != null) {
				BtnAddGroup.Dispose ();
				BtnAddGroup = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemoveGroup != null) {
				BtnRemoveGroup.Dispose ();
				BtnRemoveGroup = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (ChActAsUser != null) {
				ChActAsUser.Dispose ();
				ChActAsUser = null;
			}

			if (ChDisabled != null) {
				ChDisabled.Dispose ();
				ChDisabled = null;
			}

			if (ChIdpAdmin != null) {
				ChIdpAdmin.Dispose ();
				ChIdpAdmin = null;
			}

			if (LstGroupMembershipView != null) {
				LstGroupMembershipView.Dispose ();
				LstGroupMembershipView = null;
			}

			if (RdoRoleGroup != null) {
				RdoRoleGroup.Dispose ();
				RdoRoleGroup = null;
			}

			if (TxtDescription != null) {
				TxtDescription.Dispose ();
				TxtDescription = null;
			}

			if (TxtEmail != null) {
				TxtEmail.Dispose ();
				TxtEmail = null;
			}

			if (TxtFirstName != null) {
				TxtFirstName.Dispose ();
				TxtFirstName = null;
			}

			if (TxtLastName != null) {
				TxtLastName.Dispose ();
				TxtLastName = null;
			}

			if (TxtPasswordDaysUntilExpiry != null) {
				TxtPasswordDaysUntilExpiry.Dispose ();
				TxtPasswordDaysUntilExpiry = null;
			}

			if (TxtPasswordLastChanged != null) {
				TxtPasswordLastChanged.Dispose ();
				TxtPasswordLastChanged = null;
			}

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}
		}
	}

	[Register ("ViewUserDetails")]
	partial class ViewUserDetails
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}
