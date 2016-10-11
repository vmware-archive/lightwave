/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;

namespace VMDir.Common
{
	public class SuperLogDto
	{	
		public string ClientIP { get;set;}
		public string Port { get;set;}
		public string LoginDN { get;set;}
		public string Operation { get;set;}
		public string ErrorCode { get;set;}
		public string Duration { get;set;}
		public long DurationLong {get;set;}
		public string String{ get; set; }
	}
}

