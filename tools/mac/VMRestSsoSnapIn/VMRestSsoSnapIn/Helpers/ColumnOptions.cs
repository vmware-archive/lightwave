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

namespace Vmware.Tools.RestSsoAdminSnapIn.Helpers
{
	public class ColumnOptions
	{
		public ColumnOptions()
		{
			Type = ColumnType.Browser;
		}
		public string DisplayName {
			get;
			set;
		}
		public string Id {
			get;
			set;
		}
		public int Width {
			get;
			set;
		}
		public uint DisplayOrder {
			get;
			set;
		}
		public ColumnType Type{
			get;set;
		}
	}

	public enum ColumnType
	{
		Text,
		Browser
	}
}

