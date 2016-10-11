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

using System.Linq;
using System.Collections.Generic;
using VMAFD.Client;
using VMPSCHighAvailability.Common.DTO;

namespace VMPSCHighAvailability.Common.Helpers
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
        public static StateDescriptionDto GetStateDescription(CDC_DC_STATE cdcState)
        {
            var stateDescription = new StateDescriptionDto();

            if (cdcState == CDC_DC_STATE.CDC_DC_STATE_LEGACY)
            {
                stateDescription.Description = Constants.StateLegacyMode;
                stateDescription.LongDescription = Constants.StateDescHighAvailabilityDisabled;
            }
            else if (cdcState == CDC_DC_STATE.CDC_DC_STATE_NO_DC_LIST)
            {
                stateDescription.Description = Constants.StateNoKnowledgeOfAnyDcs;
                stateDescription.LongDescription = Constants.StateDescNoKnowledgeOfAnyDcs;
            }
            else if (cdcState == CDC_DC_STATE.CDC_DC_STATE_SITE_AFFINITIZED)
            {
                stateDescription.Description = Constants.StateSiteAffinitized;
                stateDescription.LongDescription = Constants.StateDescSiteAffinitized;
            }
            else if (cdcState == CDC_DC_STATE.CDC_DC_STATE_OFF_SITE)
            {
                stateDescription.Description = Constants.StateAffinitizedToOffsiteDc;
                stateDescription.LongDescription = Constants.StateDescAffinitizedToOffsiteDc;
            }
            else if (cdcState == CDC_DC_STATE.CDC_DC_STATE_NO_DCS_ALIVE)
            {
                stateDescription.Description = Constants.StateAllKnownDcsAreDown;
                stateDescription.LongDescription = Constants.StateDescAllKnownDcsAreDown;
            }
            else
            {
                stateDescription.Description = Constants.StateInvalidState;
                stateDescription.LongDescription = Constants.StateDescInvalidState;
            }
            return stateDescription;
        }

        public static Health GetHealth(StateDescriptionDto state, IEnumerable<NodeDto> dcs)
        {
            Health health = Health.Full;

            if (state.Description == Constants.StateLegacyMode)
            {
                health = Health.Legacy;
            }
            else
            {
                var activeDcs = dcs.Count(x => x.Active);
                health = (activeDcs >= 2) ? Health.Full
                                          : ((activeDcs == 1) ? Health.Limited
                                                              : Health.Down);
            }
            return health;
        }

        public static string GetHealthDescription(Health health)
        {
            var description = string.Empty;
            if (health == Health.Full)
            {
                description = Constants.FullHealth;
            }
            else if (health == Health.Limited)
            {
                description = Constants.LimitedHealth;
            }
            else if (health == Health.Down)
            {
                description = Constants.DownHealth;
            }
            else if (health == Health.Legacy)
            {
                description = Constants.LegacyHealth;
            }
            return description;
        }  

        public static string GetActiveServiceDesc(InfrastructureDto dc)
        {
            var activeServices = dc.Services.Count(x => x.Alive);
            var totalServices = dc.Services.Count();
            var status = string.Format("{0} out of {1} services are {2}", activeServices, totalServices, Constants.Active);
            return status;
        }

        public static string GetHealthDescription()
        {
            return Constants.GetHealthDescription();
        }
    }
}

