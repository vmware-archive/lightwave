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
using System.Threading;
using System.Collections.Generic;
using VMPSCHighAvailability.Common;
using System.Linq;
using Foundation;
using AppKit;

namespace VMPSCHighAvailability.UI
{
	public partial class SplitCustomViewController : AppKit.NSViewController
	{
		private NSView _oldView;
		public NSViewController DetailViewController { get; set; }
		private Timer _timer;
		private int _value;

		#region Constructors

		// Called when created from unmanaged code
		public SplitCustomViewController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public SplitCustomViewController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}

		// Call to load from the XIB/NIB file
		public SplitCustomViewController () : base ("SplitCustomView", NSBundle.MainBundle)
		{
			Initialize ();
		}

		// Shared initialization code
		void Initialize ()
		{
		}

		/// <summary>
		/// Awakes from nib.
		/// </summary>
		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			_timer = new Timer(timer_Tick, null, -1, -1);
			_value = Constants.TopologyTimeout;
			_oldView = DetailViewController.View;
			DetailView.AddSubview(_oldView);
			TreeView.OutlineTableColumn.DataCell = new NSBrowserCell ();
		}
		#endregion

		//strongly typed view accessor
		public new SplitCustomView View {
			get {
				return (SplitCustomView)base.View;
			}
		}

		public void RefreshDetailView()
		{
			if (DetailViewController != null) {
				var newView = DetailViewController.View;
				var cg = new CoreGraphics.CGSize ();
				cg.Height = DetailView.Frame.Height;
				cg.Width = DetailView.Frame.Width;
				newView.SetFrameSize (cg);
				DetailView.ReplaceSubviewWith(_oldView, newView);
				_oldView = newView;
			}
		}

		/// <summary>
		/// Timer tick.
		/// </summary>
		/// <param name="state">State.</param>
		private void timer_Tick(Object state)
		{	
			InvokeOnMainThread(() =>
				{
					_value--;
					ProgressIndicator.IncrementBy(1);
					var displayText = string.Format(" in {0} seconds..", _value);
					SetRefreshText (Constants.LoadingTopologyDetails + displayText);
				});
		}

		/// <summary>
		/// Sets the refresh status.
		/// </summary>
		/// <param name="start">If set to <c>true</c> start.</param>
		public void SetRefreshText (string text)
		{
			RefreshTextField.StringValue = text;
		}

		/// <summary>
		/// Sets the refresh status.
		/// </summary>
		/// <param name="start">If set to <c>true</c> start.</param>
		public void SetRefreshStatus (bool start)
		{
			PnlSplitView.Hidden = start;
			ProgressIndicator.Hidden = !start;
			RefreshTextField.Hidden = !start;
			if (start) {
				var interval = Constants.MilliSecsMultiplier;
				_timer.Change (interval, interval);
				ProgressIndicator.MinValue = 0;
				ProgressIndicator.MaxValue = Constants.TopologyTimeout;
				ProgressIndicator.StartAnimation (this);
				ProgressIndicator.IncrementBy(1);
			} else {
				ProgressIndicator.IncrementBy (-1 * ProgressIndicator.DoubleValue);
				_value = Constants.TopologyTimeout;
				_timer.Change(-1,-1);
				ProgressIndicator.StopAnimation (this);
			}
		}
	}
}
