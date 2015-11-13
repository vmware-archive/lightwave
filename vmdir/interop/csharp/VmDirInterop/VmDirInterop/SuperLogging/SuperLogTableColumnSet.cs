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
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using VmDirInterop.SuperLogging.Constants;
using VmDirInterop.SuperLogging.Interfaces;

namespace VmDirInterop.SuperLogging
{
    public class SuperLogTableColumnSet : IEnumerable
    {
        private static int _colNum = Enum.GetNames(typeof(SuperLogTableColumn)).Length;
        private bool[] _isColumnSet = new bool[_colNum];

        public bool this[SuperLogTableColumn index]
        {
            get
            {
                return _isColumnSet[(int)index];
            }

            set
            {
                _isColumnSet[(int)index] = value;
            }
        }

        public IEnumerator GetEnumerator()
        {
            foreach (SuperLogTableColumn col in Enum.GetValues(typeof(SuperLogTableColumn)))
            {
                if (this[col])
                {
                    yield return col;
                }
            }
        }

        public VMDIR_SUPERLOG_TABLE_COLUMN_SET ToStruct()
        {
            VMDIR_SUPERLOG_TABLE_COLUMN_SET colSetStruct = new VMDIR_SUPERLOG_TABLE_COLUMN_SET();
            colSetStruct.isColumnSet = new UInt32[_colNum];
            foreach (SuperLogTableColumn col in this)
            {
                colSetStruct.isColumnSet[(int)col] = 1;
            }
            return colSetStruct;
        }
    }
}
