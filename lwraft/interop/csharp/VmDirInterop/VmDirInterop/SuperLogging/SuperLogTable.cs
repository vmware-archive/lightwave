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
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using VmDirInterop.SuperLogging.Interfaces;

namespace VmDirInterop.SuperLogging
{
    public class SuperLogTable : ISuperLogTable
    {
        private IntPtr _pTable;
        private List<ISuperLogTableRow> _rows = new List<ISuperLogTableRow>();
        private static int _rowSize =
            Marshal.SizeOf(typeof(VMDIR_SUPERLOG_TABLE_ROW));

        public SuperLogTable(IntPtr pTable)
        {
            _pTable = pTable;
            if (_pTable != IntPtr.Zero)
            {
                VMDIR_SUPERLOG_TABLE table =
                    (VMDIR_SUPERLOG_TABLE)Marshal.PtrToStructure(
                        _pTable,
                        typeof(VMDIR_SUPERLOG_TABLE));

                long ptr = table.rows.ToInt64();
                for (int i = 0; i < table.numRows; i++, ptr += _rowSize)
                {
                    VMDIR_SUPERLOG_TABLE_ROW r =
                        (VMDIR_SUPERLOG_TABLE_ROW)Marshal.PtrToStructure(
                            new IntPtr(ptr),
                            typeof(VMDIR_SUPERLOG_TABLE_ROW));
                    _rows.Add(new SuperLogTableRow(r));
                }
            }
        }

        ~SuperLogTable()
        {
            SuperLoggingClientLibrary.VmDirFreeSuperLogTable(_pTable);
        }

        public IEnumerator GetEnumerator()
        {
            foreach (ISuperLogTableRow r in _rows)
            {
                yield return r;
            }
        }

        public List<ISuperLogTableRow> getRows()
        {
            return _rows;
        }

        public int getRowNum()
        {
            return _rows.Count;
        }
    }
}
