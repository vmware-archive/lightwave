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

using System.Drawing;
using VMPSCHighAvailability.Common.DTO;

namespace VMPscHighAvailabilitySnapIn.UI
{
    internal class HealthHelper
    {
        public static Color GetHealthColor(Health health)
        {
            Color color;
            switch(health)
            {
                case Health.Full:
                    color = Color.Green;
                    break;
                case Health.Limited:
                    color = Color.Orange;
                    break;
                case Health.Down:
                    color = Color.Red;
                    break;
                case Health.Legacy:
                    color = Color.Black;
                    break;
                default:
                    color = Color.Olive;
                    break;
            }

            return color;
        }

        public static Color GetHealthColor(bool isActive)
        {
            return isActive
                ? Color.Green
                : Color.Red;
        }
    }
}
