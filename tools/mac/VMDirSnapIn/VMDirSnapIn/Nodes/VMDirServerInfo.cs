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
using VMDir.Common.DTO;
using VMDir.Common.VMDirUtilities;
using System.Threading.Tasks;
using VMIdentity.CommonUtils;

namespace Nodes
{
	public class VMDirServerInfo : Foundation.NSObject
	{
		public bool IsLoggedIn { get; set; }
		public bool loginComplete { get; set; }
		public VMDirServerDTO DTO { get; set; }

		public VMDirServerInfo(VMDirServerDTO dto)
		{
			DTO = dto;
			IsLoggedIn = false;
			loginComplete = false;
		}

		public async Task DoLogin()
		{
			try
			{
				Task t = new Task(ServerConnect);
				t.Start();
				if (await Task.WhenAny(t, Task.Delay(CommonConstants.TEN_SEC * 3)) == t)
				{
					await t;
				}
				else {
					throw new Exception(CommonConstants.SERVER_TIMEOUT);
				}
			}
			catch (Exception ex)
			{
				throw ex;
			}
		}

		public void ServerConnect()
		{
			try
			{
				DTO.Connection = new LdapConnectionService(DTO.Server, DTO.BindDN, DTO.Password);

				if (DTO.Connection.CreateConnection() == 1)
					IsLoggedIn = true;
				else
					IsLoggedIn = false;
			}
			catch (Exception)
			{
				IsLoggedIn = false;
				throw;
			}
		}
	}
}

