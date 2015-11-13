/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VMDir.Common.Schema
{
    public class ContentRuleDTO
    {
        SortOnFirstAccessStringList _aux = new SortOnFirstAccessStringList ();
        SortOnFirstAccessStringList _must = new SortOnFirstAccessStringList ();
        SortOnFirstAccessStringList _may = new SortOnFirstAccessStringList ();

        public string Name { get; set; }

        public List<string> Aux { get { return _aux.Data; } set { _aux.Data = value; } }

        public List<string> Must { get { return _must.Data; } set { _must.Data = value; } }

        public List<string> May { get { return _may.Data; } set { _may.Data = value; } }
    }

    public class SortOnFirstAccessStringList
    {
        bool _isSorted = false;
        private List<string> _list;

        public List<string> Data {
            get {
                if (_list != null && !_isSorted) {
                    _list.Sort ();
                    _isSorted = true;
                }
                return _list;
            }
            set { _list = value; }
        }
    }
}
