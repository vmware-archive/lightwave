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
using System.Linq;
using System.Text;

namespace VMAfd
{
    public delegate UInt32 GetStatusDelegate (string server, out VMAfdStatus statusOut);
    public delegate UInt32 GetStringDelegate (string server, out IntPtr result);
    public delegate UInt32 SetStringDelegate (string server, string value);

    public delegate UInt32 GetValueDelegate<T> (string server, T value);

    public static class VMAfdPropertyDef
    {
        static List<PropertyDefEntry> _propDefs = new List<PropertyDefEntry> ();

        public static List<PropertyDefEntry> PropDefs {
            get {
                if (_propDefs.Count == 0)
                    InitPropDefs ();
                return _propDefs;
            }
        }

        static void InitPropDefs ()
        {
            _propDefs.Clear ();

            _propDefs.Add (new PropertyDefEntry { Name = "Status", GetStatus = VMAfdAdaptor.VmAfdGetStatusA });
            _propDefs.Add (new PropertyDefEntry {
                Name = "CMLocation",
                GetString = VMAfdAdaptor.VmAfdGetCMLocationA,
                SetString = VMAfdAdaptor.VmAfdSetCMLocationA
            });
            _propDefs.Add (new PropertyDefEntry {
                Name = "DCName",
                GetString = VMAfdAdaptor.VmAfdGetDCNameA,
                SetString = VMAfdAdaptor.VmAfdSetDCNameA
            });
            _propDefs.Add (new PropertyDefEntry {
                Name = "Domain",
                GetString = VMAfdAdaptor.VmAfdGetDomainNameA,
                SetString = VMAfdAdaptor.VmAfdSetDomainNameA
            });
            _propDefs.Add (new PropertyDefEntry {
                Name = "LDU",
                GetString = VMAfdAdaptor.VmAfdGetLDUA,
                SetString = VMAfdAdaptor.VmAfdSetLDUA
            });
        }
    }

    public interface IAfdPropertyDef
    {
        string Name { get; set; }

        bool CanSet { get; }

        object GetValue ();

        void SetValue (object value);
    }

    public class PropertyDefEntry
    {
        public string Name{ get; set; }

        public GetStatusDelegate GetStatus { get; set; }

        public GetStringDelegate GetString { get; set; }

        public SetStringDelegate SetString { get; set; }
    }
}
