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

using System.Collections.Generic;
using System.Linq;
using VMAFD.Client;
using VMIdentity.CommonUtils;
using VMPSCHighAvailability.Common.DTO;

namespace VMPSCHighAvailability.Common.Service
{
    /// <summary>
    /// Service helper
    /// </summary>
	public class ServiceHelper
	{
		/// <summary>
		/// Converts the dc to psc dto.
		/// </summary>
		/// <param name="dcs">Dcs.</param>
		/// <param name="dc">Dc.</param>
		public List<InfrastructureDto> ConvertDcToPscDto (CDC_DC_INFO dc, IList<string> dcs)
		{
			return ((dcs == null)? new List<InfrastructureDto> ()
				: dcs.Select (
					x => new InfrastructureDto 
                    {
						Name = x ,
						IsAffinitized = (x == dc.pszDCName)
					}
				).ToList ());
		}


		/// <summary>
		/// Gets the services.
		/// </summary>
		/// <returns>The services.</returns>
		/// <param name="status">Status.</param>
		public static List<ServiceDto> GetServices(string hostname, VMAFD_HEARTBEAT_STATUS status)
		{
			var services = new List<ServiceDto> ();
			if (status.info != null) {

				foreach (var info in status.info)
                {
					var service = new ServiceDto () 
                    {
						HostName = hostname,
						ServiceName = GetServiceName(info.pszServiceName),
						Description = GetServiceDescription(info.pszServiceName),
						Port = info.dwPort,
						Alive = info.bIsAlive == 1,
                        LastHeartbeat = DateTimeConverter.FromUnixToDateTime(info.dwLastHeartbeat)
					};
					services.Add (service);
				}
				var afdService = new ServiceDto () 
				{
					HostName = hostname,
					ServiceName = Constants.AuthFrameworkServiceName,
					Description = Constants.AuthFrameworkServiceDesc,
					Alive = true,
					LastHeartbeat = System.DateTime.UtcNow
				};
				services.Add (afdService);
			}
			return services.OrderBy(x=>x.ServiceName).ToList();
		}

		/// <summary>
		/// Gets the name of the service.
		/// </summary>
		/// <returns>The service name.</returns>
		/// <param name="name">Name.</param>
		public static string GetServiceName(string name)
		{	
			if (name == Constants.IdentityManagementService) {
				return Constants.IdentityManagementServiceName;
			}
			if (name == Constants.LicenseService) {
				return Constants.LicenseServiceName;
			}
			if (name == Constants.SsoAdminService) {
				return Constants.AdminServiceName;
			}
			if (name == Constants.WebSsoService) {
				return Constants.WebSsoServiceName;
			}
			if (name == Constants.StsService) {
				return Constants.StsServiceName;
			}
			if (name == Constants.CertificateService) {
				return Constants.CertificateServiceName;
			}
			return name;
		}

		public static string GetServiceDescription(string name)
		{	
			if (name == Constants.IdentityManagementService) {
				return Constants.IdentityManagementServiceDesc;
			}
			if (name == Constants.LicenseService) {
				return Constants.LicenseServiceDesc;
			}
			if (name == Constants.SsoAdminService) {
				return Constants.AdminServiceDesc;
			}
			if (name == Constants.WebSsoService) {
				return Constants.WebSsoServiceDesc;
			}
			if (name == Constants.StsService) {
				return Constants.StsServiceDesc;
			}
			if (name == Constants.CertificateService) {
				return Constants.CertificateServiceDesc;
			}
            if (name == Constants.LookupService)
            {
                return Constants.LookupServiceDesc;
			}
            
			return string.Empty;
		}
	}
}

