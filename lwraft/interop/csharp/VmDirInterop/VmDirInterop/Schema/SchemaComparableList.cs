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
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Utils;

namespace VmDirInterop.Schema
{
    public class SchemaComparableList<T> : List<T> where T : ISchemaComparable<T>
    {
        public SchemaComparableList()
        {
        }

        public SchemaComparableList(IList<T> ilist) : base(ilist)
        {
            Sort();
        }

        public TupleList<T, T> GetDiff(SchemaComparableList<T> other)
        {
            if (other == null)
            {
                throw new ArgumentNullException("other");
            }

            TupleList<T, T> diff = new TupleList<T, T>();
            int i = 0, j = 0;

            while (i < this.Count || j < other.Count)
            {
                if (i == this.Count)
                {
                    diff.Add(default(T), other[j]);
                    j++;
                }
                else if (j == other.Count)
                {
                    diff.Add(this[i], default(T));
                    i++;
                }
                else
                {
                    int nameCmp = this[i].NameCompareTo(other[j]);
                    if (nameCmp < 0)
                    {
                        diff.Add(this[i], default(T));
                        i++;
                    }
                    else if (nameCmp > 0)
                    {
                        diff.Add(default(T), other[j]);
                        j++;
                    }
                    else
                    {
                        if (this[i].FullCompareTo(other[j]) != 0)
                        {
                            diff.Add(this[i], other[j]);
                        }
                        i++;
                        j++;
                    }
                }
            }

            return diff;
        }
    }
}
