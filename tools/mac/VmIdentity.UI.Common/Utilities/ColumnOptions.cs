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

namespace VmIdentity.UI.Common.Utilities
{
	/// <summary>
	/// Column options.
	/// </summary>
	public class ColumnOptions
	{
		/// <summary>
		/// Initializes a new instance of the <see cref="VmIdentity.UI.Common.Utilities.ColumnOptions"/> class.
		/// </summary>
		public ColumnOptions()
		{
			Type = ColumnType.Browser;
		}

		/// <summary>
		/// Gets or sets the display name.
		/// </summary>
		/// <value>The display name.</value>
		public string DisplayName {
			get;
			set;
		}

		/// <summary>
		/// Gets or sets the identifier.
		/// </summary>
		/// <value>The identifier.</value>
		public string Id {
			get;
			set;
		}

		/// <summary>
		/// Gets or sets the width.
		/// </summary>
		/// <value>The width.</value>
		public int Width {
			get;
			set;
		}

		/// <summary>
		/// Gets or sets the display order.
		/// </summary>
		/// <value>The display order.</value>
		public uint DisplayOrder {
			get;
			set;
		}

		/// <summary>
		/// Gets or sets the type.
		/// </summary>
		/// <value>The type.</value>
		public ColumnType Type{
			get;set;
		}
	}

	/// <summary>
	/// Column type.
	/// </summary>
	public enum ColumnType
	{
		/// <summary>
		/// The text.
		/// </summary>
		Text,

		/// <summary>
		/// The browser.
		/// </summary>
		Browser
	}
}

