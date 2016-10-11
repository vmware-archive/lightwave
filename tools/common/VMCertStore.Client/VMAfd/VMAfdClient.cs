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
using System.Runtime.InteropServices;
using VMAfd;
using VMAfd.Client;
using VMCertStore.Client.Exceptions;

namespace VMCertStore.Client
{
    public class VMAfdClient
    {
        public string ServerName { get; protected set; }

        public VMAfdClient (string serverName)
        {
            ServerName = serverName;
        }

        public Dictionary<string, string> GetProperties ()
        {
            var props = new Dictionary<string, string> ();
            foreach (var propEntry in VMAfdPropertyDef.PropDefs) {
                try {
                    string value = "";
                    if (propEntry.GetStatus != null) {
                        var status = VMAfdStatus.Unknown;
                        var result = propEntry.GetStatus (ServerName, out status);
                        VMAfdClientError.Check (result);
                        value = status.ToString ();
                    } else if (propEntry.GetString != null) {
                        var ptr = new IntPtr ();
                        var result = propEntry.GetString (ServerName, out ptr);
                        VMAfdClientError.Check (result);
                        value = Marshal.PtrToStringAnsi (ptr);
                        if (value == null)
                            value = "";
                    }
                    props [propEntry.Name] = value;
                } catch (VMAfdException exp) {
                    props [propEntry.Name] = exp.Message;
                } catch (VMAfdValueNotSetException) {
                    props [propEntry.Name] = "<Value not set>";
                }
            }
            return props;
        }

        public bool SetProperty (string key, string value)
        {
            var entry = VMAfdPropertyDef.PropDefs.Find (x => x.Name == key);
            if (entry == null)
                return false;
            VMAfdClientError.Check (entry.SetString (ServerName, value));
            return true;
        }

        public void JoinVMwareDirectory (string domain, string ou, string user, string pass)
        {
            var result = VMAfdAdaptor.VmAfdJoinVMwareDirectoryA (ServerName, domain, ou, user, pass);
            VMAfdClientError.Check (result);
        }

        public void JoinActiveDirectory (string domain, string computername, string user, string pass)
        {
            var result = VMAfdAdaptor.VmAfdJoinActiveDirectoryA (ServerName, domain, computername, user, pass);
            VMAfdClientError.Check (result);
        }

        public UInt32 VmDirSplit (
            string serverName,
            string sourceUserName,
            string sourcePassword,
            string targetDomain,
            string targetUserName,
            string targetPassword
        )
        {
            UInt32 result = VMAfdAdaptor.VmAfdVdcSplitA (
                                serverName,
                                sourceUserName,
                                sourcePassword,
                                targetDomain,
                                targetUserName,
                                targetPassword
                            );

            //string errMsg = VMAfdAdaptor.VmAfdGetErrorMessageA(result);

            VMAfdClientError.Check (result);
            return result;
        }

        public UInt32 VmDirMerge (
            string serverName,
            string sourceUserName,
            string sourcePassword,
            string targetHost,
            string targetUserName,
            string targetPassword
        )
        {
            UInt32 result = VMAfdAdaptor.VmAfdVdcMergeA (
                                serverName,
                                sourceUserName,
                                sourcePassword,
                                targetHost,
                                targetUserName,
                                targetPassword
                            );

            VMAfdClientError.Check (result);
            return result;
        }

        public UInt32 VmDirForceReplication (
            string serverName
        )
        {
            UInt32 result = VMAfdAdaptor.VmAfdForceReplicationA (
                                serverName
                            );

            VMAfdClientError.Check (result);
            return result;
        }
    }
}
