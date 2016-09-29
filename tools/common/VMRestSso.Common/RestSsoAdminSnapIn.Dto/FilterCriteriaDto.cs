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
using System.Collections.Generic;
using System.ComponentModel;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    public class FilterCriteriaDto
    {
        public FilterColumn Column { get; set; }
        public Operator Operator { get; set; }
        public string Value { get; set; }
        public bool Apply(EventLogDto dto)
        {
            var match = false;
            if(Column == FilterColumn.EventType)
            {
                match = IsStringMatch(dto.Type);
            }
            else if (Column == FilterColumn.CorrelationID)
            {
                match = IsStringMatch(dto.CorrelationId);
            }
            else if (Column == FilterColumn.Status)
            {
                match = IsStringMatch(dto.Level.ToString());
            }
            else if (Column == FilterColumn.Duration)
            {
                match = IsNumericMatch(dto.ElapsedMillis);
            }
            else if (Column == FilterColumn.Start)
            {
                match = IsDateMatch(dto.Start);
            }
            else if (Column == FilterColumn.Provider)
            {
                match = IsStringMatch(dto.ProviderName);
            }
            else if (Column == FilterColumn.Account)
            {
                match = IsStringMatch(dto.AccountName);
            }
            return match;
        }

        private bool IsStringMatch(string attribute)
        {
            var match = false;

            if (Operator == Dto.Operator.EqualTo)
            {
                match = attribute == Value;
            }
            else if (Operator == Dto.Operator.BeginsWith)
            {
                match = attribute.StartsWith(Value);
            }
            else if (Operator == Dto.Operator.EndsWith)
            {
                match = attribute.EndsWith(Value);
            }
            else if (Operator == Dto.Operator.NotEqualTo)
            {
                match = attribute != Value;
            }
            else if (Operator == Dto.Operator.Contains)
            {
                match = attribute.Contains(Value);
            }
            return match;
        }

        private bool IsNumericMatch(long attribute)
        {
            var match = false;
            var value = long.Parse(Value);

            if (Operator == Dto.Operator.EqualTo)
            {
                match = attribute == value;
            }
            else if (Operator == Dto.Operator.LessThan)
            {
                match = attribute < value;
            }
            else if (Operator == Dto.Operator.GreaterThan)
            {
                match = attribute > value;
            }
            else if (Operator == Dto.Operator.NotEqualTo)
            {
                match = attribute != value;
            }
            return match;
        }

        private bool IsDateMatch(long attribute)
        {
            var match = false;
            var date = DateTime.Parse(Value);
            var value = DateTimeHelper.WindowsToUnix(date);

            if (Operator == Dto.Operator.EqualTo)
            {
                match = attribute == value;
            }
            else if (Operator == Dto.Operator.LessThan)
            {
                match = attribute < value;
            }
            else if (Operator == Dto.Operator.GreaterThan)
            {
                match = attribute > value;
            }
            else if (Operator == Dto.Operator.NotEqualTo)
            {
                match = attribute != value;
            }
            return match;
        }
    }

    public enum FilterColumn
    {
        [Description("Event Type")]
        EventType = 0,
        [Description("Correlation ID")]
        CorrelationID = 1,
        [Description("Status")]
        Status = 2,
        [Description("Start")]
        Start = 3,
        [Description("Duration")]
        Duration = 4,
        [Description("Provider")]
        Provider = 5,
        [Description("Account")]
        Account = 6
    }

    public enum Operator
    {
        [Description("starts with")]
        BeginsWith=0,
        [Description("<")]
        LessThan=1,
        [Description("=")]
        EqualTo=2,
        [Description("!=")]
        NotEqualTo=3,
        [Description(">")]
        GreaterThan=4,
        [Description("ends with")]
        EndsWith=5,
        [Description("contains")]
        Contains=6
    }
}
