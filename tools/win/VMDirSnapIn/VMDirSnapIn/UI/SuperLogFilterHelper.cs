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
using VmDirInterop.SuperLogging.Interfaces;

namespace VMDirSnapIn.UI
{
    public class SuperLogFilterHelper
    {
        Dictionary<int, ISuperLogEntry> _viewCache
            = new Dictionary<int, ISuperLogEntry>();

        Dictionary<FilterColumn, Func<ISuperLogEntry, object>> _valDict
            = new Dictionary<FilterColumn, Func<ISuperLogEntry, object>>
        {
            {FilterColumn.ClientIP,     x=>x.getClientIP()},
            {FilterColumn.ClientPort,   x=>x.getClientPort()},
            {FilterColumn.LoginDN,      x=>x.getLoginDN()},
            {FilterColumn.Operation,    x=>x.getOperation()},
            {FilterColumn.ErrorCode,    x=>x.getErrorCode()},
            {FilterColumn.Duration,     x=>x.getEndTime() - x.getStartTime()},
        };

        Dictionary<NumericFilterOperation, Func<UInt64, UInt64, bool>> _numopDict
            = new Dictionary<NumericFilterOperation, Func<UInt64, UInt64, bool>>
        {
            {NumericFilterOperation.EQ, (x, y) => x == y},
            {NumericFilterOperation.GTE, (x, y) => x >= y},
            {NumericFilterOperation.LTE, (x, y) => x <= y},
        };

        Dictionary<StringFilterOperation, Func<string, string, bool>> _strOpDict
            = new Dictionary<StringFilterOperation, Func<string, string, bool>>
        {
            {StringFilterOperation.BeginsWith, (x, y) => x.StartsWith(y)},
            {StringFilterOperation.Contains, (x, y) => x.Contains(y)},
            {StringFilterOperation.EndsWith, (x, y) => x.EndsWith(y)},
            {StringFilterOperation.Equals, (x, y) => x.Equals(y)}
        };

        public Dictionary<int, ISuperLogEntry> ViewCache
        {
            get { return _viewCache; }
        }
        public FilterColumn FilterColumn { get; set; }
        public NumericFilterOperation NumericFilter { get; set; }
        public StringFilterOperation StringFilter { get; set; }
        public string FilterText { get; set; }


        public bool IsNumericFilter()
        {
            return FilterColumn == FilterColumn.Duration;
        }

        public void SetFilterOperation(int operation)
        {
            if (IsNumericFilter())
                NumericFilter = (NumericFilterOperation)operation;
            else
                StringFilter = (StringFilterOperation)operation;
        }

        public bool IsEnabled()
        {
            if (IsNumericFilter())
            {
                return FilterColumn != FilterColumn.None
                    && NumericFilter != NumericFilterOperation.None
                    && !string.IsNullOrEmpty(FilterText);
            }
            else
            {
                return FilterColumn != FilterColumn.None
                    && StringFilter != StringFilterOperation.None
                    && !string.IsNullOrEmpty(FilterText);
            }
        }

        public bool Check(ISuperLogEntry entry)
        {
            var columnVal = _valDict[FilterColumn](entry);
            if (IsNumericFilter())
            {
                if (_numopDict.ContainsKey(NumericFilter))
                {
                    var filterVal = Convert.ToUInt64(FilterText);
                    return _numopDict[NumericFilter]((UInt64)columnVal, filterVal);
                }
            }
            else
            {
                if (_strOpDict.ContainsKey(StringFilter))
                {
                    return _strOpDict[StringFilter](columnVal.ToString(), FilterText);
                }
            }
            return false;
        }

        public bool Filter(Dictionary<int, ISuperLogEntry> viewCache)
        {
            _viewCache.Clear();

            if (!IsEnabled())
                return false;

            int i = 0;
            foreach (var entry in viewCache)
            {
                if (Check(entry.Value))
                {
                    _viewCache[i++] = entry.Value;
                }
            }
            return true;
        }
    }
}
