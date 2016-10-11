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
using VMAFD.Client;
using VMPSCHighAvailability.Common.DTO;

namespace VMPSCHighAvailability.UI.Helpers
{
	/// <summary>
	/// Cdc state helper.
	/// </summary>
	public class CdcDcStateHelper
	{
		/// <summary>
		/// Gets the description corresponding to the CDC DC State
		/// </summary>
		/// <returns>The description.</returns>
		/// <param name="cdcState">Cdc state.</param>
		public static PscStateDescriptionDto GetStateDescription(CDC_DC_STATE cdcState)
		{
			var stateDescription = new PscStateDescriptionDto { State = cdcState};

			switch (cdcState) {

			case CDC_DC_STATE.CDC_DC_STATE_DISABLED:
				stateDescription.Description = Constants.StateHighAvailabilityDisabled;
				stateDescription.LongDescription = Constants.StateDescHighAvailabilityDisabled;
				break;

			case CDC_DC_STATE.CDC_DC_STATE_NO_DC_LIST:
				stateDescription.Description = Constants.StateNoKnowledgeOfAnyDcs;
				stateDescription.LongDescription = Constants.StateDescNoKnowledgeOfAnyDcs;
				break;

			case CDC_DC_STATE.CDC_DC_STATE_SITE_AFFINITIZED:
				stateDescription.Description = Constants.StateSiteAffinitized;
				stateDescription.LongDescription = Constants.StateDescSiteAffinitized;
				break;

			case CDC_DC_STATE.CDC_DC_STATE_OFF_SITE:
				stateDescription.Description = Constants.StateAffinitizedToOffsiteDc;
				stateDescription.LongDescription = Constants.StateDescAffinitizedToOffsiteDc;
				break;

			case CDC_DC_STATE.CDC_DC_STATE_NO_DCS_ALIVE:
				stateDescription.Description = Constants.StateAllKnownDcsAreDown;
				stateDescription.LongDescription = Constants.StateDescAllKnownDcsAreDown;
				break;

			default:
				stateDescription.Description = Constants.StateInvalidState;
				stateDescription.LongDescription = Constants.StateDescInvalidState;
				break;
			}
			return stateDescription;
		}
	}
}

